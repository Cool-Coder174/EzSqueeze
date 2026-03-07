cmake_minimum_required(VERSION 3.15)
project(EzSqueezeTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

set(DSP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../source/dsp)

set(UNIT_TESTS
    unit/test_detector.cpp
    unit/test_gain_computer.cpp
    unit/test_envelope.cpp
    unit/test_lookahead.cpp
    unit/test_stereo.cpp
    unit/test_sidechain_filter.cpp
)

add_executable(ezsqueeze_unit_tests ${UNIT_TESTS})
target_include_directories(ezsqueeze_unit_tests PRIVATE ${DSP_INCLUDE_DIR})
target_link_libraries(ezsqueeze_unit_tests PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(ezsqueeze_unit_tests)

add_executable(ezsqueeze_benchmarks benchmarks/bench_dsp.cpp)
target_include_directories(ezsqueeze_benchmarks PRIVATE ${DSP_INCLUDE_DIR})
target_link_libraries(ezsqueeze_benchmarks PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(ezsqueeze_benchmarks)
