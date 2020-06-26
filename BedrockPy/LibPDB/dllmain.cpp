#pragma comment(lib, "DelayImp.lib")
#include <Windows.h>
#include <iostream>
#include <filesystem>
#include "PDB.h"

using namespace std::filesystem;
SymbolModule m_Impl;
typedef std::tuple<FunctionRAVList, FunctionSymList, FunctionDecList> ret_t;
ret_t ret;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

__declspec(dllexport) ret_t &extract_symbols(const std::string &pdb_path)
{
    const auto &pdb_file = path(pdb_path);
    m_Impl.Open(pdb_file.string().c_str());
    ret = std::make_tuple(m_Impl.m_FunctionRAVList, m_Impl.m_FunctionSymList, m_Impl.m_FunctionDecList);
    return ret;
}

#ifdef __cplusplus
}
#endif // __cplusplus

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
        default:
            break;
    }
    return TRUE;
}
