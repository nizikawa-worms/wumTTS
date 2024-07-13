cmake_minimum_required(VERSION 3.19)
project(wumTTS
    VERSION "1.0.0.0"
    DESCRIPTION "Text-to-speech for game chat"
)

add_subdirectory(flite)
add_subdirectory(hacklib)

set(CMAKE_CXX_STANDARD 20)
configure_file(resource.rc.in resource.rc)
add_compile_definitions(PROJECT_NAME="${PROJECT_NAME}")
add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")

include_directories(include hacklib/include flite/inc)
link_directories(lib)
add_subdirectory(lib/minhook-1.3.3)

add_library(wumTTS SHARED
    src/dllmain.cpp
    src/Hooks.cpp
    src/Speaker.cpp
    src/GameChat.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
)

set_property(TARGET
    wumTTS hacklib minhook
    flite_audio flite_hrg flite_regex flite_stats flite_utils flite_cg flite_lexicon flite_speech flite_synth flite_wavesynth flite_cmu_grapheme_lang
    flite_cmu_grapheme_lex flite_cmulex flite_cmu_us_kal flite_cmu_us_kal16 flite_cmu_us_slt flite_cmu_us_rms flite_cmu_us_awb flite_usenglish
    PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries(wumTTS
    PUBLIC hacklib
    PUBLIC minhook
    PUBLIC flite_audio
    PUBLIC flite_hrg
    PUBLIC flite_regex
    PUBLIC flite_stats
    PUBLIC flite_utils
    PUBLIC flite_cg
    PUBLIC flite_lexicon
    PUBLIC flite_speech
    PUBLIC flite_synth
    PUBLIC flite_wavesynth
    PUBLIC flite_cmu_grapheme_lang
    PUBLIC flite_cmu_grapheme_lex
    PUBLIC flite_cmulex
    PUBLIC flite_cmu_us_kal
    PUBLIC flite_cmu_us_kal16
    PUBLIC flite_cmu_us_slt
    PUBLIC flite_cmu_us_rms
    PUBLIC flite_cmu_us_awb
    PUBLIC flite_usenglish
)
