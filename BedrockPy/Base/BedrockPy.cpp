// BedrockPy.cpp : Source file for your target.
//
#pragma comment(lib, "DelayImp.lib")
#include "BedrockPy.hpp"
#include "BedrockPy_Core_Tools.hpp"
#include<vector>
#include <filesystem>
#include <Python.h>



namespace py = pybind11;
using namespace py::literals;
using namespace std::filesystem;

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD ul_reason_for_call,
	LPVOID lpReserved) {
    py::dict global_dict;
	switch (int(ul_reason_for_call)) {
		case DLL_PROCESS_ATTACH:
		{
//            std::cout << "BedrockPy " << "(ver:" << BEDROCKPY_VERSION << ")" << std::endl;
            auto proposals = std::vector<path> {
                    R"(.\BedrockPy\Python38.dll)",
                    R"(.\Python38.dll)",
                    R"(..\Python38.dll)",
                    R"(..\BedrockPy\Python38.dll)",
                    R"(..\..\BedrockPy\Python38.dll)"
            };
            bool python_found = false;
            path python_dir;
            for (const auto& python_dll: proposals){
                if (exists(python_dll))
                {
//                    std::wcout << L"Python dll found: " << absolute(python_dll) << std::endl;
                    python_dir = absolute(python_dll).parent_path();
                    Py_SetPythonHome(python_dir.c_str());
                    Py_SetPath((python_dir / L"./Lib").c_str());
                    python_found = true;
                }
            }
            if(!python_found){
                std::wcerr << L"Cannot find Python3.8 at the following locations: " << std::endl;
                for (const auto& python_dll: proposals){
                    std::wcout << absolute(python_dll) << std::endl;
                }
                exit(1);
            }
            std::wcout << L"Initializing embeded Python...";
            try {
                py::initialize_interpreter();
            }
            catch (std::exception &e) {
                std::cout << "Failed." << std::endl;
                std::cout << e.what() << std::endl;
                return FALSE;
            }
            std::cout << "OK." << std::endl;
            try {
                py::module::import("_BedrockPy_Core_Tools");
                py::module py_sys = py::module::import("sys");
                py::object py_path = py_sys.attr("path");
                py_path.attr("insert")(1, (python_dir / L"./Lib/site-packages").c_str());
                py_path.attr("insert")(2, (python_dir / L"./PyMods").c_str());
                py_path.attr("insert")(3, (python_dir / L"../PyMods").c_str());
//                std::cout << "GIL State:" << PyGILState_Check() << std::endl;
                py::module::import("BedrockPy_Core");
            }
            catch (std::exception &e) {
                std::cout << "Failed." << std::endl;
                std::cout << e.what() << std::endl;
                std::wcout << L"Press any key to continue..." << std::endl;
                std::cin.get();
                return FALSE;
            }
			break;
		}
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
		{
            std::cout << "Deinitializing BedrockPy...";
//			py::finalize_interpreter();
//            std::cout << "Done!" << std::endl;
			break;
		}
		default: break;

	}
    return TRUE;
}
