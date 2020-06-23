cmake_minimum_required (VERSION 3.8)

project("BedrockPy")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/tools ${CMAKE_MODULE_PATH})
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/Dist)

set(VCPKG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg)
#set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}\\scripts\\buildsystems\\vcpkg.cmake)
set(VCPKG_EXECUTABLE ${VCPKG_ROOT}/vcpkg.exe)
set(VCPKG_TARGET_TRIPLET x64-windows)

include(Utils)
include(Deps)
include(CMakePrintHelpers)
include(Python)
include(bds)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_FLAGS "/EHsc")

# if compile on GitHub
if(DEFINED ENV{GITHUB_ACTIONS})
	generate_git_version (git_version)
	set(CPACK_PACKAGE_VERSION ${git_version})
	set(CPACK_PACKAGE_DIRECTORY ${PROJECT_SOURCE_DIR}/pack)
	set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
	set(CPACK_SOURCE_PACKAGE_FILE_NAME BedrockPy-${git_version})
	set(CPACK_SOURCE_IGNORE_FILES ${PROJECT_SOURCE_DIR}/Build)
	set(CPACK_PACKAGE_CHECKSUM SHA256)
	include (CPack)
# if compile on local host
else()
install_vcpkg()
file(STRINGS ${PROJECT_SOURCE_DIR}/vcpkgResponseFile.txt dependencies)
install_dependency(
	${dependencies}
)
endif()


add_subdirectory("BedrockPy")
add_subdirectory("Chakra")

