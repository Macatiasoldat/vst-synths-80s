#pragma once

// Chorus BBD du Juno-60 : une ligne à retard modulée par un triangle,
// sortie gauche/droite en opposition de phase pour l'effet d'ampleur.
// Mode I ≈ 0,5 Hz, mode II ≈ 0,83 Hz, I+II = vibrato rapide (~9,75 Hz, faible profondeur).

#include "Common.h"
#include "Patch.h"
#include <vector>

namespace retro
{
class JunoChorus
{
public:
    void prepare (double sr)
    {
        sampleRate = (float) sr;
        buffer.assign ((size_t) (sr * 0.02) + 4, 0.0f);
        writePos = 0;
        lfoPhase = 0.0f;
        lp = 0.0f;
    }

    void process (float* left, float* right, int numSamples, ChorusMode mode)
    {
        if (mode == ChorusMode::Off || buffer.empty())
            return;

        float rate = 0.513f, minMs = 1.66f, maxMs = 5.35f;
        if (mode == ChorusMode::II) { rate = 0.863f; }
        if (mode == ChorusMode::Both) { rate = 9.75f; minMs = 3.0f; maxMs = 3.6f; }

        const float centre = 0.5f * (minMs + maxMs) * 0.001f * sampleRate;
        const float depth = 0.5f * (maxMs - minMs) * 0.001f * sampleRate;
        const float lpCoef = 1.0f - std::exp (-kTwoPi * 7000.0f / sampleRate); // BBD sombre
        const int size = (int) buffer.size();

        for (int n = 0; n < numSamples; ++n)
        {
            const float in = 0.5f * (left[n] + right[n]);
            lp += (in - lp) * lpCoef;
            buffer[(size_t) writePos] = lp;

            const float tri = 4.0f * std::abs (lfoPhase - 0.5f) - 1.0f;
            lfoPhase += rate / sampleRate;
            if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

            const float wetL = read (centre + depth * tri, size);
            const float wetR = read (centre - depth * tri, size);
            left[n] = left[n] * 0.7f + wetL * 0.7f;
            right[n] = right[n] * 0.7f + wetR * 0.7f;

            if (++writePos >= size) writePos = 0;
        }
    }

private:
    float read (float delaySamples, int size) const
    {
        float pos = (float) writePos - delaySamples;
        while (pos < 0.0f) pos += (float) size;
        const int i0 = (int) pos;
        const int i1 = (i0 + 1) % size;
        const float frac = pos - (float) i0;
        return buffer[(size_t) i0] + frac * (buffer[(size_t) i1] - buffer[(size_t) i0]);
    }

    float sampleRate = 44100.0f;
    std::vector<float> buffer;
    int writePos = 0;
    float lfoPhase = 0.0f, lp = 0.0f;
};
} // namespace retro
