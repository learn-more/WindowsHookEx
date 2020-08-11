#include "pch.h"
#include "Shared.h"

static SHARED_SETTINGS g_Settings;

HOOKDLL_EXPORT
SHARED_SETTINGS*
HOOKDLL_DECLSPEC
HookDll_GetSettings(void)
{
    return &g_Settings;
}

struct HOOK_INFO
{
    LPCWSTR Name;
    BOOL CanBeSetLocal;
    HOOKPROC lpFn;
};

// https://docs.microsoft.com/en-us/windows/win32/winmsg/using-hooks

HOOK_INFO g_HookInfo[] = {
    { L"WH_MSGFILTER", TRUE },
    { L"WH_JOURNALRECORD", FALSE },
    { L"WH_JOURNALPLAYBACK", FALSE },
    { L"WH_KEYBOARD", TRUE },
    { L"WH_GETMESSAGE", TRUE },
    { L"WH_CALLWNDPROC", TRUE },
    { L"WH_CBT", TRUE },
    { L"WH_SYSMSGFILTER", FALSE },
    { L"WH_MOUSE", TRUE },
    { L"WH_HARDWARE", }, /* Does not exist anymore? */
    { L"WH_DEBUG", TRUE },
    { L"WH_SHELL", TRUE },
    { L"WH_FOREGROUNDIDLE", TRUE },
    { L"WH_CALLWNDPROCRET", TRUE },
    { L"WH_KEYBOARD_LL", FALSE },
    { L"WH_MOUSE_LL", FALSE },
};

HOOKDLL_EXPORT
LPCWSTR
HOOKDLL_DECLSPEC
HookDll_HookName(int idHook)
{
    if (idHook < WH_MIN || idHook > WH_MAX)
        return nullptr;
    if (idHook == 8)
        return nullptr;
    return g_HookInfo[idHook + 1].Name;
}


HOOKDLL_EXPORT
void HOOKDLL_DECLSPEC HookDll_InstallHook(void)
{
#if 0
    int idHook = 0;
    HOOKPROC lpfn;
    HINSTANCE hmod;
    DWORD dwThreadId;

    HHOOK hhook = SetWindowsHookExW(idHook, lpfn, hmod, dwThreadId);
#endif


}
