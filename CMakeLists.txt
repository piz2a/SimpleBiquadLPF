cmake_minimum_required(VERSION 3.15...4.2) # 최신 CMake 호환성 확보
project(SimpleFilter VERSION 0.0.1)
# JUCE의 안전 장치를 전역 수준에서 강제로 꺼버립니다.
add_compile_definitions(JUCE_VST3_CAN_REPLACE_VST2=0)

# 1. JUCE 프레임워크 연결 (지호님의 경로 ../JUCE 반영)
add_subdirectory(../JUCE bin/JUCE)

# 2. 오디오 플러그인 타겟 생성
juce_add_plugin(SimpleFilter
    COMPANY_NAME "Jihoaudio"           # 지호님의 회사/브랜드명
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE             
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE      # 빌드 후 시스템 플러그인 폴더로 복사
    PLUGIN_CODE "Slft"                # 4글자 고유 코드
    FORMATS VST3 AU Standalone        # macOS이므로 VST3와 AU 생성
    VST3_CAN_REPLACE_VST2 FALSE
)

# 3. 소스 파일 등록 (.jucer에 있던 파일들)
target_sources(SimpleFilter PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/SimpleFilterChannel.cpp
    Source/SimpleFilterChannel.h
    Source/SineWaveChannel.cpp
    Source/SineWaveChannel.h
    Source/Square.cpp
    Source/Square.h
)

# 4. 사용 중인 JUCE 모듈 링크
target_link_libraries(SimpleFilter PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra
    juce::juce_gui_basics
    juce::juce_graphics
    juce::juce_events
    juce::juce_core
    juce::juce_data_structures
)

# 5. C++ 표준 설정
target_compile_features(SimpleFilter PRIVATE cxx_std_20)

# 6. 컴파일러 설정 (Clang 17의 엄격한 경고 해결)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(SimpleFilter PRIVATE -Wno-error -Wno-reserved-identifier)
endif()
