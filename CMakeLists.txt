cmake_minimum_required (VERSION 3.8)

project("BedrockPy")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

include(Utils)

generate_git_version (git_version)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS ON)


add_subdirectory("BedrockPy")

set (CPACK_PACKAGE_VERSION ${git_version})
set (CPACK_PACKAGE_DIRECTORY ${PROJECT_SOURCE_DIR}/pack)
set (CPACK_PACKAGE_CHECKSUM SHA256)

include (CPack)