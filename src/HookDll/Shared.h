#pragma once

#if defined(IN_HOOKDLL)
#define HOOKDLL_EXPORT      __declspec(dllexport)
#else
#define HOOKDLL_EXPORT      __declspec(dllimport)
#endif

#define HOOKDLL_DECLSPEC    __stdcall


struct SHARED_SETTINGS
{
    // Settings for the 'Current' hook
    int idHook;
    BOOL GlobalHook;

    // 
    DWORD HostProcess;
    HHOOK hHook;
    HWND IgnoreWnd;

    LONG NumberOfDllsLoaded;
};

#define EVENT_DLL_LOAD      100
#define EVENT_DLL_UNLOAD    101

// Linked list, variable data instead?
struct HOOK_EVENT
{
    DWORD ProcessId;
    DWORD ThreadId;
    DWORD HookType;
    union
    {
        struct
        {
            int nCode;
            WPARAM wParam;
            LPARAM lParam;
            union
            {
                MSG msg;
                CWPSTRUCT wp;
                CWPRETSTRUCT wpr;
                DEBUGHOOKINFO dbg;
            } m;
        } Hook;
        WCHAR Buffer[32];
    } Info;
};


HOOKDLL_EXPORT
SHARED_SETTINGS*
HOOKDLL_DECLSPEC
HookDll_GetSettings(void);

HOOKDLL_EXPORT
LPCWSTR
HOOKDLL_DECLSPEC
HookDll_HookName(int idHook);


HOOKDLL_EXPORT
BOOL
HOOKDLL_DECLSPEC
HookDll_CanBeSetLocal(int idHook);


HOOKDLL_EXPORT
VOID
HOOKDLL_DECLSPEC
HookDll_GetEvents(_Out_ HOOK_EVENT* Event, _Inout_ SIZE_T* NumberOfEvents);

HOOKDLL_EXPORT
VOID
HOOKDLL_DECLSPEC
HookDll_FormatInfo(LPWSTR pszDest, size_t cchDest, const HOOK_EVENT* Event);

HOOKDLL_EXPORT
VOID
HOOKDLL_DECLSPEC
HookDll_FormatMiscInfo(LPWSTR pszDest, size_t cchDest);


HOOKDLL_EXPORT
BOOL
HOOKDLL_DECLSPEC
HookDll_InstallHook();

HOOKDLL_EXPORT
VOID
HOOKDLL_DECLSPEC
HookDll_UninstallHook();

