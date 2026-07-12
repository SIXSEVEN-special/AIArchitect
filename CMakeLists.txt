cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
project(AIArchitect VERSION 1.0.0)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define the GEODE_SDK environment variable to point to your Geode SDK installation")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

# Works around an upstream fmt/Clang bug (consteval format-string checking
# fails to compile on some newer Clang versions). This is Geode's own
# official escape hatch for this exact problem - see geode-sdk/geode's
# top-level CMakeLists.txt, which defines FMT_CONSTEVAL to nothing when
# this cache var is set, before fmt gets built.
set(GEODE_DISABLE_FMT_CONSTEVAL ON CACHE BOOL "" FORCE)

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

file(GLOB SOURCES
    src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

setup_geode_mod(${PROJECT_NAME})