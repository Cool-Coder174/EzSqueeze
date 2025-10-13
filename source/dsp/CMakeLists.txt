cmake_minimum_required(VERSION 3.15)

project(EzSqueezeDSP VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# Debug/Release flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Create DSP library
add_library(EzSqueezeDSP STATIC
    Detector.cpp
    GainComputer.cpp
    LookaheadBuffer.cpp
    StereoLink.cpp
    SidechainFilter.cpp
    EnvelopeFollower.cpp
    AutoMakeupGain.cpp
    ProgramDependentRelease.cpp
    CompressorProcessor.cpp
)

# Set include directories
target_include_directories(EzSqueezeDSP PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Create test executable
add_executable(test_compressor
    test_compressor.cpp
)

# Link test with DSP library
target_link_libraries(test_compressor EzSqueezeDSP)

# Math library for Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(test_compressor m)
endif()

# Enable testing
enable_testing()
add_test(NAME CompressorTest COMMAND test_compressor)

# Installation
install(TARGETS EzSqueezeDSP
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES
    Detector.h
    GainComputer.h
    LookaheadBuffer.h
    StereoLink.h
    SidechainFilter.h
    EnvelopeFollower.h
    AutoMakeupGain.h
    ProgramDependentRelease.h
    CompressorProcessor.h
    DESTINATION include/EzSqueeze/DSP
)