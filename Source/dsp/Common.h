#pragma once

// Briques DSP communes, sans dépendance à JUCE (testables en ligne de commande).

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace retro
{
constexpr float kPi = 3.14159265358979f;
constexpr float kTwoPi = 2.0f * kPi;

inline float midiToHz (float note) { return 440.0f * std::exp2 ((note - 69.0f) / 12.0f); }

// Générateur pseudo-aléatoire rapide, sortie dans [-1, 1).
struct Rng
{
    uint32_t state = 22222;
    explicit Rng (uint32_t seed = 22222) : state (seed) {}
    float next()
    {
        state = state * 1664525u + 1013904223u;
        return (float) (state >> 8) * (2.0f / 16777216.0f) - 1.0f;
    }
};

// ADSR « analogique » : attaque linéaire, decay et release exponentiels.
class Envelope
{
public:
    enum class Stage { Idle, Attack, Decay, Sustain, Release };

    void setSampleRate (double sr) { sampleRate = (float) sr; }

    void set (float attackSec, float decaySec, float sustainLevel, float releaseSec)
    {
        attackInc = 1.0f / std::max (1.0f, attackSec * sampleRate);
        decayCoef = coefFor (decaySec);
        sustain = std::clamp (sustainLevel, 0.0f, 1.0f);
        releaseCoef = coefFor (releaseSec);
    }

    void noteOn() { stage = Stage::Attack; }
    void noteOff() { if (stage != Stage::Idle) stage = Stage::Release; }
    void reset() { stage = Stage::Idle; value = 0.0f; }
    bool isActive() const { return stage != Stage::Idle; }
    Stage getStage() const { return stage; }

    float process()
    {
        switch (stage)
        {
            case Stage::Attack:
                value += attackInc;
                if (value >= 1.0f) { value = 1.0f; stage = Stage::Decay; }
                break;
            case Stage::Decay:
                value = sustain + (value - sustain) * decayCoef;
                break;
            case Stage::Sustain:
                value = sustain;
                break;
            case Stage::Release:
                value *= releaseCoef;
                if (value < 1.0e-5f) { value = 0.0f; stage = Stage::Idle; }
                break;
            case Stage::Idle:
                break;
        }
        return value;
    }

private:
    // Atteint ~1 % de l'écart après « seconds ».
    float coefFor (float seconds) const { return std::exp (-4.6f / std::max (1.0f, seconds * sampleRate)); }

    Stage stage = Stage::Idle;
    float sampleRate = 44100.0f;
    float value = 0.0f, attackInc = 0.01f, decayCoef = 0.999f, sustain = 1.0f, releaseCoef = 0.999f;
};

// Dérive lente de l'accord, façon VCO qui chauffe (bruit filtré très bas).
class Drift
{
public:
    explicit Drift (uint32_t seed) : rng (seed) {}
    void setSampleRate (double sr) { coef = 1.0f - std::exp (-kTwoPi * 0.7f / (float) sr); }
    float process() // ~[-1, 1]
    {
        target += (rng.next() * 0.02f - target * 0.0005f);
        target = std::clamp (target, -1.0f, 1.0f);
        value += (target - value) * coef;
        return value;
    }

private:
    Rng rng;
    float target = 0.0f, value = 0.0f, coef = 0.0001f;
};
} // namespace retro
