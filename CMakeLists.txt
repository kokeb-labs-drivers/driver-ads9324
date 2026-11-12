#===============================================================================
# ADS9324 driver: root CMake
#===============================================================================
# One file, two build systems:
#
# - ESP-IDF (ESP_PLATFORM): this folder is an ESP-IDF component. Point
#   EXTRA_COMPONENT_DIRS at it, check it out under components/, or add it as a
#   component-manager git dependency. menuconfig reads _config/Kconfig.
# - Any other CMake build (host, STM32, RP2040, nRF, ...): one INTERFACE target,
#   ads9324, consumed as ads9324::ads9324 both through add_subdirectory() and
#   through find_package(ads9324) after install.
#
#   cmake -S . -B build -DADS9324_BUILD_TESTS=ON
#   cmake --build build && ctest --test-dir build --output-on-failure
#===============================================================================
cmake_minimum_required(VERSION 3.16)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/ads9324_build_settings.cmake)

#===============================================================================
# ESP-IDF component
#===============================================================================
if(ESP_PLATFORM)
    idf_component_register(
        SRCS ${ADS9324_SOURCE_FILES}
        INCLUDE_DIRS ${ADS9324_PUBLIC_INCLUDE_DIRS}
        REQUIRES ${ADS9324_IDF_REQUIRES}
        KCONFIG ${CMAKE_CURRENT_LIST_DIR}/_config/Kconfig
    )
    if(ADS9324_SOURCE_FILES)
        target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_20)
    else()
        target_compile_features(${COMPONENT_LIB} INTERFACE cxx_std_20)
    endif()
    return()
endif()

project(ads9324_driver
    VERSION ${ADS9324_VERSION}
    DESCRIPTION "C++20 driver for the TI ADS9324 16-channel, 16-bit, 1 MSPS simultaneous-sampling SAR ADC with integrated PGA and SPI interface"
    LANGUAGES CXX)

include(GNUInstallDirs)

# ISO C++20 for every target this project builds (tests, fuzzers, examples), never the GNU dialect
# that CMake picks by default (handbook KB-CPP-001, KB-CPP20-001). CXX_EXTENSIONS does not travel
# through an INTERFACE target, so each folder that builds a target sets it as well.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# The installed layout keeps the source tree's inc/ + src/ pair, because ads9324.hpp includes
# "../src/ads9324.ipp" relative to itself:
#   <prefix>/include/ads9324/inc/*.hpp, ads9324_version.h
#   <prefix>/include/ads9324/src/ads9324.ipp
set(ADS9324_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/ads9324/inc")

#===============================================================================
# Target
#===============================================================================
add_library(${ADS9324_TARGET_NAME} INTERFACE)
target_include_directories(${ADS9324_TARGET_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
        $<BUILD_INTERFACE:${ADS9324_VERSION_HEADER_DIR}>
        $<INSTALL_INTERFACE:${ADS9324_INSTALL_INCLUDEDIR}>
)
target_compile_features(${ADS9324_TARGET_NAME} INTERFACE cxx_std_20)

# ads9324::ads9324 everywhere: the alias for add_subdirectory(), EXPORT_NAME for the installed package.
add_library(ads9324::ads9324 ALIAS ${ADS9324_TARGET_NAME})
set_target_properties(${ADS9324_TARGET_NAME} PROPERTIES EXPORT_NAME ads9324)

option(ADS9324_ENABLE_WARNINGS "Add -Wall -Wextra -Wpedantic to every target that links ads9324::ads9324" OFF)
if(ADS9324_ENABLE_WARNINGS)
    target_compile_options(${ADS9324_TARGET_NAME} INTERFACE -Wall -Wextra -Wpedantic)
endif()

#===============================================================================
# Host tests, documentation snippets, install check (tests/)
#===============================================================================
option(ADS9324_BUILD_TESTS "Build the host tests in tests/" OFF)
option(ADS9324_BUILD_FUZZERS "Also build the libFuzzer targets in tests/fuzz/ (Clang only)" OFF)
if(ADS9324_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

#===============================================================================
# Install and package export
#===============================================================================
include(CMakePackageConfigHelpers)

install(TARGETS ${ADS9324_TARGET_NAME} EXPORT ads9324Targets
    INCLUDES DESTINATION ${ADS9324_INSTALL_INCLUDEDIR})

install(DIRECTORY inc/ DESTINATION ${ADS9324_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
install(FILES "${ADS9324_VERSION_HEADER}" DESTINATION ${ADS9324_INSTALL_INCLUDEDIR})
install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ads9324/src
    FILES_MATCHING PATTERN "*.ipp")

install(EXPORT ads9324Targets
    FILE ads9324Targets.cmake
    NAMESPACE ads9324::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ads9324)

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/ads9324Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ads9324Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ads9324)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ads9324ConfigVersion.cmake"
    VERSION ${ADS9324_VERSION}
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ads9324Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ads9324ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ads9324)
