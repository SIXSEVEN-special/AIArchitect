cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
project(AIArchitect VERSION 1.0.0)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define the GEODE_SDK environment variable to point to your Geode SDK installation")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# --- fmt consteval bug workaround -----------------------------------------
# fmt's base.h unconditionally re-defines FMT_USE_CONSTEVAL/FMT_CONSTEVAL
# via its own #if/#elif chain, which silently overwrites any -D compiler
# flag we pass in (confirmed via the "-Wmacro-redefined" warnings in CI
# logs). The only reliable fix is to patch the header text itself, right
# after CPM has fetched it but before anything compiles it.
file(GLOB_RECURSE AIARCHITECT_FMT_BASE_HEADERS
    "${CMAKE_SOURCE_DIR}/cpm-cache/fmt/*/include/fmt/base.h"
    "${CMAKE_BINARY_DIR}/_deps/fmt-src/include/fmt/base.h"
    "${CMAKE_BINARY_DIR}/**/fmt*/include/fmt/base.h"
)
foreach(AIARCHITECT_FMT_HDR ${AIARCHITECT_FMT_BASE_HEADERS})
    file(READ "${AIARCHITECT_FMT_HDR}" AIARCHITECT_FMT_CONTENT)
    if(NOT AIARCHITECT_FMT_CONTENT MATCHES "AIARCHITECT_CONSTEVAL_PATCH")
        string(REGEX REPLACE
            "define[ \t]+FMT_CONSTEVAL[ \t]+consteval"
            "define FMT_CONSTEVAL consteval\n// AIARCHITECT_CONSTEVAL_PATCH\n#undef FMT_USE_CONSTEVAL\n#define FMT_USE_CONSTEVAL 0\n#undef FMT_CONSTEVAL\n#define FMT_CONSTEVAL"
            AIARCHITECT_FMT_PATCHED
            "${AIARCHITECT_FMT_CONTENT}"
        )
        file(WRITE "${AIARCHITECT_FMT_HDR}" "${AIARCHITECT_FMT_PATCHED}")
        message(STATUS "AI Architect: patched fmt's base.h at ${AIARCHITECT_FMT_HDR} to disable broken consteval codepath")
    endif()
endforeach()
if(NOT AIARCHITECT_FMT_BASE_HEADERS)
    message(WARNING "AI Architect: could not locate fmt's base.h to patch - the consteval build error may still occur")
endif()
# ---------------------------------------------------------------------------

file(GLOB SOURCES
    src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

setup_geode_mod(${PROJECT_NAME})