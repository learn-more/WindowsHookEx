#include "pch.h"
#include "Shared.h"
#include "SharedMem.h"
#include "Event.h"
#include "Callbacks.h"
#include "Format.h"
#include <strsafe.h>

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
VOID
HOOKDLL_DECLSPEC
HookDll_GetEvents(_Out_ HOOK_EVENT* Events, _Inout_ SIZE_T* NumberOfEvents)
{
    size_t Tmp = *NumberOfEvents;
    Event_Pop(Events, Tmp);
    *NumberOfEvents = Tmp;
}


HOOKDLL_EXPORT
VOID
HOOKDLL_DECLSPEC
HookDll_FormatInfo(LPWSTR pszDest, size_t cchDest, const HOOK_EVENT* Event)
{
    auto& hook = Event->Info.Hook;
    LPCWSTR Ptr;

    switch (Event->HookType)
    {
    case WH_KEYBOARD:
        StringCchPrintfW(pszDest, cchDest, L"nCode=%d, vk=%d, lParam=%p", hook.nCode, hook.msg.wParam, hook.msg.lParam);
        break;
    case EVENT_DLL_LOAD:
    case EVENT_DLL_UNLOAD:
        StringCchPrintfW(pszDest, cchDest, L"%s", Event->Info.Buffer);
        break;
    case WH_SHELL:
        Ptr = Format_HSHELL(hook.nCode);
        if (Ptr)
            StringCchPrintfW(pszDest, cchDest, L"nCode=%s, wParam=%p, lParam=%p", Ptr, hook.msg.wParam, hook.msg.lParam);
        else
            StringCchPrintfW(pszDest, cchDest, L"nCode=?0x%x?, wParam=%p, lParam=%p", hook.nCode, hook.msg.wParam, hook.msg.lParam);
        break;
    case WH_CBT:
        Ptr = Format_CBT(hook.nCode);
        if (Ptr)
            StringCchPrintfW(pszDest, cchDest, L"nCode=%s, wParam=%p, lParam=%p", Ptr, hook.msg.wParam, hook.msg.lParam);
        else
            StringCchPrintfW(pszDest, cchDest, L"nCode=?0x%x?, wParam=%p, lParam=%p", hook.nCode, hook.msg.wParam, hook.msg.lParam);
        break;
    default:
        StringCchPrintfW(pszDest, cchDest, L"nCode=%d, wParam=%p, lParam=%p", hook.nCode, hook.msg.wParam, hook.msg.lParam);
        break;
    }
}

HOOKDLL_EXPORT
VOID
HOOKDLL_DECLSPEC
HookDll_FormatMiscInfo(LPWSTR pszDest, size_t cchDest)
{
    SHARED_MEM* Ptr = SharedMem_Pointer();

    if (Ptr->Queue.Dropped || Ptr->Queue.ReadAbort || Ptr->Queue.WriteAbort)
    {
        StringCchPrintfW(pszDest, cchDest, L"Instances: %d, Dropped: %d, Read/Write abort: %d/%d",
            Ptr->Settings.NumberOfDllsLoaded - 1,
            Ptr->Queue.Dropped,
            Ptr->Queue.ReadAbort,
            Ptr->Queue.WriteAbort);
    }
    else
    {
        StringCchPrintfW(pszDest, cchDest, L"Instances: %d", Ptr->Settings.NumberOfDllsLoaded - 1);
    }


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

