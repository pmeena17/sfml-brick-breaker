cmake_minimum_required(VERSION 3.28)

project(BrickBreaker
    VERSION 2.0.0
    DESCRIPTION "Brick Breaker, rewritten in modern C++20"
    LANGUAGES CXX)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not supported. Use: cmake -S . -B build")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(BB_BUILD_GAME "Build the playable SFML game (downloads SFML if it is not installed)" ON)
option(BB_BUILD_TESTS "Build the unit tests" ON)
option(BB_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(BB_ENABLE_SANITIZERS "Build with AddressSanitizer + UBSan (GCC/Clang only)" OFF)
set(BB_SFML_TAG "3.0.2" CACHE STRING "SFML git tag to download when SFML 3 is not installed")

if(BB_ENABLE_SANITIZERS AND NOT MSVC)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

include(cmake/CompilerWarnings.cmake)

add_subdirectory(src)

if(BB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
