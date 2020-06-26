include(Utils)

find_path_(
		PYTHON38_INCLUDE_DIR
		NAMES Python.h
		HINTS "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include/python3.8"
)

find_file_(
		PYTHON38_LIB
		NAMES python38.lib
		HINTS "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib"
)

find_file_(
		PYTHON38_DLL
		NAMES python38.dll
		HINTS "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/bin"
)

file(GLOB PYTHON38_RUNTIME ${VCPKG_ROOT}/packages/python3-full_${VCPKG_TARGET_TRIPLET}/share/bin/*.dll)

file(GLOB PYTHON38_EXE ${VCPKG_ROOT}/packages/python3-full_${VCPKG_TARGET_TRIPLET}/exe/*.exe)

set(PYTHON38_PACKAGE_DIR ${VCPKG_ROOT}/packages/python3-full_${VCPKG_TARGET_TRIPLET}/share/python3.8/Lib)


find_path_(
	PYBIND11_INCLUDE_DIR
	NAMES pybind11/pybind11.h
	HINTS "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include/"
)


find_file_(
	CHAKRACORE_LIB
	NAMES Chakracore.lib
	HINTS "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib"
)

find_file_(
	CHAKRACORE_DLL
	NAMES ChakraCore.dll
	HINTS "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/bin"
)

find_path_(
		DETOURS_INCLUDE_DIR
		NAMES detours/detours.h
		HINTS "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include/"
)

find_file_(
		DETOURS_LIB
		NAMES detours.lib
		HINTS "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib"
)

find_path_(
		DIA_INCLUDE_DIR
		NAMES dia2.h
		HINTS "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/DIA SDK/include"
)

find_file_(
		DBGHELP_LIB
		NAMES DbgHelp.Lib
		HINTS "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64"
)
