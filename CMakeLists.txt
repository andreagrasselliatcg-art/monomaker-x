cmake_minimum_required(VERSION 3.22)

project(MonoMakerX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(MonoMakerX
    COMPANY_NAME                "Tune Talks"
    COMPANY_WEBSITE             ""
    BUNDLE_ID                   com.tunetalks.monomakerx
    PLUGIN_MANUFACTURER_CODE    Tntk
    PLUGIN_CODE                 Mmkx
    PRODUCT_NAME                "Mono Maker - X"
    DESCRIPTION                 "Automatable mono-to-stereo width for pre-drops"
    FORMATS                     VST3 AU Standalone
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             Fx Spatial
    AU_MAIN_TYPE                kAudioUnitType_Effect
)

target_sources(MonoMakerX PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/MonoMakerLookAndFeel.cpp
)

target_compile_definitions(MonoMakerX PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(MonoMakerX
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
