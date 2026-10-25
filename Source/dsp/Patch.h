#pragma once

// Description d'un son. Chaque modèle de synthé est un « Patch » qui configure
// l'un des deux moteurs (FM ou soustractif) avec le caractère de la machine d'origine.

namespace retro
{
enum class Model { DX7, Juno60, Jupiter8, OberheimOBXa, Prophet5 };
enum class Engine { FM, Analog };
enum class FilterType { Ladder24, SVF12 };
enum class ChorusMode { Off, I, II, Both };

inline const char* modelName (Model m)
{
    switch (m)
    {
        case Model::DX7: return "DX7";
        case Model::Juno60: return "Juno-60";
        case Model::Jupiter8: return "Jupiter-8";
        case Model::OberheimOBXa: return "OB-Xa";
        case Model::Prophet5: return "Prophet-5";
    }
    return "?";
}

struct EnvParams { float attack = 0.005f, decay = 0.3f, sustain = 0.8f, release = 0.3f; };

struct OscParams
{
    float saw = 1.0f, pulse = 0.0f, tri = 0.0f; // formes d'onde mixables (Juno, Prophet)
    float pulseWidth = 0.5f;
    float semitones = 0.0f, cents = 0.0f;
    float level = 1.0f;
};

struct AnalogParams
{
    OscParams osc1, osc2;
    float subLevel = 0.0f, noiseLevel = 0.0f;

    FilterType filterType = FilterType::Ladder24;
    float cutoffHz = 2000.0f, resonance = 0.2f;
    float filterEnvOctaves = 2.0f;   // profondeur de l'enveloppe de filtre
    float keyTrack = 0.5f;           // 1 = le filtre suit le clavier
    float svfMode = 0.0f;            // Oberheim : LP -> notch -> HP
    float drive = 1.0f;
    float highPassHz = 0.0f;         // HPF du Juno

    EnvParams ampEnv, filterEnv;
    float velocityToAmp = 0.5f, velocityToFilter = 0.3f;

    float lfoRateHz = 5.0f, lfoToPitchSemis = 0.0f, lfoToPwm = 0.0f, lfoToCutoffOct = 0.0f;
    float lfoDelaySec = 0.0f;

    // Poly-Mod du Prophet-5 : env. filtre et osc B modulent la fréquence de l'osc A.
    float polyModEnvToOscA = 0.0f;   // demi-tons
    float polyModOscBToOscA = 0.0f;  // demi-tons
    bool oscSync = false;            // osc A synchronisé sur osc B

    float driftCents = 3.0f;         // instabilité des VCO
    ChorusMode chorus = ChorusMode::Off;
};

struct FMOperator
{
    float ratio = 1.0f, detuneCents = 0.0f;
    float level = 0.0f;              // porteuse : amplitude ; modulatrice : index (en cycles)
    EnvParams env { 0.001f, 1.0f, 1.0f, 0.3f };
    float velocitySens = 0.5f;
};

struct FMParams
{
    int algorithm = 5;               // numérotation DX7 (1, 2, 5, 16, 22, 32 implémentés)
    float feedback = 0.0f;
    FMOperator ops[6];               // ops[0] = OP1 ... ops[5] = OP6
    float lfoRateHz = 5.0f, lfoToPitchSemis = 0.0f;
};

struct Patch
{
    const char* name = "Init";
    Model model = Model::Prophet5;
    Engine engine = Engine::Analog;
    int polyphony = 8;               // nombre de voix de la machine d'origine
    AnalogParams analog;
    FMParams fm;
    float gain = 0.5f;
};
} // namespace retro
