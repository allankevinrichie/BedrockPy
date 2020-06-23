// BedrockPy.cpp : Source file for your target.
//
#pragma comment(lib, "DelayImp.lib")
#include "BedrockPy.hpp"
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
            std::cout << "BedrockPy " << "(ver:" << BEDROCKPY_VERSION << ")" << std::endl;
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
                    std::wcout << L"Python dll found: " << absolute(python_dll) << std::endl;
                    python_dir = absolute(python_dll).parent_path();
                    Py_SetPythonHome(python_dir.c_str());
                    Py_SetPath(python_dir.concat(LR"(.\Lib)").c_str());
                    python_found = true;
                }
            }
            if(!python_found){
                std::wcout << L"Cannot find Python3.8 at the following locations: " << std::endl;
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
            std::cout << "Embedded Python Info:" << std::endl;
            try {
                //global_dict.update(py::module::import("__main__").attr("__dict__"))
                global_dict["sys"] = py::module::import("sys");
                py::exec("sys.path.insert(0, './PyAddOns')", global_dict);
                py::print("Version: ", global_dict["sys"].attr("version"));
                //py::print("Python module search path: ", global_dict["sys"].attr("path"));
                //py::print("globals: ", global_dict);
                py::print("Initializing BedrockPy Core...");
                global_dict["core"] = py::module::import("bedrockpy_core");
                global_dict["core"].attr("init")();
                //py::module::import("IPython").attr("start_ipython")();
            }
            catch (std::exception &e) {
                std::cout << "Failed." << std::endl;
                std::cout << e.what() << std::endl;
                std::wcout << L"Press any key to continue..." << std::endl;
                std::cin.get();
                return FALSE;
            }
            //std::cout << "Importing " << "PYMCAPI..." << std::endl;
            //try {
            //    auto mcpy_init = py::module::import("bedrockpy").attr("init");
            //    mcpy_init();
            //}
            //catch (std::exception e) {
            //    std::cout << "Failed." << std::endl;
            //    std::cout << e.what() << std::endl;
            //    return FALSE;
            //}
			break;
		}
		case DLL_THREAD_ATTACH:
			break;
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
