#pragma once

// Banque d'usine : quelques sons emblématiques par machine.
// Les valeurs sont des approximations à l'oreille, à affiner en A/B avec les originaux.

#include "Patch.h"
#include <vector>

namespace retro
{
namespace presets_detail
{
inline Patch analogBase (const char* name, Model model, int poly)
{
    Patch p;
    p.name = name;
    p.model = model;
    p.engine = Engine::Analog;
    p.polyphony = poly;
    p.gain = 0.7f;
    return p;
}

inline Patch fmBase (const char* name, int algorithm)
{
    Patch p;
    p.name = name;
    p.model = Model::DX7;
    p.engine = Engine::FM;
    p.polyphony = 16;
    p.fm.algorithm = algorithm;
    p.gain = 0.4f;
    return p;
}

inline FMOperator op (float ratio, float level, EnvParams env, float detune = 0.0f, float velSens = 0.5f)
{
    FMOperator o;
    o.ratio = ratio; o.level = level; o.env = env; o.detuneCents = detune; o.velocitySens = velSens;
    return o;
}
} // namespace presets_detail

inline std::vector<Patch> factoryPresets()
{
    using namespace presets_detail;
    std::vector<Patch> bank;

    // ---------------------------------------------------------------- DX7 (FM)
    {
        auto p = fmBase ("DX7 E.Piano 1", 5);
        p.fm.feedback = 0.3f;
        p.fm.ops[0] = op (1.0f, 1.0f,   { 0.001f, 3.5f, 0.0f, 0.4f }, +3.0f, 0.6f);
        p.fm.ops[1] = op (1.0f, 0.35f,  { 0.001f, 2.0f, 0.0f, 0.4f }, 0.0f, 0.8f);
        p.fm.ops[2] = op (1.0f, 0.8f,   { 0.001f, 4.0f, 0.0f, 0.4f }, -3.0f, 0.6f);
        p.fm.ops[3] = op (14.0f, 0.18f, { 0.001f, 0.35f, 0.0f, 0.2f }, 0.0f, 1.0f); // le « tine »
        p.fm.ops[4] = op (1.0f, 0.5f,   { 0.001f, 3.0f, 0.0f, 0.4f }, +7.0f, 0.4f);
        p.fm.ops[5] = op (1.0f, 0.25f,  { 0.001f, 1.5f, 0.0f, 0.4f }, 0.0f, 0.7f);
        bank.push_back (p);
    }
    {
        auto p = fmBase ("DX7 Brass", 22);
        p.fm.feedback = 0.6f;
        p.fm.lfoRateHz = 5.5f; p.fm.lfoToPitchSemis = 0.04f;
        p.fm.ops[0] = op (1.0f, 0.9f,  { 0.06f, 0.4f, 0.85f, 0.25f }, 0.0f, 0.3f);
        p.fm.ops[1] = op (1.0f, 0.55f, { 0.12f, 0.6f, 0.6f, 0.25f }, 0.0f, 0.7f);
        p.fm.ops[2] = op (1.0f, 0.8f,  { 0.05f, 0.4f, 0.85f, 0.25f }, +5.0f, 0.3f);
        p.fm.ops[3] = op (1.0f, 0.8f,  { 0.05f, 0.4f, 0.85f, 0.25f }, -5.0f, 0.3f);
        p.fm.ops[4] = op (1.0f, 0.7f,  { 0.05f, 0.4f, 0.85f, 0.25f }, +9.0f, 0.3f);
        p.fm.ops[5] = op (1.0f, 0.45f, { 0.10f, 0.5f, 0.7f, 0.25f }, 0.0f, 0.7f);
        bank.push_back (p);
    }
    {
        auto p = fmBase ("DX7 Solid Bass", 16);
        p.fm.feedback = 0.5f;
        p.polyphony = 4;
        p.fm.ops[0] = op (0.5f, 1.0f,  { 0.001f, 1.2f, 0.4f, 0.08f }, 0.0f, 0.4f);
        p.fm.ops[1] = op (0.5f, 0.6f,  { 0.001f, 0.25f, 0.1f, 0.08f }, 0.0f, 0.9f);
        p.fm.ops[2] = op (1.0f, 0.4f,  { 0.001f, 0.15f, 0.0f, 0.08f }, 0.0f, 0.9f);
        p.fm.ops[3] = op (3.0f, 0.3f,  { 0.001f, 0.08f, 0.0f, 0.08f }, 0.0f, 0.9f);
        p.fm.ops[4] = op (0.5f, 0.3f,  { 0.001f, 0.6f, 0.2f, 0.08f }, 0.0f, 0.6f);
        p.fm.ops[5] = op (1.0f, 0.3f,  { 0.001f, 0.4f, 0.1f, 0.08f }, 0.0f, 0.6f);
        bank.push_back (p);
    }
    {
        auto p = fmBase ("DX7 Tubular Bells", 5);
        p.fm.feedback = 0.2f;
        p.fm.ops[0] = op (1.0f, 0.9f,   { 0.001f, 7.0f, 0.0f, 3.0f }, 0.0f, 0.5f);
        p.fm.ops[1] = op (3.5f, 0.45f,  { 0.001f, 5.0f, 0.0f, 3.0f }, 0.0f, 0.7f);
        p.fm.ops[2] = op (1.0f, 0.7f,   { 0.001f, 6.0f, 0.0f, 3.0f }, +4.0f, 0.5f);
        p.fm.ops[3] = op (3.5f, 0.4f,   { 0.001f, 4.0f, 0.0f, 3.0f }, +2.0f, 0.7f);
        p.fm.ops[4] = op (2.0f, 0.35f,  { 0.001f, 2.5f, 0.0f, 2.0f }, 0.0f, 0.5f);
        p.fm.ops[5] = op (5.19f, 0.35f, { 0.001f, 0.8f, 0.0f, 1.0f }, 0.0f, 0.8f);
        bank.push_back (p);
    }

    // ---------------------------------------------------------------- Juno-60 : 1 DCO (scie + carré + sub), HPF, chorus
    {
        auto p = analogBase ("Juno Chorus Pad", Model::Juno60, 6);
        auto& a = p.analog;
        a.osc1 = { 1.0f, 0.6f, 0.0f, 0.5f };
        a.osc2.level = 0.0f;
        a.subLevel = 0.4f;
        a.lfoRateHz = 0.6f; a.lfoToPwm = 0.35f;
        a.cutoffHz = 900.0f; a.resonance = 0.15f; a.filterEnvOctaves = 1.0f; a.keyTrack = 0.6f;
        a.highPassHz = 120.0f;
        a.ampEnv = { 0.6f, 1.0f, 0.9f, 1.4f };
        a.filterEnv = { 0.8f, 2.0f, 0.5f, 1.5f };
        a.driftCents = 0.5f; // DCO : très stable
        a.chorus = ChorusMode::II;
        bank.push_back (p);
    }
    {
        auto p = analogBase ("Juno Sub Bass", Model::Juno60, 6);
        auto& a = p.analog;
        a.osc1 = { 0.0f, 1.0f, 0.0f, 0.35f };
        a.osc2.level = 0.0f;
        a.subLevel = 1.0f;
        a.cutoffHz = 250.0f; a.resonance = 0.35f; a.filterEnvOctaves = 3.0f; a.keyTrack = 0.3f;
        a.ampEnv = { 0.002f, 0.4f, 0.7f, 0.1f };
        a.filterEnv = { 0.001f, 0.25f, 0.1f, 0.1f };
        a.driftCents = 0.5f;
        a.chorus = ChorusMode::Off;
        bank.push_back (p);
    }
    {
        auto p = analogBase ("Juno Poly Stab", Model::Juno60, 6);
        auto& a = p.analog;
        a.osc1 = { 1.0f, 0.0f, 0.0f, 0.5f };
        a.osc2.level = 0.0f;
        a.subLevel = 0.3f;
        a.cutoffHz = 400.0f; a.resonance = 0.45f; a.filterEnvOctaves = 4.0f;
        a.ampEnv = { 0.001f, 0.35f, 0.0f, 0.25f };
        a.filterEnv = { 0.001f, 0.2f, 0.0f, 0.2f };
        a.driftCents = 0.5f;
        a.chorus = ChorusMode::I;
        bank.push_back (p);
    }

    // ---------------------------------------------------------------- Jupiter-8 : 2 VCO, filtre 24 dB, 8 voix
    {
        auto p = analogBase ("Jupiter Strings", Model::Jupiter8, 8);
        auto& a = p.analog;
        a.osc1 = { 1.0f, 0.0f, 0.0f, 0.5f };
        a.osc2 = { 1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 9.0f, 0.9f };
        a.lfoRateHz = 5.2f; a.lfoToPitchSemis = 0.06f; a.lfoDelaySec = 0.8f;
        a.cutoffHz = 1400.0f; a.resonance = 0.1f; a.filterEnvOctaves = 0.8f; a.keyTrack = 0.7f;
        a.highPassHz = 150.0f;
        a.ampEnv = { 0.35f, 1.0f, 0.85f, 1.2f };
        a.filterEnv = { 0.5f, 1.5f, 0.6f, 1.2f };
        a.driftCents = 3.0f;
        a.chorus = ChorusMode::I; // le Jupiter n'a pas de chorus interne, mais il passait toujours dans un
        bank.push_back (p);
    }
    {
        auto p = analogBase ("Jupiter Poly Brass", Model::Jupiter8, 8);
        auto& a = p.analog;
        a.osc1 = { 1.0f, 0.0f, 0.0f, 0.5f };
        a.osc2 = { 1.0f, 0.0f, 0.0f, 0.5f, 0.0f, -7.0f, 1.0f };
        a.cutoffHz = 500.0f; a.resonance = 0.15f; a.filterEnvOctaves = 3.2f; a.keyTrack = 0.5f;
        a.ampEnv = { 0.03f, 0.5f, 0.8f, 0.3f };
        a.filterEnv = { 0.08f, 0.6f, 0.45f, 0.3f };
        a.driftCents = 3.0f;
        bank.push_back (p);
    }

    // ---------------------------------------------------------------- OB-Xa : filtre SEM 12 dB, gros désaccord
    {
        auto p = analogBase ("OB Brass Stab 84", Model::OberheimOBXa, 8);
        auto& a = p.analog;
        a.osc1 = { 1.0f, 0.0f, 0.0f, 0.5f };
        a.osc2 = { 1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 12.0f, 1.0f };
        a.filterType = FilterType::SVF12;
        a.cutoffHz = 700.0f; a.resonance = 0.25f; a.filterEnvOctaves = 3.5f; a.keyTrack = 0.5f;
        a.ampEnv = { 0.005f, 0.8f, 0.75f, 0.35f };
        a.filterEnv = { 0.01f, 0.45f, 0.35f, 0.35f };
        a.driftCents = 6.0f;
        a.drive = 1.5f;
        bank.push_back (p);
    }
    {
        auto p = analogBase ("OB Fat Pad", Model::OberheimOBXa, 8);
        auto& a = p.analog;
        a.osc1 = { 1.0f, 0.0f, 0.0f, 0.5f };
        a.osc2 = { 0.0f, 1.0f, 0.0f, 0.4f, 0.0f, 14.0f, 0.8f };
        a.filterType = FilterType::SVF12;
        a.svfMode = 0.15f;
        a.lfoRateHz = 0.3f; a.lfoToCutoffOct = 0.4f; a.lfoToPwm = 0.2f;
        a.cutoffHz = 1100.0f; a.resonance = 0.3f; a.filterEnvOctaves = 1.0f;
        a.ampEnv = { 0.9f, 1.0f, 1.0f, 2.0f };
        a.filterEnv = { 1.2f, 2.0f, 0.6f, 2.0f };
        a.driftCents = 8.0f;
        a.chorus = ChorusMode::Off;
        bank.push_back (p);
    }

    // ---------------------------------------------------------------- Prophet-5 : 2 VCO, filtre 24 dB, Poly-Mod, 5 voix
    {
        auto p = analogBase ("Prophet Poly Sweep", Model::Prophet5, 5);
        auto& a = p.analog;
        a.osc1 = { 1.0f, 0.5f, 0.0f, 0.4f };
        a.osc2 = { 1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 6.0f, 0.9f };
        a.cutoffHz = 300.0f; a.resonance = 0.55f; a.filterEnvOctaves = 4.5f; a.keyTrack = 0.5f;
        a.ampEnv = { 0.01f, 1.5f, 0.7f, 1.0f };
        a.filterEnv = { 1.5f, 2.5f, 0.2f, 1.2f };
        a.driftCents = 4.0f;
        bank.push_back (p);
    }
    {
        auto p = analogBase ("Prophet PolyMod Bell", Model::Prophet5, 5);
        auto& a = p.analog;
        a.osc1 = { 0.0f, 0.0f, 1.0f, 0.5f };
        a.osc2 = { 0.0f, 0.0f, 1.0f, 0.5f, 19.0f, 0.0f, 0.3f };
        a.polyModOscBToOscA = 9.0f;  // FM « analogique » de B vers A
        a.polyModEnvToOscA = 4.0f;
        a.cutoffHz = 2500.0f; a.resonance = 0.2f; a.filterEnvOctaves = 1.5f; a.keyTrack = 1.0f;
        a.ampEnv = { 0.001f, 2.5f, 0.0f, 1.8f };
        a.filterEnv = { 0.001f, 0.6f, 0.0f, 0.6f };
        a.driftCents = 3.0f;
        bank.push_back (p);
    }
    {
        auto p = analogBase ("Prophet Sync Lead", Model::Prophet5, 5);
        auto& a = p.analog;
        a.osc1 = { 1.0f, 0.0f, 0.0f, 0.5f };
        a.osc1.semitones = 7.0f;
        a.osc2 = { 1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.35f };
        a.oscSync = true;
        a.polyModEnvToOscA = 14.0f;  // le balayage classique du sync
        a.lfoRateHz = 5.8f; a.lfoToPitchSemis = 0.15f; a.lfoDelaySec = 0.5f;
        a.cutoffHz = 1500.0f; a.resonance = 0.35f; a.filterEnvOctaves = 2.0f; a.keyTrack = 0.8f;
        a.ampEnv = { 0.005f, 0.5f, 0.9f, 0.3f };
        a.filterEnv = { 0.001f, 0.35f, 0.3f, 0.3f };
        a.driftCents = 4.0f;
        bank.push_back (p);
    }

    return bank;
}
} // namespace retro
