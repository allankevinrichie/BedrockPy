cmake_minimum_required (VERSION 3.8)

project("BedrockPy")

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS ON)

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    message(FATAL_ERROR "x64 system is required to build BedrockPy, aborting..." )
    set (VCPKG_TARGET_TRIPLET "x86-windows")
else()
    set (VCPKG_TARGET_TRIPLET "x64-windows")
endif()

include(install_dependency)

add_subdirectory("BedrockPy")
