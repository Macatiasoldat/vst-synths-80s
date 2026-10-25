#pragma once

// Voix soustractive commune aux Juno-60, Jupiter-8, OB-Xa et Prophet-5.
// Ce qui les distingue vient du Patch : type de filtre, Poly-Mod, sub-osc, dérive, chorus...

#include "Filters.h"
#include "Patch.h"

namespace retro
{
// Correction PolyBLEP pour des dents de scie et carrés sans repliement audible.
inline float polyBlep (float t, float dt)
{
    if (t < dt) { t /= dt; return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}

class Oscillator
{
public:
    void reset (float startPhase) { phase = startPhase; }

    float process (float freqHz, float sampleRate, const OscParams& p, float pwOffset)
    {
        const float dt = std::min (freqHz / sampleRate, 0.49f);
        const float pw = std::clamp (p.pulseWidth + pwOffset, 0.05f, 0.95f);
        float out = 0.0f;

        if (p.saw > 0.0f)
            out += p.saw * (2.0f * phase - 1.0f - polyBlep (phase, dt));
        if (p.pulse > 0.0f)
        {
            float sq = phase < pw ? 1.0f : -1.0f;
            sq += polyBlep (phase, dt);
            float t2 = phase - pw;
            if (t2 < 0.0f) t2 += 1.0f;
            sq -= polyBlep (t2, dt);
            out += p.pulse * sq;
        }
        if (p.tri > 0.0f)
            out += p.tri * (4.0f * std::abs (phase - 0.5f) - 1.0f);

        phase += dt;
        wrapped = phase >= 1.0f;
        if (wrapped) phase -= 1.0f;
        lastOut = out;
        return out;
    }

    float lastOut = 0.0f;
    bool wrapped = false;

private:
    float phase = 0.0f;
};

class AnalogVoice
{
public:
    explicit AnalogVoice (uint32_t seed) : rng (seed), drift1 (seed * 7 + 1), drift2 (seed * 13 + 5) {}

    void prepare (double sr)
    {
        sampleRate = (float) sr;
        for (auto* e : { &ampEnv, &filterEnv }) e->setSampleRate (sr);
        ladder.setSampleRate (sr);
        svf.setSampleRate (sr);
        hpf.setSampleRate (sr);
        drift1.setSampleRate (sr);
        drift2.setSampleRate (sr);
    }

    void noteOn (int midiNote, float velocity, const AnalogParams& p)
    {
        note = midiNote;
        vel = velocity;
        if (! ampEnv.isActive())
        {
            ladder.reset(); svf.reset(); hpf.reset();
            // Phases libres comme sur un vrai VCO (le Juno, à DCO, est plus régulier).
            osc1.reset (0.5f + 0.5f * rng.next());
            osc2.reset (0.5f + 0.5f * rng.next());
            sub.reset (0.0f);
        }
        lfoDelayCounter = 0.0f;
        updateParams (p);
        ampEnv.noteOn();
        filterEnv.noteOn();
    }

    void noteOff() { ampEnv.noteOff(); filterEnv.noteOff(); }
    void kill() { ampEnv.reset(); filterEnv.reset(); }
    bool isActive() const { return ampEnv.isActive(); }
    bool isReleasing() const { return ampEnv.getStage() == Envelope::Stage::Release; }
    int getNote() const { return note; }

    void updateParams (const AnalogParams& p)
    {
        ampEnv.set (p.ampEnv.attack, p.ampEnv.decay, p.ampEnv.sustain, p.ampEnv.release);
        filterEnv.set (p.filterEnv.attack, p.filterEnv.decay, p.filterEnv.sustain, p.filterEnv.release);
    }

    // lfo : LFO global du synthé dans [-1, 1] ; bendSemis : pitch bend.
    float process (const AnalogParams& p, float lfo, float bendSemis)
    {
        lfoDelayCounter += 1.0f / sampleRate;
        const float lfoAmt = p.lfoDelaySec > 0.0f ? std::min (1.0f, lfoDelayCounter / p.lfoDelaySec) * lfo : lfo;

        const float fenv = filterEnv.process();
        const float pitch = (float) note + bendSemis + p.lfoToPitchSemis * lfoAmt;
        const float d1 = drift1.process() * p.driftCents * 0.01f;
        const float d2 = drift2.process() * p.driftCents * 0.01f;

        const float f2 = midiToHz (pitch + p.osc2.semitones + p.osc2.cents * 0.01f + d2);
        const float o2 = osc2.process (f2, sampleRate, p.osc2, p.lfoToPwm * lfoAmt);

        if (p.oscSync && osc2.wrapped) osc1.reset (0.0f); // hard sync A sur B (version naïve)

        const float polyMod = p.polyModEnvToOscA * fenv + p.polyModOscBToOscA * o2;
        const float f1 = midiToHz (pitch + p.osc1.semitones + p.osc1.cents * 0.01f + d1 + polyMod);
        const float o1 = osc1.process (f1, sampleRate, p.osc1, p.lfoToPwm * lfoAmt);

        static const OscParams squareSub { 0.0f, 1.0f, 0.0f, 0.5f };
        const float s = p.subLevel > 0.0f ? sub.process (f1 * 0.5f, sampleRate, squareSub, 0.0f) : 0.0f;

        float x = o1 * p.osc1.level + o2 * p.osc2.level + s * p.subLevel + rng.next() * p.noiseLevel;
        x *= 0.5f;

        const float octaves = fenv * p.filterEnvOctaves * (1.0f - p.velocityToFilter + p.velocityToFilter * vel)
                            + p.keyTrack * ((float) note - 60.0f) / 12.0f
                            + p.lfoToCutoffOct * lfoAmt;
        const float cutoff = p.cutoffHz * std::exp2 (octaves);

        float y = p.filterType == FilterType::Ladder24
                    ? ladder.process (x, cutoff, p.resonance, p.drive)
                    : svf.process (x, cutoff, p.resonance, p.svfMode);
        y = hpf.process (y, p.highPassHz);

        const float velGain = 1.0f - p.velocityToAmp + p.velocityToAmp * vel;
        return y * ampEnv.process() * velGain;
    }

private:
    float sampleRate = 44100.0f;
    int note = 60;
    float vel = 1.0f, lfoDelayCounter = 0.0f;
    Rng rng;
    Drift drift1, drift2;
    Oscillator osc1, osc2, sub;
    Envelope ampEnv, filterEnv;
    LadderFilter ladder;
    StateVariableFilter svf;
    OnePoleHighPass hpf;
};
} // namespace retro
