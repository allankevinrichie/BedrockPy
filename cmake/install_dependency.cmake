function(install_dependency arg)
    list(LENGTH ARGV argv_len)
    set(i 0)
    message(STATUS "vcpkg triplet: ${VCPKG_TARGET_TRIPLET}")
    while( i LESS ${argv_len})
        list(GET ARGV ${i} argv_value)
        message(STATUS "Processing package: ${argv_value}...")
        execute_process(
            COMMAND vcpkg install ${argv_value} --triplet ${VCPKG_TARGET_TRIPLET} 
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            RESULT_VARIABLE VCPKG_INSTALL_RES
            OUTPUT_QUIET
        )
        if(NOT (${VCPKG_INSTALL_RES} EQUAL 0))
            message(SEND_ERROR "Install ${argv_value} failed.")
        else()
            message(STATUS "Install ${argv_value} succeeded.")
        endif()
        math(EXPR i "${i} + 1")
    endwhile()
endfunction()
