#pragma once

// Filtres « zero-delay feedback » (topologie TPT, cf. Zavalishin) :
//  - LadderFilter : passe-bas 4 pôles 24 dB/oct (Prophet-5, Jupiter-8, Juno-60)
//  - StateVariableFilter : 2 pôles 12 dB/oct à mode variable (Oberheim SEM / OB-Xa)

#include "Common.h"

namespace retro
{
class LadderFilter
{
public:
    void setSampleRate (double sr) { sampleRate = (float) sr; }
    void reset() { s[0] = s[1] = s[2] = s[3] = 0.0f; }

    // resonance dans [0, 1] ; auto-oscillation vers 1.
    float process (float x, float cutoffHz, float resonance, float drive)
    {
        const float fc = std::clamp (cutoffHz, 20.0f, sampleRate * 0.45f);
        const float g = std::tan (kPi * fc / sampleRate);
        const float G = g / (1.0f + g);
        const float k = 4.0f * std::clamp (resonance, 0.0f, 1.0f);
        const float inv = 1.0f / (1.0f + g);

        const float S = G * G * G * s[0] * inv + G * G * s[1] * inv + G * s[2] * inv + s[3] * inv;
        const float G4 = G * G * G * G;
        float u = (x * (1.0f + 0.5f * k) - k * S) / (1.0f + k * G4); // compensation partielle des basses
        u = std::tanh (u * drive) / drive;

        float y = u;
        for (int i = 0; i < 4; ++i)
        {
            const float v = G * (y - s[i]);
            y = v + s[i];
            s[i] = y + v;
        }
        return y;
    }

private:
    float sampleRate = 44100.0f;
    float s[4] {};
};

class StateVariableFilter
{
public:
    void setSampleRate (double sr) { sampleRate = (float) sr; }
    void reset() { ic1 = ic2 = 0.0f; }

    // mode : 0 = passe-bas, 0.5 = notch, 1 = passe-haut (le fameux bouton du SEM).
    float process (float x, float cutoffHz, float resonance, float mode)
    {
        const float fc = std::clamp (cutoffHz, 20.0f, sampleRate * 0.45f);
        const float g = std::tan (kPi * fc / sampleRate);
        const float k = 2.0f - 1.96f * std::clamp (resonance, 0.0f, 1.0f);
        const float a1 = 1.0f / (1.0f + g * (g + k));
        const float a2 = g * a1;
        const float a3 = g * a2;

        const float v3 = x - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;
        ic1 = std::tanh (ic1); // légère saturation de l'intégrateur

        const float low = v2;
        const float high = x - k * v1 - v2;
        const float notch = low + high;
        if (mode < 0.5f)
            return low + (notch - low) * (mode * 2.0f);
        return notch + (high - notch) * (mode * 2.0f - 1.0f);
    }

private:
    float sampleRate = 44100.0f;
    float ic1 = 0.0f, ic2 = 0.0f;
};

class OnePoleHighPass
{
public:
    void setSampleRate (double sr) { sampleRate = (float) sr; }
    void reset() { z = 0.0f; }
    float process (float x, float cutoffHz)
    {
        if (cutoffHz <= 0.0f)
            return x;
        const float a = std::exp (-kTwoPi * cutoffHz / sampleRate);
        z = x + a * (z - x); // passe-bas
        return x - z;
    }

private:
    float sampleRate = 44100.0f;
    float z = 0.0f;
};
} // namespace retro
