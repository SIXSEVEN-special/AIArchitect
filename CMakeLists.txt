cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
project(AIArchitect VERSION 1.0.0)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define the GEODE_SDK environment variable to point to your Geode SDK installation")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Work around an upstream fmt bug where newer Clang versions choke on
# fmt's consteval-based format-string checking inside fmt's own source
# (see fmtlib/fmt issues #4247 / #4264). Setting this directly on the
# fmt target (rather than via global CMAKE_CXX_FLAGS) is what actually
# takes effect, since fmt's CMakeLists sets its own compile options.
if (TARGET fmt)
    target_compile_definitions(fmt PRIVATE FMT_USE_CONSTEVAL=0)
endif()

file(GLOB SOURCES
    src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

# Same workaround for our own translation units, since they also use
# fmt::format / log::warn which would hit the same consteval codepath.
target_compile_definitions(${PROJECT_NAME} PRIVATE FMT_USE_CONSTEVAL=0)

setup_geode_mod(${PROJECT_NAME})