define_dll(
        LibPDB
        DESTINATION BedrockPy
        INCLUDE ${DIA_INCLUDE_DIR}
        DELAY_LINK ${DBGHELP_LIB}
)
