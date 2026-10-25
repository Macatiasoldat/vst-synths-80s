# Retro80 — prototype de VST des synthés des années 80

Un plugin instrument (VST3, AU sur Mac, et une appli autonome) qui reproduit le caractère de cinq machines :
**Yamaha DX7, Roland Juno-60, Roland Jupiter-8, Oberheim OB-Xa, Sequential Prophet-5**.

## Architecture

```
Source/
  dsp/                 ← tout le son, en C++ pur (aucune dépendance à JUCE)
    Common.h           enveloppes ADSR, dérive des VCO, bruit
    Filters.h          filtre ladder 24 dB (Prophet/Jupiter/Juno), filtre SEM 12 dB à mode variable (Oberheim), HPF
    AnalogVoice.h      voix soustractive : 2 oscillateurs PolyBLEP, sub, bruit, Poly-Mod, sync, 2 enveloppes
    FMVoice.h          voix FM 6 opérateurs façon DX7 (algorithmes 1, 2, 5, 16, 22, 32 + feedback)
    Chorus.h           chorus BBD du Juno (modes I, II, I+II)
    SynthEngine.h      polyphonie (nombre de voix fidèle à chaque machine), vol de voix, LFO, macros
    Patch.h            description d'un son
    Presets.h          banque d'usine (14 sons)
  PluginProcessor.*    adaptateur JUCE : MIDI, paramètres automatisables, sauvegarde d'état
  PluginEditor.*       interface provisoire (réglages + clavier à l'écran)
tools/render_presets.cpp   rend chaque preset en .wav, sans DAW
```

Deux moteurs seulement : un moteur **FM** pour le DX7 et un moteur **soustractif** partagé par les quatre analogiques.
Ce qui fait l'identité de chaque machine est dans le preset : type de filtre, sub-oscillateur et chorus du Juno,
Poly-Mod et sync du Prophet, filtre 12 dB et gros désaccord de l'Oberheim, nombre de voix d'origine (Juno 6, Prophet 5, Jupiter/OB 8, DX7 16).

Réglages exposés au DAW : Preset, Brillance, Résonance, Attaque, Release, Chorus, Dérive analogique, Volume.

## Compiler

Prérequis : CMake 3.22+ et un compilateur C++17 (Xcode sur Mac, Visual Studio 2022 sur Windows). JUCE est téléchargé automatiquement.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Résultat dans `build/Retro80_artefacts/Release/` : `VST3/Retro80.vst3` (à copier dans le dossier VST3 du système),
`Standalone/Retro80` (jouable sans DAW), et `AU/` sur Mac.

Sous Linux, il faut en plus : `libasound2-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxcomposite-dev libfreetype-dev libfontconfig1-dev libgl1-mesa-dev`.

Pour écouter tous les presets sans DAW : `./build/render_presets dossier_de_sortie`.

## Prochaines étapes

1. Ajuster les presets à l'oreille contre des enregistrements des machines d'origine.
2. Enveloppes DX7 authentiques (4 taux / 4 niveaux), les 32 algorithmes, et import des banques `.syx` du DX7.
3. Interface graphique dédiée, une « face » par machine.
4. Mode unisson (Jupiter/OB), glide, arpégiateur du Juno, sauvegarde de presets utilisateur.
5. Sur-échantillonnage du filtre et du sync pour un haut du spectre plus propre.

Les noms des machines sont des marques de leurs fabricants : à retirer des noms de presets si le plugin est distribué.
