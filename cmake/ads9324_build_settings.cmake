#===============================================================================
# ADS9324 driver: build settings
#===============================================================================
# Read by the root CMakeLists.txt in both of its modes (plain CMake and the
# ESP-IDF component): target name, version, include directories, sources and
# ESP-IDF requirements. This file is the single source
# of the version; tests/CMakeLists.txt checks every other copy against it.
#===============================================================================

include_guard(GLOBAL)

set(ADS9324_TARGET_NAME "ads9324")

#===============================================================================
# Version (single source of truth)
#===============================================================================
set(ADS9324_VERSION_MAJOR 0)
set(ADS9324_VERSION_MINOR 1)
set(ADS9324_VERSION_PATCH 0)
set(ADS9324_VERSION "${ADS9324_VERSION_MAJOR}.${ADS9324_VERSION_MINOR}.${ADS9324_VERSION_PATCH}")

#===============================================================================
# Generated version header (build tree, never the source tree)
#===============================================================================
set(ADS9324_VERSION_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/../inc/ads9324_version.h.in")
set(ADS9324_VERSION_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/ads9324_generated")
set(ADS9324_VERSION_HEADER "${ADS9324_VERSION_HEADER_DIR}/ads9324_version.h")

# The directory must exist before ESP-IDF's idf_component_register() checks the include paths.
file(MAKE_DIRECTORY "${ADS9324_VERSION_HEADER_DIR}")
configure_file("${ADS9324_VERSION_TEMPLATE}" "${ADS9324_VERSION_HEADER}" @ONLY)

#===============================================================================
# Public include directories, sources, ESP-IDF requirements
#===============================================================================
set(ADS9324_PUBLIC_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../inc"
    "${ADS9324_VERSION_HEADER_DIR}"
)

# Header-only: the template implementation is src/ads9324.ipp, included by inc/ads9324.hpp.
set(ADS9324_SOURCE_FILES)

set(ADS9324_IDF_REQUIRES)
