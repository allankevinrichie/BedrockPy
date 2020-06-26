#macro(add_python_target tgt)
#  foreach(file ${ARGN})
#    set(OUT ${CMAKE_CURRENT_BINARY_DIR}/${file}.pyo)
#    list(APPEND OUT_FILES ${OUT})
#    add_custom_command(OUTPUT ${OUT}
#        COMMAND <python command you use to byte-compile .py file>)
#  endforeach()
#
#  add_custom_target(${tgt} ALL DEPENDS ${OUT_FILES})
#endmacro()

function(set_python38_env)
    set(oneValueArgs DESTINATION REQUIREMENTS)
    cmake_parse_arguments(
            ARG
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN}
    )
    install(
            FILES ${PYTHON38_EXE}
            DESTINATION ./${ARG_DESTINATION}/
            COMPONENT PYTHON
    )
    install(
            DIRECTORY ${PYTHON38_PACKAGE_DIR}
            DESTINATION ./${ARG_DESTINATION}/
            COMPONENT PYTHON
    )
    install(
            DIRECTORY ${PYTHON38_INCLUDE_DIR}/
            DESTINATION ./${ARG_DESTINATION}/include/
            COMPONENT PYTHON
    )
    install(
            FILES ${PYTHON38_LIB}
            DESTINATION ./${ARG_DESTINATION}/libs/
            COMPONENT PYTHON
    )
    install(
            CODE
            "execute_process(COMMAND ./python.exe -m ensurepip --upgrade WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${ARG_DESTINATION}/)"
            COMPONENT PYTHON
    )
    if (DEFINED ARG_REQUIREMENTS)
        install(
                CODE "execute_process(COMMAND ./python.exe -m pip install -r ${ARG_REQUIREMENTS} --no-warn-script-location WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${ARG_DESTINATION}/)"
                COMPONENT PYTHON
        )
    endif()
endfunction()
