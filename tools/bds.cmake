function(set_bds_server)
    set(oneValueArgs VERSION)
    cmake_parse_arguments(
            ARG
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN}
    )
    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_PREFIX}/BedrockPy/python.exe ${CMAKE_SOURCE_DIR}/tools/dlbds.py --bds-version ${ARG_VERSION} WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/)")
endfunction()