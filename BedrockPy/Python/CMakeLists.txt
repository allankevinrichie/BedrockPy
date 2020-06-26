#SUBDIRLIST(BUILT_IN_MODULES ${CMAKE_CURRENT_SOURCE_DIR})
#foreach(MODULE ${BUILT_IN_MODULES})
#    def_py(
#            ${MODULE}
#            DESTINATION ${CMAKE_INSTALL_PREFIX}/BedrockPy/PyMods
#    )
#endforeach()
def_py(
        BedrockPy_Core
        DESTINATION ${CMAKE_INSTALL_PREFIX}/BedrockPy/PyMods
)

install(
        DIRECTORY ${PYBIND11_INCLUDE_DIR}/pybind11
        DESTINATION ${CMAKE_INSTALL_PREFIX}/BedrockPy/include
)