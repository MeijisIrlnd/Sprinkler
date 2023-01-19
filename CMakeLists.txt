cmake_minimum_required(VERSION 3.7.2)
project(Sprinkler VERSION 0.0.1)
find_package(JUCE CONFIG REQUIRED)


juce_add_plugin(
    Sprinkler 
    PLUGIN_MANUFACTURER_CODE SMPI
    PLUGIN_CODE SM02
    FORMATS AU VST3 Standalone 
    PRODUCT_NAME "Sprinkler"
)
juce_add_module(${CMAKE_CURRENT_SOURCE_DIR}/Modules/SDSP)
juce_generate_juce_header(Sprinkler)

target_sources(Sprinkler PRIVATE 
Source/PluginProcessor.cpp
Source/PluginEditor.cpp 
Source/Audio/FrequencyDelay.cpp 
Source/Audio/Sprinkler.cpp
Source/UI/LF.cpp
)

target_include_directories(Sprinkler PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/Include
${CMAKE_CURRENT_SOURCE_DIR}/Source)


target_compile_definitions(Sprinkler PUBLIC 
JUCE_WEB_BROWSER=0
JUCE_USE_CURL=0
JUCE_VST3_CAN_REPLACE_VST2=0)

juce_add_binary_data(SprinklerData SOURCES Resources/NotoSansJP-Regular.otf)
target_link_libraries(Sprinkler PRIVATE 
SprinklerData
juce::juce_audio_utils 
juce::juce_dsp
SDSP
PUBLIC 
juce::juce_recommended_config_flags
juce::juce_recommended_lto_flags
juce::juce_recommended_warning_flags)