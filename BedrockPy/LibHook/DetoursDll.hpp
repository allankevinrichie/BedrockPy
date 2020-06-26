#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
//#include <detours/detours.h>

#define detours_api __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    typedef struct _DETOUR_TRAMPOLINE DETOUR_TRAMPOLINE, * PDETOUR_TRAMPOLINE;

    /////////////////////////////////////////////////////////////// Helper Macros.
    //
    #define DETOURS_STRINGIFY(x)    DETOURS_STRINGIFY_(x)
    #define DETOURS_STRINGIFY_(x)    #x

    ///////////////////////////////////////////////////////////// Binary Typedefs.
    //
    typedef BOOL(CALLBACK* PF_DETOUR_BINARY_BYWAY_CALLBACK)(
        _In_opt_ PVOID pContext,
        _In_opt_ LPCSTR pszFile,
        _Outptr_result_maybenull_ LPCSTR* ppszOutFile);

    typedef BOOL(CALLBACK* PF_DETOUR_BINARY_FILE_CALLBACK)(
        _In_opt_ PVOID pContext,
        _In_ LPCSTR pszOrigFile,
        _In_ LPCSTR pszFile,
        _Outptr_result_maybenull_ LPCSTR* ppszOutFile);

    typedef BOOL(CALLBACK* PF_DETOUR_BINARY_SYMBOL_CALLBACK)(
        _In_opt_ PVOID pContext,
        _In_ ULONG nOrigOrdinal,
        _In_ ULONG nOrdinal,
        _Out_ ULONG* pnOutOrdinal,
        _In_opt_ LPCSTR pszOrigSymbol,
        _In_opt_ LPCSTR pszSymbol,
        _Outptr_result_maybenull_ LPCSTR* ppszOutSymbol);

    typedef BOOL(CALLBACK* PF_DETOUR_BINARY_COMMIT_CALLBACK)(
        _In_opt_ PVOID pContext);

    typedef BOOL(CALLBACK* PF_DETOUR_ENUMERATE_EXPORT_CALLBACK)(_In_opt_ PVOID pContext,
        _In_ ULONG nOrdinal,
        _In_opt_ LPCSTR pszName,
        _In_opt_ PVOID pCode);

    typedef BOOL(CALLBACK* PF_DETOUR_IMPORT_FILE_CALLBACK)(_In_opt_ PVOID pContext,
        _In_opt_ HMODULE hModule,
        _In_opt_ LPCSTR pszFile);

    typedef BOOL(CALLBACK* PF_DETOUR_IMPORT_FUNC_CALLBACK)(_In_opt_ PVOID pContext,
        _In_ DWORD nOrdinal,
        _In_opt_ LPCSTR pszFunc,
        _In_opt_ PVOID pvFunc);

    // Same as PF_DETOUR_IMPORT_FUNC_CALLBACK but extra indirection on last parameter.
    typedef BOOL(CALLBACK* PF_DETOUR_IMPORT_FUNC_CALLBACK_EX)(_In_opt_ PVOID pContext,
        _In_ DWORD nOrdinal,
        _In_opt_ LPCSTR pszFunc,
        _In_opt_ PVOID* ppvFunc);

    typedef VOID* PDETOUR_BINARY;
    typedef VOID* PDETOUR_LOADED_BINARY;

    //////////////////////////////////////////////////////////// Transaction APIs.
    //
    detours_api LONG WINAPI DetourTransactionBegin(VOID);
    detours_api LONG WINAPI DetourTransactionAbort(VOID);
    detours_api LONG WINAPI DetourTransactionCommit(VOID);
    detours_api LONG WINAPI DetourTransactionCommitEx(_Out_opt_ PVOID** pppFailedPointer);

    detours_api LONG WINAPI DetourUpdateThread(_In_ HANDLE hThread);

    detours_api LONG WINAPI DetourAttach(_Inout_ PVOID* ppPointer,
        _In_ PVOID pDetour);

    detours_api LONG WINAPI DetourAttachEx(_Inout_ PVOID* ppPointer,
        _In_ PVOID pDetour,
        _Out_opt_ PDETOUR_TRAMPOLINE* ppRealTrampoline,
        _Out_opt_ PVOID* ppRealTarget,
        _Out_opt_ PVOID* ppRealDetour);

    detours_api LONG WINAPI DetourDetach(_Inout_ PVOID* ppPointer,
        _In_ PVOID pDetour);

    detours_api BOOL WINAPI DetourSetIgnoreTooSmall(_In_ BOOL fIgnore);
    detours_api BOOL WINAPI DetourSetRetainRegions(_In_ BOOL fRetain);
    detours_api PVOID WINAPI DetourSetSystemRegionLowerBound(_In_ PVOID pSystemRegionLowerBound);
    detours_api PVOID WINAPI DetourSetSystemRegionUpperBound(_In_ PVOID pSystemRegionUpperBound);

    ////////////////////////////////////////////////////////////// Code Functions.
    //
    detours_api PVOID WINAPI DetourFindFunction(_In_ LPCSTR pszModule,
        _In_ LPCSTR pszFunction);
    detours_api PVOID WINAPI DetourCodeFromPointer(_In_ PVOID pPointer,
        _Out_opt_ PVOID* ppGlobals);
    detours_api PVOID WINAPI DetourCopyInstruction(_In_opt_ PVOID pDst,
        _Inout_opt_ PVOID* ppDstPool,
        _In_ PVOID pSrc,
        _Out_opt_ PVOID* ppTarget,
        _Out_opt_ LONG* plExtra);
    detours_api BOOL WINAPI DetourSetCodeModule(_In_ HMODULE hModule,
        _In_ BOOL fLimitReferencesToModule);

    ///////////////////////////////////////////////////// Loaded Binary Functions.
    //
    detours_api HMODULE WINAPI DetourGetContainingModule(_In_ PVOID pvAddr);
    detours_api HMODULE WINAPI DetourEnumerateModules(_In_opt_ HMODULE hModuleLast);
    detours_api PVOID WINAPI DetourGetEntryPoint(_In_opt_ HMODULE hModule);
    detours_api ULONG WINAPI DetourGetModuleSize(_In_opt_ HMODULE hModule);
    detours_api BOOL WINAPI DetourEnumerateExports(_In_ HMODULE hModule,
        _In_opt_ PVOID pContext,
        _In_ PF_DETOUR_ENUMERATE_EXPORT_CALLBACK pfExport);
    detours_api BOOL WINAPI DetourEnumerateImports(_In_opt_ HMODULE hModule,
        _In_opt_ PVOID pContext,
        _In_opt_ PF_DETOUR_IMPORT_FILE_CALLBACK pfImportFile,
        _In_opt_ PF_DETOUR_IMPORT_FUNC_CALLBACK pfImportFunc);

    detours_api BOOL WINAPI DetourEnumerateImportsEx(_In_opt_ HMODULE hModule,
        _In_opt_ PVOID pContext,
        _In_opt_ PF_DETOUR_IMPORT_FILE_CALLBACK pfImportFile,
        _In_opt_ PF_DETOUR_IMPORT_FUNC_CALLBACK_EX pfImportFuncEx);

    detours_api PVOID WINAPI DetourFindPayload(_In_opt_ HMODULE hModule,
        _In_ REFGUID rguid,
        _Out_ DWORD* pcbData);

    detours_api PVOID WINAPI DetourFindPayloadEx(_In_ REFGUID rguid,
        _Out_ DWORD* pcbData);

    detours_api DWORD WINAPI DetourGetSizeOfPayloads(_In_opt_ HMODULE hModule);



    ///////////////////////////////////////////////// Persistent Binary Functions.
    //
    detours_api PDETOUR_BINARY WINAPI DetourBinaryOpen(_In_ HANDLE hFile);

    detours_api PVOID WINAPI DetourBinaryEnumeratePayloads(_In_ PDETOUR_BINARY pBinary,
        _Out_opt_ GUID* pGuid,
        _Out_ DWORD* pcbData,
        _Inout_ DWORD* pnIterator);

    detours_api PVOID WINAPI DetourBinaryFindPayload(_In_ PDETOUR_BINARY pBinary,
        _In_ REFGUID rguid,
        _Out_ DWORD* pcbData);

    detours_api PVOID WINAPI DetourBinarySetPayload(_In_ PDETOUR_BINARY pBinary,
        _In_ REFGUID rguid,
        _In_reads_opt_(cbData) PVOID pData,
        _In_ DWORD cbData);
    detours_api BOOL WINAPI DetourBinaryDeletePayload(_In_ PDETOUR_BINARY pBinary, _In_ REFGUID rguid);
    detours_api BOOL WINAPI DetourBinaryPurgePayloads(_In_ PDETOUR_BINARY pBinary);
    detours_api BOOL WINAPI DetourBinaryResetImports(_In_ PDETOUR_BINARY pBinary);
    detours_api BOOL WINAPI DetourBinaryEditImports(_In_ PDETOUR_BINARY pBinary,
        _In_opt_ PVOID pContext,
        _In_opt_ PF_DETOUR_BINARY_BYWAY_CALLBACK pfByway,
        _In_opt_ PF_DETOUR_BINARY_FILE_CALLBACK pfFile,
        _In_opt_ PF_DETOUR_BINARY_SYMBOL_CALLBACK pfSymbol,
        _In_opt_ PF_DETOUR_BINARY_COMMIT_CALLBACK pfCommit);
    detours_api BOOL WINAPI DetourBinaryWrite(_In_ PDETOUR_BINARY pBinary, _In_ HANDLE hFile);
    detours_api BOOL WINAPI DetourBinaryClose(_In_ PDETOUR_BINARY pBinary);


    /////////////////////////////////////////////////// Create Process & Load Dll.
    //

    typedef BOOL(WINAPI* PDETOUR_CREATE_PROCESS_ROUTINEA)(
        _In_opt_ LPCSTR lpApplicationName,
        _Inout_opt_ LPSTR lpCommandLine,
        _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
        _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
        _In_ BOOL bInheritHandles,
        _In_ DWORD dwCreationFlags,
        _In_opt_ LPVOID lpEnvironment,
        _In_opt_ LPCSTR lpCurrentDirectory,
        _In_ LPSTARTUPINFOA lpStartupInfo,
        _Out_ LPPROCESS_INFORMATION lpProcessInformation);

    typedef BOOL(WINAPI* PDETOUR_CREATE_PROCESS_ROUTINEW)(
        _In_opt_ LPCWSTR lpApplicationName,
        _Inout_opt_ LPWSTR lpCommandLine,
        _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
        _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
        _In_ BOOL bInheritHandles,
        _In_ DWORD dwCreationFlags,
        _In_opt_ LPVOID lpEnvironment,
        _In_opt_ LPCWSTR lpCurrentDirectory,
        _In_ LPSTARTUPINFOW lpStartupInfo,
        _Out_ LPPROCESS_INFORMATION lpProcessInformation);

    detours_api BOOL WINAPI DetourCreateProcessWithDllA(_In_opt_ LPCSTR lpApplicationName,
        _Inout_opt_ LPSTR lpCommandLine,
        _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
        _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
        _In_ BOOL bInheritHandles,
        _In_ DWORD dwCreationFlags,
        _In_opt_ LPVOID lpEnvironment,
        _In_opt_ LPCSTR lpCurrentDirectory,
        _In_ LPSTARTUPINFOA lpStartupInfo,
        _Out_ LPPROCESS_INFORMATION lpProcessInformation,
        _In_ LPCSTR lpDllName,
        _In_opt_ PDETOUR_CREATE_PROCESS_ROUTINEA pfCreateProcessA);

    detours_api BOOL WINAPI DetourCreateProcessWithDllW(_In_opt_ LPCWSTR lpApplicationName,
        _Inout_opt_ LPWSTR lpCommandLine,
        _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
        _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
        _In_ BOOL bInheritHandles,
        _In_ DWORD dwCreationFlags,
        _In_opt_ LPVOID lpEnvironment,
        _In_opt_ LPCWSTR lpCurrentDirectory,
        _In_ LPSTARTUPINFOW lpStartupInfo,
        _Out_ LPPROCESS_INFORMATION lpProcessInformation,
        _In_ LPCSTR lpDllName,
        _In_opt_ PDETOUR_CREATE_PROCESS_ROUTINEW pfCreateProcessW);

    detours_api BOOL WINAPI DetourCreateProcessWithDllExA(_In_opt_ LPCSTR lpApplicationName,
        _Inout_opt_ LPSTR lpCommandLine,
        _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
        _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
        _In_ BOOL bInheritHandles,
        _In_ DWORD dwCreationFlags,
        _In_opt_ LPVOID lpEnvironment,
        _In_opt_ LPCSTR lpCurrentDirectory,
        _In_ LPSTARTUPINFOA lpStartupInfo,
        _Out_ LPPROCESS_INFORMATION lpProcessInformation,
        _In_ LPCSTR lpDllName,
        _In_opt_ PDETOUR_CREATE_PROCESS_ROUTINEA pfCreateProcessA);

    detours_api BOOL WINAPI DetourCreateProcessWithDllExW(_In_opt_ LPCWSTR lpApplicationName,
        _Inout_opt_  LPWSTR lpCommandLine,
        _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
        _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
        _In_ BOOL bInheritHandles,
        _In_ DWORD dwCreationFlags,
        _In_opt_ LPVOID lpEnvironment,
        _In_opt_ LPCWSTR lpCurrentDirectory,
        _In_ LPSTARTUPINFOW lpStartupInfo,
        _Out_ LPPROCESS_INFORMATION lpProcessInformation,
        _In_ LPCSTR lpDllName,
        _In_opt_ PDETOUR_CREATE_PROCESS_ROUTINEW pfCreateProcessW);

    detours_api BOOL WINAPI DetourCreateProcessWithDllsA(_In_opt_ LPCSTR lpApplicationName,
        _Inout_opt_ LPSTR lpCommandLine,
        _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
        _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
        _In_ BOOL bInheritHandles,
        _In_ DWORD dwCreationFlags,
        _In_opt_ LPVOID lpEnvironment,
        _In_opt_ LPCSTR lpCurrentDirectory,
        _In_ LPSTARTUPINFOA lpStartupInfo,
        _Out_ LPPROCESS_INFORMATION lpProcessInformation,
        _In_ DWORD nDlls,
        _In_reads_(nDlls) LPCSTR* rlpDlls,
        _In_opt_ PDETOUR_CREATE_PROCESS_ROUTINEA pfCreateProcessA);

    detours_api BOOL WINAPI DetourCreateProcessWithDllsW(_In_opt_ LPCWSTR lpApplicationName,
        _Inout_opt_ LPWSTR lpCommandLine,
        _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
        _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
        _In_ BOOL bInheritHandles,
        _In_ DWORD dwCreationFlags,
        _In_opt_ LPVOID lpEnvironment,
        _In_opt_ LPCWSTR lpCurrentDirectory,
        _In_ LPSTARTUPINFOW lpStartupInfo,
        _Out_ LPPROCESS_INFORMATION lpProcessInformation,
        _In_ DWORD nDlls,
        _In_reads_(nDlls) LPCSTR* rlpDlls,
        _In_opt_ PDETOUR_CREATE_PROCESS_ROUTINEW pfCreateProcessW);

    detours_api BOOL WINAPI DetourProcessViaHelperA(_In_ DWORD dwTargetPid,
        _In_ LPCSTR lpDllName,
        _In_ PDETOUR_CREATE_PROCESS_ROUTINEA pfCreateProcessA);

    detours_api BOOL WINAPI DetourProcessViaHelperW(_In_ DWORD dwTargetPid,
        _In_ LPCSTR lpDllName,
        _In_ PDETOUR_CREATE_PROCESS_ROUTINEW pfCreateProcessW);

    detours_api BOOL WINAPI DetourProcessViaHelperDllsA(_In_ DWORD dwTargetPid,
        _In_ DWORD nDlls,
        _In_reads_(nDlls) LPCSTR* rlpDlls,
        _In_ PDETOUR_CREATE_PROCESS_ROUTINEA pfCreateProcessA);

    detours_api BOOL WINAPI DetourProcessViaHelperDllsW(_In_ DWORD dwTargetPid,
        _In_ DWORD nDlls,
        _In_reads_(nDlls) LPCSTR* rlpDlls,
        _In_ PDETOUR_CREATE_PROCESS_ROUTINEW pfCreateProcessW);

    detours_api BOOL WINAPI DetourUpdateProcessWithDll(_In_ HANDLE hProcess,
        _In_reads_(nDlls) LPCSTR* rlpDlls,
        _In_ DWORD nDlls);

    detours_api BOOL WINAPI DetourUpdateProcessWithDllEx(_In_ HANDLE hProcess,
        _In_ HMODULE hImage,
        _In_ BOOL bIs32Bit,
        _In_reads_(nDlls) LPCSTR* rlpDlls,
        _In_ DWORD nDlls);

    detours_api BOOL WINAPI DetourCopyPayloadToProcess(_In_ HANDLE hProcess,
        _In_ REFGUID rguid,
        _In_reads_bytes_(cbData) PVOID pvData,
        _In_ DWORD cbData);
    detours_api BOOL WINAPI DetourRestoreAfterWith(VOID);
    detours_api BOOL WINAPI DetourRestoreAfterWithEx(_In_reads_bytes_(cbData) PVOID pvData,
        _In_ DWORD cbData);
    detours_api BOOL WINAPI DetourIsHelperProcess(VOID);
    detours_api VOID CALLBACK DetourFinishHelperProcess(_In_ HWND,
        _In_ HINSTANCE,
        _In_ LPSTR,
        _In_ INT);

#ifdef __cplusplus
}
#endif // __cplusplus