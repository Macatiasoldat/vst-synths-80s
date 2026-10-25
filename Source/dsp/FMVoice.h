#pragma once

// Voix FM 6 opérateurs inspirée du Yamaha DX7.
// Chaque opérateur est un sinus avec son enveloppe ; l'algorithme décide qui module qui.

#include "Common.h"
#include "Patch.h"

namespace retro
{
struct FMAlgorithm
{
    int number;
    uint8_t modulators[6]; // bit j à 1 : l'opérateur j module l'opérateur i
    uint8_t carriers;      // bit i à 1 : l'opérateur i est audible
    int feedbackOp;
};

// Sur le DX7, un modulateur a toujours un numéro plus grand que sa cible,
// donc on peut calculer les opérateurs de OP6 vers OP1 en un seul passage.
inline const FMAlgorithm* findAlgorithm (int number)
{
    constexpr uint8_t b0 = 1, b1 = 2, b2 = 4, b3 = 8, b4 = 16, b5 = 32;
    static const FMAlgorithm algorithms[] = {
        //  n   OP1  OP2  OP3  OP4  OP5  OP6   porteuses       feedback
        {  1, { b1,  0,   b3,  b4,  b5,  0 }, b0 | b2,                   5 }, // 2>1, 6>5>4>3
        {  2, { b1,  0,   b3,  b4,  b5,  0 }, b0 | b2,                   1 }, // idem, feedback OP2
        {  5, { b1,  0,   b3,  0,   b5,  0 }, b0 | b2 | b4,              5 }, // trois paires (E.Piano)
        { 16, { b1 | b2 | b4, 0, b3, 0, b5, 0 }, b0,                    5 }, // une porteuse, trois branches
        { 22, { b1,  0,   b5,  b5,  b5,  0 }, b0 | b2 | b3 | b4,         5 }, // 2>1, 6>(3,4,5) (cuivres)
        { 32, { 0,   0,   0,   0,   0,   0 }, b0 | b1 | b2 | b3 | b4 | b5, 5 }, // additif (orgue)
    };
    for (auto& a : algorithms)
        if (a.number == number) return &a;
    return &algorithms[2];
}

class FMVoice
{
public:
    void prepare (double sr)
    {
        sampleRate = (float) sr;
        for (auto& e : env) e.setSampleRate (sr);
    }

    void noteOn (int midiNote, float velocity, const FMParams& p)
    {
        note = midiNote;
        vel = velocity;
        if (! isActive())
        {
            for (auto& ph : phase) ph = 0.0f; // le DX7 redémarre ses phases à chaque note
            fb1 = fb2 = 0.0f;
        }
        updateParams (p);
        for (auto& e : env) e.noteOn();
    }

    void noteOff() { for (auto& e : env) e.noteOff(); }
    void kill() { for (auto& e : env) e.reset(); }
    int getNote() const { return note; }

    bool isActive() const
    {
        const auto* algo = findAlgorithm (currentAlgorithm);
        for (int i = 0; i < 6; ++i)
            if ((algo->carriers >> i) & 1)
                if (env[i].isActive()) return true;
        return false;
    }

    bool isReleasing() const { return env[0].getStage() == Envelope::Stage::Release; }

    void updateParams (const FMParams& p)
    {
        currentAlgorithm = p.algorithm;
        for (int i = 0; i < 6; ++i)
            env[i].set (p.ops[i].env.attack, p.ops[i].env.decay, p.ops[i].env.sustain, p.ops[i].env.release);
    }

    // brightness : multiplie l'index des modulateurs (macro « Brillance »).
    float process (const FMParams& p, float lfo, float bendSemis, float brightness)
    {
        const auto* algo = findAlgorithm (p.algorithm);
        const float baseHz = midiToHz ((float) note + bendSemis + p.lfoToPitchSemis * lfo);
        float out[6] {};
        float mix = 0.0f;
        int carrierCount = 0;

        for (int i = 5; i >= 0; --i)
        {
            const auto& op = p.ops[i];
            float mod = 0.0f;
            for (int j = i + 1; j < 6; ++j)
                if ((algo->modulators[i] >> j) & 1)
                    mod += out[j];
            if (i == algo->feedbackOp)
                mod += p.feedback * 0.5f * (fb1 + fb2);

            const float s = std::sin (kTwoPi * (phase[i] + mod));
            const float velGain = 1.0f - op.velocitySens + op.velocitySens * vel;
            const bool isCarrier = (algo->carriers >> i) & 1;
            const float level = isCarrier ? op.level : op.level * brightness;
            out[i] = s * env[i].process() * level * velGain;

            if (i == algo->feedbackOp) { fb2 = fb1; fb1 = out[i]; }

            const float freq = baseHz * op.ratio * std::exp2 (op.detuneCents / 1200.0f);
            phase[i] += freq / sampleRate;
            phase[i] -= std::floor (phase[i]);

            if (isCarrier) { mix += out[i]; ++carrierCount; }
        }
        return carrierCount > 0 ? mix / std::sqrt ((float) carrierCount) : 0.0f;
    }

private:
    float sampleRate = 44100.0f;
    int note = 60, currentAlgorithm = 5;
    float vel = 1.0f, fb1 = 0.0f, fb2 = 0.0f;
    float phase[6] {};
    Envelope env[6];
};
} // namespace retro
