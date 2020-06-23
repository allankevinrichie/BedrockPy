#include "Chakra.hpp"
#include <filesystem>
#include <libloaderapi.h>
#include <vector>

using namespace std::filesystem;

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			std::wcout << "Chakra injected!" << std::endl;
            unsigned long size=GetCurrentDirectory(0, nullptr);
            if(size == 0){
                std::wcout << L"GetCurrentDirectory failed, error code: " << GetLastError() << std::endl;
                break;
            } else {
                auto *current_dir = new char[size];
                GetCurrentDirectory(255, current_dir);
                std::string current_dir_s(current_dir);
                delete[] current_dir;
                std::cout << "Working directory detected: " << current_dir_s << std::endl;
            }
			auto proposals = std::vector<path> {
			    R"(.\BedrockPy\BedrockPy.dll)",
                R"(.\BedrockPy.dll)",
                R"(..\BedrockPy.dll)",
                R"(..\BedrockPy\BedrockPy.dll)"
			};
            bool load_succeed = false;
			for (const auto& base_dll: proposals){
                if (exists(base_dll))
                {
                    std::wcout << L"BedrockPy found: " << absolute(base_dll).c_str() << std::endl;
                    SetDllDirectory(absolute(base_dll).parent_path().string().c_str());
                    if (!LoadLibrary(absolute(base_dll).string().c_str()))
                    {
                        std::wcout << L"Can't load bedrockpy. Error code: " << GetLastError() << std::endl;
                        exit(1);
                    }
                    load_succeed = true;
                }
			}
			if (!load_succeed){
			    std::wcout << L"Cannot find BedrockPy at the following locations: " << std::endl;
                for (const auto& base_dll: proposals){
                    std::wcout << absolute(base_dll).c_str() << std::endl;
                }
                exit(1);
			}
		}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
	default:
		break;
	}
	return TRUE;
}
