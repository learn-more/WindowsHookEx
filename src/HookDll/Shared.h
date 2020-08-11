#pragma once

#if defined(IN_HOOKDLL)
#define HOOKDLL_EXPORT      __declspec(dllexport)
#else
#define HOOKDLL_EXPORT      __declspec(dllimport)
#endif

#define HOOKDLL_DECLSPEC    __stdcall


struct SHARED_SETTINGS
{
    int idHook = WH_KEYBOARD;
};



HOOKDLL_EXPORT
SHARED_SETTINGS*
HOOKDLL_DECLSPEC
HookDll_GetSettings(void);

HOOKDLL_EXPORT
LPCWSTR
HOOKDLL_DECLSPEC
HookDll_HookName(int idHook);

