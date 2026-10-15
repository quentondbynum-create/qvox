cmake_minimum_required(VERSION 3.22)
project(QVox VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Pull JUCE automatically — first build downloads it (~150MB), then it's cached.
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(QVox
    COMPANY_NAME "Headroom Audio"
    PLUGIN_NAME "QVox"
    DESCRIPTION "Complete vocal chain: de-ess, compress, double, air — radio-ready vocals fast"
    PLUGIN_MANUFACTURER_CODE Hdrm
    PLUGIN_CODE Qvox
    BUNDLE_ID com.headroomaudio.qvox
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "QVox"
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_SYNTH FALSE
)

juce_add_binary_data(QVoxAssets SOURCES Source/Assets/headroom-logo.png)

target_sources(QVox PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(QVox PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(QVox PRIVATE
    QVoxAssets
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
)
