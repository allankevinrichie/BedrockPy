cmake_minimum_required (VERSION 3.8)

project("BedrockPy")

if(DEFINED ENV{VCPKG_INSTALLATION_ROOT})
    message(STATUS "use system vcpkg: $ENV{VCPKG_INSTALLATION_ROOT}\\vcpkg.exe")
    set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_INSTALLATION_ROOT}/scripts/buildsystems/vcpkg.cmake)
else()
    message(STATUS "current CMake toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS ON)


if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    message(FATAL_ERROR "x64 system is required to build BedrockPy, aborting..." )
    set (VCPKG_TARGET_TRIPLET "x86-windows")
else()
    set (VCPKG_TARGET_TRIPLET "x64-windows")
endif()

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(install_dependency)

add_subdirectory("BedrockPy")
