#pragma once
#include "pch.h"
#include "..\Detours\detours.h"
#ifdef _WIN64
	#pragma comment(lib, "..\\Libraries\\x64\\detours.lib")
#else
	#pragma comment(lib, "..\\Libraries\\x86\\detours.lib")
#endif // _WIN64

namespace TranslucentFlyoutsLib
{
	class Detours
	{
	public:
		template <typename T, typename... Args>
		auto OldFunction(Args&&... args)
		{
			return reinterpret_cast<T*>(GetOldFunction())(args...);
		}

		static void Begin()
		{
			DetourSetIgnoreTooSmall(TRUE);
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
		}
		static void Commit()
		{
			DetourTransactionCommit();
		}

		template <typename... Args>
		static void Batch(Args&&... args)
		{
			Batch(std::forward<Args>(args)...);
		}
		template <typename T, typename... Args>
		static void Batch(T& t, Args&&... args)
		{
			t.SetHookState();
			Batch(args...);
		}
		static void Batch() {}

		Detours()
		{

		}
		Detours(LPCSTR pszModule, LPCSTR pszFunction, PVOID NewAddr = nullptr)
		{
			PVOID OldAddr = DetourFindFunction(pszModule, pszFunction);
			pvOldAddr = OldAddr;
			pvNewAddr = NewAddr;
		}
		Detours(PVOID OldAddr, PVOID NewAddr = nullptr)
		{
			pvOldAddr = OldAddr;
			pvNewAddr = NewAddr;
		}

		PVOID GetOldFunction() const
		{
			return pvOldAddr;
		}
		void SetHookState(BOOL bHookState = -1)
		{
			if (bHookState == -1)
			{
				bHookState = !bHookInstalled;
			}
			if (bHookState == TRUE)
			{
				DetourAttach(&(PVOID&)pvOldAddr, pvNewAddr);
				bHookInstalled = true;
			}
			else
			{
				DetourDetach(&(PVOID&)pvOldAddr, pvNewAddr);
				bHookInstalled = false;
			}
		}
		bool IsHookInstalled()
		{
			return bHookInstalled;
		}
	private:
		PVOID pvNewAddr = nullptr;
		PVOID pvOldAddr = nullptr;
		bool bHookInstalled = false;
	};

	using DetoursHook = Detours;

	static inline bool VerifyCaller(LPCTSTR pszCallerModuleName, PVOID pvCaller = _ReturnAddress())
	{
		HMODULE hModule = DetourGetContainingModule(pvCaller);
		return hModule == GetModuleHandle(pszCallerModuleName);
	}

	static inline bool VerifyProcessModule(LPCTSTR pszTargetModuleName)
	{
		HMODULE hModule = GetModuleHandle(nullptr);
		TCHAR pszModuleName[MAX_PATH + 1] = {};
		GetModuleFileName(hModule, pszModuleName, MAX_PATH);
		return _tcsicmp(pszModuleName, pszTargetModuleName);
	}
}