cmake_minimum_required(VERSION 3.15)
project(EzSqueeze LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(EZSQUEEZE_BUILD_TESTS "Build unit tests" ON)

# Core DSP library
file(GLOB DSP_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/dsp/*.h)
file(GLOB DSP_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/dsp/*.cpp)

add_library(ezsqueeze_dsp STATIC ${DSP_HEADERS} ${DSP_SOURCES})
target_include_directories(ezsqueeze_dsp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source)

# Advanced modules library
file(GLOB MODULE_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/*/*.h)
file(GLOB MODULE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/*/*.cpp)

add_library(ezsqueeze_modules STATIC ${MODULE_HEADERS} ${MODULE_SOURCES})
target_include_directories(ezsqueeze_modules PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/modules)

target_link_libraries(ezsqueeze_modules PUBLIC ezsqueeze_dsp)

# Tests
if (EZSQUEEZE_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.4
    )
    FetchContent_MakeAvailable(catch2)

    enable_testing()

    add_executable(ezsqueeze_tests
        tests/unit/test_detector.cpp
        tests/unit/test_gain_computer.cpp
        tests/unit/test_envelope.cpp
    )
    target_include_directories(ezsqueeze_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)
    target_link_libraries(ezsqueeze_tests PRIVATE ezsqueeze_dsp Catch2::Catch2WithMain)

    include(CTest)
    include(Catch)
    catch_discover_tests(ezsqueeze_tests)
endif()
