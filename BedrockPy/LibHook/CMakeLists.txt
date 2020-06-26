define_dll(
        LibHook
        DESTINATION BedrockPy
        INCLUDE ${DETOURS_INCLUDE_DIR}
        LINK ${DETOURS_LIB}
)
