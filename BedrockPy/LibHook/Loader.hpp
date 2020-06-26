#pragma once

#define WIN32_LEAN_AND_MEAN
// Windows 头文件
#include <windows.h>
// C++标准库 头文件
#include <cstdint>
#include <iostream>
// Detours
#include <detours/detours.h>

#define loader_api __declspec(dllexport)

typedef uint64_t VA;
typedef unsigned int RVA;

template<typename Type>
using Ptr = Type*;

enum class HookErrorCode {
	ERR_SUCCESS,
	ERR_TRANSACTION_BEGIN,
	ERR_UPDATE_THREAD,
	ERR_ATTACH,
	ERR_DETACH,
	ERR_TRANSACTION_COMMIT
};
template<typename T = Ptr<void>>
auto Hook(Ptr<T> p, T f) {
	int error = DetourTransactionBegin();
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_TRANSACTION_BEGIN;
	}
	error = DetourUpdateThread(GetCurrentThread());
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_UPDATE_THREAD;
	}
	error = DetourAttach(
		reinterpret_cast<Ptr<PVOID>>(p),
		reinterpret_cast<PVOID>(f)
	);
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_ATTACH;
	}
	error = DetourTransactionCommit();
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_TRANSACTION_COMMIT;
	}
	return HookErrorCode::ERR_SUCCESS;
}
template<typename T = Ptr<void>>
auto UnHook(Ptr<T> p, T f) {
	int error = DetourTransactionBegin();
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_TRANSACTION_BEGIN;
	}
	error = DetourUpdateThread(GetCurrentThread());
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_UPDATE_THREAD;
	}
	error = DetourDetach(
		reinterpret_cast<Ptr<PVOID>>(p),
		reinterpret_cast<PVOID>(f)
	);
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_DETACH;
	}
	error = DetourTransactionCommit();
	if (error != NO_ERROR) {
		return HookErrorCode::ERR_TRANSACTION_COMMIT;
	}
	return HookErrorCode::ERR_SUCCESS;
}

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	loader_api void RegisterStaticHook(RVA sym, void* hook, void** org);

#ifdef __cplusplus
}
#endif // __cplusplus