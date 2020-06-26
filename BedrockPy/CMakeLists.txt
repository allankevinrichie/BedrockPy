add_subdirectory(Base)
add_subdirectory(Python)
add_subdirectory(LibHook)
add_subdirectory(LibPDB)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp)
