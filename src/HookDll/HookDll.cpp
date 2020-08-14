#include "pch.h"
#include "Shared.h"
#include "SharedMem.h"
#include "Event.h"
#include "Callbacks.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

HOOKDLL_EXPORT
SHARED_SETTINGS*
HOOKDLL_DECLSPEC
HookDll_GetSettings(void)
{
    return &SharedMem_Pointer()->Settings;
}

struct HOOK_INFO
{
    LPCWSTR Name;
    BOOL CanBeSetLocal;
    HOOKPROC lpFn;
};

// https://docs.microsoft.com/en-us/windows/win32/winmsg/using-hooks

HOOK_INFO g_HookInfo[] = {
    { L"WH_MSGFILTER", TRUE, MessageProc },
    { L"WH_JOURNALRECORD", FALSE, JournalRecordProc },
    { L"WH_JOURNALPLAYBACK", FALSE, JournalPlaybackProc },
    { L"WH_KEYBOARD", TRUE, KeyboardProc },
    { L"WH_GETMESSAGE", TRUE, GetMsgProc },
    { L"WH_CALLWNDPROC", TRUE, CallWndProc },
    { L"WH_CBT", TRUE, CBTProc },
    { L"WH_SYSMSGFILTER", FALSE, SysMsgProc },
    { L"WH_MOUSE", TRUE, MouseProc },
    { L"WH_HARDWARE", }, /* Does not exist anymore? */
    { L"WH_DEBUG", TRUE, DebugProc },
    { L"WH_SHELL", TRUE, ShellProc },
    { L"WH_FOREGROUNDIDLE", TRUE, ForegroundIdleProc },
    { L"WH_CALLWNDPROCRET", TRUE, CallWndRetProc },
    { L"WH_KEYBOARD_LL", FALSE, LowLevelKeyboardProc },
    { L"WH_MOUSE_LL", FALSE, LowLevelMouseProc },
};

static
HOOK_INFO* GetHookInfo(int idHook)
{
    if (idHook < WH_MIN || idHook > WH_MAX)
        return nullptr;

    if (idHook == 8)    // WH_HARDWARE
        return nullptr;

    HOOK_INFO* info = g_HookInfo + idHook + 1;

    if (info->lpFn == nullptr)
        return nullptr;

    return info;
}


HOOKDLL_EXPORT
LPCWSTR
HOOKDLL_DECLSPEC
HookDll_HookName(int idHook)
{
    HOOK_INFO* hookInfo = GetHookInfo(idHook);

    if (!hookInfo)
    {
        switch (idHook)
        {
        case EVENT_DLL_LOAD: return L"EVENT_DLL_LOAD";
        case EVENT_DLL_UNLOAD: return L"EVENT_DLL_UNLOAD";
        default:
            break;
        }

        return nullptr;
    }

    return hookInfo->Name;
}

HOOKDLL_EXPORT
BOOL
HOOKDLL_DECLSPEC
HookDll_CanBeSetLocal(int idHook)
{
    HOOK_INFO* hookInfo = GetHookInfo(idHook);

    if (!hookInfo)
        return FALSE;

    return hookInfo->CanBeSetLocal;
}

HOOKDLL_EXPORT
BOOL
HOOKDLL_DECLSPEC
HookDll_GetEvent(_Out_ HOOK_EVENT* Event)
{
    return Event_Pop(*Event) ? TRUE : FALSE;
}



HOOKDLL_EXPORT
BOOL
HOOKDLL_DECLSPEC
HookDll_InstallHook(void)
{
    SHARED_SETTINGS* Settings = &SharedMem_Pointer()->Settings;

    if (Settings->hHook != nullptr)
        return FALSE;

    Settings->HostProcess = GetCurrentProcessId();

    int idHook = Settings->idHook;
    HOOK_INFO* hookInfo = GetHookInfo(idHook);

    if (!hookInfo)
    {
        return FALSE;
    }

    HOOKPROC lpfn = hookInfo->lpFn;
    HINSTANCE hmod = (HINSTANCE)&__ImageBase;
    DWORD dwThreadId = Settings->GlobalHook ? 0 : GetCurrentThreadId();

    Settings->hHook = SetWindowsHookExW(idHook, lpfn, hmod, dwThreadId);
    return TRUE;
}


HOOKDLL_EXPORT
VOID
HOOKDLL_DECLSPEC
HookDll_UninstallHook()
{
    SHARED_SETTINGS* Settings = &SharedMem_Pointer()->Settings;

    if (Settings->hHook && Settings->HostProcess == GetCurrentProcessId())
    {
        UnhookWindowsHookEx(Settings->hHook);
        Settings->hHook = nullptr;
        Settings->HostProcess = 0;
    }
}

