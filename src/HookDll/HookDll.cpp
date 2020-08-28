//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// SPDX-License-Identifier: MIT
//

#include "pch.h"
#include "Shared.h"
#include "SharedMem.h"
#include "Event.h"
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
    { L"WH_MSGFILTER", TRUE, HookDll_MessageProc },
    { L"WH_JOURNALRECORD", FALSE, HookDll_JournalRecordProc },
    { L"WH_JOURNALPLAYBACK", FALSE, HookDll_JournalPlaybackProc },
    { L"WH_KEYBOARD", TRUE, HookDll_KeyboardProc },
    { L"WH_GETMESSAGE", TRUE, HookDll_GetMsgProc },
    { L"WH_CALLWNDPROC", TRUE, HookDll_CallWndProc },
    { L"WH_CBT", TRUE, HookDll_CBTProc },
    { L"WH_SYSMSGFILTER", FALSE, HookDll_SysMsgProc },
    { L"WH_MOUSE", TRUE, HookDll_MouseProc },
    { L"WH_HARDWARE", }, /* Does not exist anymore? */
    { L"WH_DEBUG", TRUE, HookDll_DebugProc },
    { L"WH_SHELL", TRUE, HookDll_ShellProc },
    { L"WH_FOREGROUNDIDLE", TRUE, HookDll_ForegroundIdleProc },
    { L"WH_CALLWNDPROCRET", TRUE, HookDll_CallWndRetProc },
    { L"WH_KEYBOARD_LL", FALSE, HookDll_LowLevelKeyboardProc },
    { L"WH_MOUSE_LL", FALSE, HookDll_LowLevelMouseProc },
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
        case EVENT_ERROR:       return L"ERROR";
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
        StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L"vk=%d, lParam=%p", hook.wParam, hook.lParam);
        if (hook.nCode != HC_ACTION)
        {
            Ptr1 = Format_Action(hook.nCode);
            StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L", (%s)", Ptr1);
        }
        break;

    case EVENT_DLL_LOAD:
    case EVENT_DLL_UNLOAD:
        StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L"%s", Event->Info.Buffer);
        break;

    case EVENT_HOOK:
    case EVENT_UNHOOK:
        StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L"");
        break;

    case EVENT_ERROR:
    {
        PWSTR pws;
        DWORD cch = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, static_cast<DWORD>(hook.nCode), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&pws, 0, NULL);

        if (cch)
        {
            StringCchCopyExW(pszDest, cchDest, pws, &pszDest, &cchDest, 0);
            LocalFree(pws);
        }
    }
        break;

    case WH_SHELL:
        Ptr1 = Format_HSHELL(hook.nCode);
        StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L"%s, wParam=%p, lParam=%p", Ptr1, hook.wParam, hook.lParam);
        break;

    case WH_CBT:
        Ptr1 = Format_CBT(hook.nCode);
        StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L"%s, wParam=%p, lParam=%p", Ptr1, hook.wParam, hook.lParam);
        break;

    case WH_GETMESSAGE:
        Ptr1 = Format_PM(static_cast<int>(hook.wParam));
        StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L"hWnd=%p, %s, (%s), wParam=%p, lParam=%p",
            hook.m.msg.hwnd, Format_WM(hook.m.msg.message), Ptr1, hook.m.msg.wParam, hook.m.msg.lParam);
        if (hook.nCode != HC_ACTION)
        {
            Ptr1 = Format_Action(hook.nCode);
            StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L", (%s)", Ptr1);
        }
        break;

    case WH_CALLWNDPROC:
        StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L"cur thread: %d, hWnd=%p, %s, wParam=%p, lParam=%p",
            hook.wParam, hook.m.wp.hwnd, Format_WM(hook.m.wp.message), hook.m.wp.wParam, hook.m.wp.lParam);
        if (hook.nCode != HC_ACTION)
        {
            Ptr1 = Format_Action(hook.nCode);
            StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L", (%s)", Ptr1);
        }
        break;

    case WH_CALLWNDPROCRET:
        StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L"cur proc: %d, hWnd=%p, msg=0x%x, wParam=%p, lParam=%p",
            hook.wParam, hook.m.wpr.hwnd, hook.m.wpr.message, hook.m.wpr.wParam, hook.m.wpr.lParam);
        if (hook.nCode != HC_ACTION)
        {
            Ptr1 = Format_Action(hook.nCode);
            StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L", (%s)", Ptr1);
        }
        break;

    case WH_DEBUG:
        Ptr1 = HookDll_HookName(static_cast<int>(hook.wParam));
        StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L"(%s), tid=0x%x, " /*L"tidinstaller=0x%x, "*/ L"code=%d, wParam=%p, lParam=%p",
            Ptr1, hook.m.dbg.idThread, /*hook.m.dbg.idThreadInstaller,*/ hook.m.dbg.code, hook.m.dbg.wParam, hook.m.dbg.lParam);
        if (hook.nCode != HC_ACTION)
        {
            Ptr1 = Format_Action(hook.nCode);
            StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L", (%s)", Ptr1);
        }
        break;

    case WH_MOUSE:
        Ptr1 = Format_WM(static_cast<int>(hook.wParam));
        Ptr2 = Format_HitTest(hook.m.mh.wHitTestCode);
        StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0,
            L"%s, x=%d, y=%d, hwnd=0x%x, hit=%s", Ptr1, hook.m.mh.pt.x, hook.m.mh.pt.y, hook.m.mh.hwnd, Ptr2);

        if (hook.nCode != HC_ACTION)
        {
            Ptr1 = Format_Action(hook.nCode);
            StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L", (%s)", Ptr1);
        }

        if (hook.m.mh.dwExtraInfo)
        {
            StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0,
                L", extra=%d", hook.m.mh.dwExtraInfo);
        }
        break;

    default:
        StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L"wParam=%p, lParam=%p", hook.wParam, hook.lParam);
        if (hook.nCode != HC_ACTION)
        {
            Ptr1 = Format_Action(hook.nCode);
            StringCchPrintfExW(pszDest, cchDest, &pszDest, &cchDest, 0, L", (%s)", Ptr1);
        }
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

    StringCchPrintfExW(pszDest, cchDest, &pszDestEnd, &cchRemaining, 0, L"External instances: %d", Ptr->Common.NumberOfDllsLoaded - 1);

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
    SHARED_MEM* Mem = SharedMem_Pointer();

    if (Mem->Common.hHook != nullptr)
        return FALSE;

    int idHook = Mem->Settings.idHook;
    HOOK_INFO* hookInfo = GetHookInfo(idHook);

    if (!hookInfo)
    {
        return FALSE;
    }

    HOOKPROC lpfn = Mem->Settings.AlternateProc ? Mem->Settings.AlternateProc : hookInfo->lpFn;
    HINSTANCE hmod = Mem->Settings.AlternateProc ? nullptr : (HINSTANCE)&__ImageBase;
    DWORD dwThreadId = Mem->Settings.GlobalHook ? 0 : GetCurrentThreadId();

    HOOK_EVENT Event = { 0 };
    Event.HookType = EVENT_HOOK;
    Event_Push(Event);
    Mem->Common.hHook = SetWindowsHookExW(idHook, lpfn, hmod, dwThreadId);

    if (!Mem->Common.hHook)
    {
        DWORD dwErr = GetLastError();
        Event.HookType = EVENT_ERROR;
        Event.Info.Hook.nCode = dwErr;
        Event_Push(Event);
    }

    return TRUE;
}


HOOKDLL_EXPORT
VOID
HOOKDLL_DECLSPEC
HookDll_UninstallHook()
{
    SHARED_MEM* Mem = SharedMem_Pointer();

    if (Mem->Common.hHook && Mem->Common.HostProcess == GetCurrentProcessId())
    {
        HOOK_EVENT Event = { 0 };
        Event.HookType = EVENT_UNHOOK;
        Event_Push(Event);

        UnhookWindowsHookEx(Mem->Common.hHook);
        Mem->Common.hHook = nullptr;
    }
}

