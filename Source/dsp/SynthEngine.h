#pragma once

// Le synthé complet : allocation des voix, LFO global, chorus, macros.
// Indépendant de JUCE : le plugin et l'outil de rendu en ligne de commande l'utilisent tels quels.

#include "AnalogVoice.h"
#include "Chorus.h"
#include "FMVoice.h"
#include <array>
#include <memory>

namespace retro
{
// Réglages globaux exposés à l'utilisateur, appliqués par-dessus le preset.
struct Macros
{
    float brightness = 0.0f;   // -1..1 : cutoff (+/- 3 oct) ou index FM (x0.25..x2)
    float resonance = 0.0f;    // -1..1 : ajouté à la résonance du preset
    float attackScale = 1.0f;  // multiplie les temps d'attaque
    float releaseScale = 1.0f; // multiplie les temps de release
    int chorus = -1;           // -1 = celui du preset, sinon ChorusMode
    float driftScale = 1.0f;   // 0 = parfaitement juste, 3 = vieille machine fatiguée
    float volume = 1.0f;
};

class SynthEngine
{
public:
    static constexpr int kMaxVoices = 16;

    SynthEngine()
    {
        for (int i = 0; i < kMaxVoices; ++i)
            analog[(size_t) i] = std::make_unique<AnalogVoice> ((uint32_t) (i * 7919 + 17));
    }

    void prepare (double sr)
    {
        sampleRate = (float) sr;
        for (auto& v : analog) v->prepare (sr);
        for (auto& v : fm) v.prepare (sr);
        chorus.prepare (sr);
    }

    void setPatch (const Patch& p)
    {
        if (p.engine != preset.engine || p.polyphony != preset.polyphony)
            allNotesOff (true);
        preset = p;
        rebuild();
    }

    void setMacros (const Macros& m) { macros = m; rebuild(); }
    const Patch& getEffectivePatch() const { return effective; }

    void setPitchBend (float semis) { bendSemis = semis; }

    void noteOn (int note, float velocity)
    {
        const int poly = std::clamp (effective.polyphony, 1, kMaxVoices);
        int chosen = -1;

        for (int i = 0; i < poly && chosen < 0; ++i) // même note : on la redéclenche
            if (voiceActive (i) && voiceNote (i) == note) chosen = i;
        for (int i = 0; i < poly && chosen < 0; ++i)
            if (! voiceActive (i)) chosen = i;
        if (chosen < 0) // vol de voix : la plus ancienne en release, sinon la plus ancienne
        {
            uint64_t oldest = UINT64_MAX;
            for (int pass = 0; pass < 2 && chosen < 0; ++pass)
                for (int i = 0; i < poly; ++i)
                    if ((pass == 1 || voiceReleasing (i)) && age[(size_t) i] < oldest)
                    { oldest = age[(size_t) i]; chosen = i; }
        }

        age[(size_t) chosen] = ++ageCounter;
        if (effective.engine == Engine::FM) fm[(size_t) chosen].noteOn (note, velocity, effective.fm);
        else analog[(size_t) chosen]->noteOn (note, velocity, effective.analog);
    }

    void noteOff (int note)
    {
        for (int i = 0; i < kMaxVoices; ++i)
            if (voiceActive (i) && voiceNote (i) == note && ! voiceReleasing (i))
            {
                if (effective.engine == Engine::FM) fm[(size_t) i].noteOff();
                else analog[(size_t) i]->noteOff();
            }
    }

    void allNotesOff (bool hardKill = false)
    {
        for (int i = 0; i < kMaxVoices; ++i)
        {
            if (hardKill) { fm[(size_t) i].kill(); analog[(size_t) i]->kill(); }
            else { fm[(size_t) i].noteOff(); analog[(size_t) i]->noteOff(); }
        }
    }

    // Rend numSamples échantillons stéréo (écrase le contenu de left/right).
    void render (float* left, float* right, int numSamples)
    {
        for (int i = 0; i < kMaxVoices; ++i)
        {
            if (effective.engine == Engine::FM) fm[(size_t) i].updateParams (effective.fm);
            else analog[(size_t) i]->updateParams (effective.analog);
        }

        const float lfoRate = effective.engine == Engine::FM ? effective.fm.lfoRateHz : effective.analog.lfoRateHz;
        const float fmBrightness = std::exp2 (macros.brightness * (macros.brightness > 0 ? 1.0f : 2.0f));

        for (int n = 0; n < numSamples; ++n)
        {
            const float lfo = std::sin (kTwoPi * lfoPhase);
            lfoPhase += lfoRate / sampleRate;
            if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

            float sum = 0.0f;
            for (int i = 0; i < kMaxVoices; ++i)
            {
                if (effective.engine == Engine::FM)
                {
                    if (fm[(size_t) i].isActive())
                        sum += fm[(size_t) i].process (effective.fm, lfo, bendSemis, fmBrightness);
                }
                else if (analog[(size_t) i]->isActive())
                    sum += analog[(size_t) i]->process (effective.analog, lfo, bendSemis);
            }
            const float out = std::tanh (sum * effective.gain * macros.volume);
            left[n] = right[n] = out;
        }

        if (effective.engine == Engine::Analog)
            chorus.process (left, right, numSamples, effective.analog.chorus);
    }

private:
    bool voiceActive (int i) const
    {
        return effective.engine == Engine::FM ? fm[(size_t) i].isActive() : analog[(size_t) i]->isActive();
    }
    bool voiceReleasing (int i) const
    {
        return effective.engine == Engine::FM ? fm[(size_t) i].isReleasing() : analog[(size_t) i]->isReleasing();
    }
    int voiceNote (int i) const
    {
        return effective.engine == Engine::FM ? fm[(size_t) i].getNote() : analog[(size_t) i]->getNote();
    }

    // Applique les macros au preset pour obtenir le patch réellement joué.
    void rebuild()
    {
        effective = preset;
        auto scaleEnv = [this] (EnvParams& e)
        {
            e.attack *= macros.attackScale;
            e.release *= macros.releaseScale;
        };

        if (effective.engine == Engine::Analog)
        {
            auto& a = effective.analog;
            a.cutoffHz *= std::exp2 (3.0f * macros.brightness);
            a.resonance = std::clamp (a.resonance + macros.resonance, 0.0f, 1.0f);
            a.driftCents *= macros.driftScale;
            if (macros.chorus >= 0) a.chorus = (ChorusMode) macros.chorus;
            scaleEnv (a.ampEnv);
            scaleEnv (a.filterEnv);
        }
        else
        {
            for (auto& op : effective.fm.ops) scaleEnv (op.env);
        }
    }

    float sampleRate = 44100.0f, lfoPhase = 0.0f, bendSemis = 0.0f;
    Patch preset, effective;
    Macros macros;
    std::array<std::unique_ptr<AnalogVoice>, kMaxVoices> analog;
    std::array<FMVoice, kMaxVoices> fm;
    std::array<uint64_t, kMaxVoices> age {};
    uint64_t ageCounter = 0;
    JunoChorus chorus;
};
} // namespace retro
