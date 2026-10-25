cmake_minimum_required(VERSION 3.22)
project(Retro80 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE est téléchargé automatiquement au premier « cmake ».
include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

# AU n'est construit que sur macOS ; VST3 et Standalone partout.
set(RETRO80_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND RETRO80_FORMATS AU)
endif()

juce_add_plugin(Retro80
    COMPANY_NAME "Retro80"
    PLUGIN_MANUFACTURER_CODE Rt80
    PLUGIN_CODE R80s
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    FORMATS ${RETRO80_FORMATS}
    PRODUCT_NAME "Retro80")

target_sources(Retro80 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(Retro80 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(Retro80
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Outil de test : rend tous les presets en .wav sans DAW.
add_executable(render_presets tools/render_presets.cpp)
