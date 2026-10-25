// Rend chaque preset d'usine dans un fichier WAV, sans JUCE ni DAW.
// Usage : render_presets <dossier_de_sortie>

#include "../Source/dsp/Presets.h"
#include "../Source/dsp/SynthEngine.h"
#include <cstdio>
#include <string>
#include <vector>

static void writeWav (const std::string& path, const std::vector<float>& l, const std::vector<float>& r, int sr)
{
    FILE* f = std::fopen (path.c_str(), "wb");
    if (! f) return;
    const uint32_t frames = (uint32_t) l.size(), dataBytes = frames * 4;
    auto u32 = [f] (uint32_t v) { std::fwrite (&v, 4, 1, f); };
    auto u16 = [f] (uint16_t v) { std::fwrite (&v, 2, 1, f); };
    std::fwrite ("RIFF", 1, 4, f); u32 (36 + dataBytes); std::fwrite ("WAVEfmt ", 1, 8, f);
    u32 (16); u16 (1); u16 (2); u32 ((uint32_t) sr); u32 ((uint32_t) sr * 4); u16 (4); u16 (16);
    std::fwrite ("data", 1, 4, f); u32 (dataBytes);
    for (uint32_t i = 0; i < frames; ++i)
        for (float s : { l[i], r[i] })
        {
            const int16_t v = (int16_t) std::lround (std::clamp (s, -1.0f, 1.0f) * 32767.0f);
            std::fwrite (&v, 2, 1, f);
        }
    std::fclose (f);
}

int main (int argc, char** argv)
{
    const std::string outDir = argc > 1 ? argv[1] : ".";
    const int sr = 48000, block = 256;
    int failures = 0;

    // Petite phrase : un accord tenu, puis trois notes graves détachées.
    struct Event { float time; bool on; int note; float vel; };
    const std::vector<Event> phrase = {
        { 0.0f, true, 60, 0.8f }, { 0.0f, true, 64, 0.8f }, { 0.0f, true, 67, 0.8f },
        { 1.6f, false, 60, 0 }, { 1.6f, false, 64, 0 }, { 1.6f, false, 67, 0 },
        { 2.2f, true, 36, 1.0f }, { 2.5f, false, 36, 0 },
        { 2.6f, true, 43, 0.6f }, { 2.9f, false, 43, 0 },
        { 3.0f, true, 48, 0.9f }, { 3.5f, false, 48, 0 },
    };

    for (const auto& patch : retro::factoryPresets())
    {
        retro::SynthEngine synth;
        synth.prepare (sr);
        synth.setPatch (patch);

        const int total = (int) (5.5f * sr);
        std::vector<float> L ((size_t) total), R ((size_t) total);
        size_t ev = 0;
        for (int pos = 0; pos < total; pos += block)
        {
            while (ev < phrase.size() && phrase[ev].time * sr <= pos)
            {
                const auto& e = phrase[ev++];
                if (e.on) synth.noteOn (e.note, e.vel); else synth.noteOff (e.note);
            }
            synth.render (&L[(size_t) pos], &R[(size_t) pos], std::min (block, total - pos));
        }

        float peak = 0.0f; double rms = 0.0; bool finite = true;
        for (size_t i = 0; i < L.size(); ++i)
        {
            finite = finite && std::isfinite (L[i]) && std::isfinite (R[i]);
            peak = std::max ({ peak, std::abs (L[i]), std::abs (R[i]) });
            rms += L[i] * L[i];
        }
        rms = std::sqrt (rms / (double) L.size());
        const bool ok = finite && peak > 0.01f;
        failures += ok ? 0 : 1;

        std::string file = patch.name;
        for (auto& c : file) if (c == ' ' || c == '.') c = '_';
        writeWav (outDir + "/" + file + ".wav", L, R, sr);
        std::printf ("%-24s %-10s pic %.2f  rms %.3f  %s\n", patch.name, retro::modelName (patch.model),
                     peak, rms, ok ? "OK" : "ÉCHEC");
    }
    return failures == 0 ? 0 : 1;
}
