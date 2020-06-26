#include "Loader.hpp"

void RegisterStaticHook(RVA sym, void* hook, void** org) {
	auto base = reinterpret_cast<VA>(GetModuleHandle(NULL));
	*org = reinterpret_cast<void*>(base + sym);
	auto ret = Hook<void*>(org, hook);
	if (ret != HookErrorCode::ERR_SUCCESS) {
		std::cout << "[Error] ";
		switch (ret) {
		case HookErrorCode::ERR_TRANSACTION_BEGIN:
			std::cout << "DetourTransactionBegin";
			break;
		case HookErrorCode::ERR_UPDATE_THREAD:
			std::cout << "DetourUpdateThread";
			break;
		case HookErrorCode::ERR_ATTACH:
			std::cout << "DetourAttach";
			break;
		case HookErrorCode::ERR_DETACH:
			std::cout << "DetourDetach";
			break;
		case HookErrorCode::ERR_TRANSACTION_COMMIT:
			std::cout << "DetourTransactionCommit";
			break;
		default:
			break;
		}
		std::cout << "failed!" << std::endl;
	}
}
