//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// SPDX-License-Identifier: MIT
//

#include "pch.h"
#include "Shared.h"
#include "SharedMem.h"
#include "Event.h"
#include "Callbacks.h"
#include "Format.h"
#include <cassert>
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
        case EVENT_DLL_LOAD:    return L"DLL_LOAD";
        case EVENT_DLL_UNLOAD:  return L"DLL_UNLOAD";
        case EVENT_HOOK:        return L"HOOK";
        case EVENT_UNHOOK:      return L"UNHOOK";
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
    LPCWSTR Ptr1, Ptr2;

    switch (Event->HookType)
    {
    case WH_KEYBOARD:
        StringCchPrintfW(pszDest, cchDest, L"nCode=%d, vk=%d, lParam=%p", hook.nCode, hook.wParam, hook.lParam);
        break;
    case EVENT_DLL_LOAD:
    case EVENT_DLL_UNLOAD:
        StringCchPrintfW(pszDest, cchDest, L"%s", Event->Info.Buffer);
        break;
    case EVENT_HOOK:
    case EVENT_UNHOOK:
        StringCchPrintfW(pszDest, cchDest, L"");
        break;
    case WH_SHELL:
        Ptr1 = Format_HSHELL(hook.nCode);
        StringCchPrintfW(pszDest, cchDest, L"%s, wParam=%p, lParam=%p", Ptr1, hook.wParam, hook.lParam);
        break;
    case WH_CBT:
        Ptr1 = Format_CBT(hook.nCode);
        StringCchPrintfW(pszDest, cchDest, L"%s, wParam=%p, lParam=%p", Ptr1, hook.wParam, hook.lParam);
        break;
    case WH_GETMESSAGE:
        Ptr1 = Format_Action(hook.nCode);
        Ptr2 = Format_PM(hook.wParam);
        StringCchPrintfW(pszDest, cchDest, L"%s, %s, hWnd=%p, msg=0x%x, wParam=%p, lParam=%p",
            Ptr1, Ptr2, hook.m.msg.hwnd, hook.m.msg.message, hook.m.msg.wParam, hook.m.msg.lParam);
        break;

    case WH_CALLWNDPROC:
        Ptr1 = Format_Action(hook.nCode);
        StringCchPrintfW(pszDest, cchDest, L"%s, cur thread: %d, hWnd=%p, msg=0x%x, wParam=%p, lParam=%p",
            Ptr1, hook.wParam, hook.m.wp.hwnd, hook.m.wp.message, hook.m.wp.wParam, hook.m.wp.lParam);
        break;

    case WH_CALLWNDPROCRET:
        Ptr1 = Format_Action(hook.nCode);
        StringCchPrintfW(pszDest, cchDest, L"%s, cur proc: %d, hWnd=%p, msg=0x%x, wParam=%p, lParam=%p",
            Ptr1, hook.wParam, hook.m.wpr.hwnd, hook.m.wpr.message, hook.m.wpr.wParam, hook.m.wpr.lParam);
        break;

    case WH_DEBUG:
        assert(hook.nCode == HC_ACTION);
        Ptr1 = HookDll_HookName(hook.wParam);
        StringCchPrintfW(pszDest, cchDest, L"(%s), tid=0x%x, " /*L"tidinstaller=0x%x, "*/ L"code=%d, wParam=%p, lParam=%p",
            Ptr1, hook.m.dbg.idThread, /*hook.m.dbg.idThreadInstaller,*/ hook.m.dbg.code, hook.m.dbg.wParam, hook.m.dbg.lParam);
        break;

    default:
        StringCchPrintfW(pszDest, cchDest, L"nCode=%d, wParam=%p, lParam=%p", hook.nCode, hook.wParam, hook.lParam);
        break;
    }
}

HOOKDLL_EXPORT
VOID
HOOKDLL_DECLSPEC
HookDll_FormatMiscInfo(LPWSTR pszDest, size_t cchDest)
{
    SHARED_MEM* Ptr = SharedMem_Pointer();

    STRSAFE_LPWSTR pszDestEnd;
    size_t cchRemaining;

    StringCchPrintfExW(pszDest, cchDest, &pszDestEnd, &cchRemaining, 0, L"External instances: %d", Ptr->Settings.NumberOfDllsLoaded - 1);

    if (Ptr->Queue.Dropped)
    {
        StringCchPrintfExW(pszDestEnd, cchRemaining, &pszDestEnd, &cchRemaining, 0, L", Dropped: %d", Ptr->Queue.Dropped);
    }

    if (Ptr->Queue.ReadAbort)
    {
        StringCchPrintfExW(pszDestEnd, cchRemaining, &pszDestEnd, &cchRemaining, 0, L", R abort: %d", Ptr->Queue.ReadAbort);
    }

    if (Ptr->Queue.WriteAbort)
    {
        StringCchPrintfExW(pszDestEnd, cchRemaining, &pszDestEnd, &cchRemaining, 0, L", W abort: %d", Ptr->Queue.WriteAbort);
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

    int idHook = Settings->idHook;
    HOOK_INFO* hookInfo = GetHookInfo(idHook);

    if (!hookInfo)
    {
        return FALSE;
    }

    HOOKPROC lpfn = hookInfo->lpFn;
    HINSTANCE hmod = (HINSTANCE)&__ImageBase;
    DWORD dwThreadId = Settings->GlobalHook ? 0 : GetCurrentThreadId();

    HOOK_EVENT Event = { 0 };
    Event.HookType = EVENT_HOOK;
    Event_Push(Event);
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
        HOOK_EVENT Event = { 0 };
        Event.HookType = EVENT_UNHOOK;
        Event_Push(Event);

        UnhookWindowsHookEx(Settings->hHook);
        Settings->hHook = nullptr;
        Settings->IgnoreWnd = 0;
    }
}

