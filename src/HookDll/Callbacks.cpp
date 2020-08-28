//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// SPDX-License-Identifier: MIT
//

#include "pch.h"
#include "Shared.h"
#include "Event.h"


/****************************************************************
  WH_CALLWNDPROC hook procedure
 ****************************************************************/
LRESULT CALLBACK HookDll_CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_CALLWNDPROC;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.m.wp = *(CWPSTRUCT*)lParam;
    if (!Event_Ignored(Event.Info.Hook.m.wp.hwnd))
    {
        Event_Push(Event);
    }

    return CallNextHookEx(0, nCode, wParam, lParam);
}

/****************************************************************
  WH_GETMESSAGE hook procedure
 ****************************************************************/
LRESULT CALLBACK HookDll_GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_GETMESSAGE;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.m.msg = *(MSG*)lParam;
    if (!Event_Ignored(Event.Info.Hook.m.msg.hwnd))
    {
        Event_Push(Event);
    }

    return CallNextHookEx(0, nCode, wParam, lParam);
}


/****************************************************************
  WH_DEBUG hook procedure
 ****************************************************************/
LRESULT CALLBACK HookDll_DebugProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_DEBUG;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.m.dbg = *(DEBUGHOOKINFO*)lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
}

/****************************************************************
  WH_CBT hook procedure
 ****************************************************************/
LRESULT CALLBACK HookDll_CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_CBT;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
}

/****************************************************************
  WH_MOUSE hook procedure
 ****************************************************************/
LRESULT CALLBACK HookDll_MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_MOUSE;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.m.mh = *(MOUSEHOOKSTRUCT*)lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
}

/****************************************************************
  WH_KEYBOARD hook procedure - TODO
 ****************************************************************/
LRESULT CALLBACK HookDll_KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_KEYBOARD;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
}


/****************************************************************
  WH_MSGFILTER hook procedure - TODO
 ****************************************************************/
LRESULT CALLBACK HookDll_MessageProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = (DWORD)WH_MSGFILTER;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
}



/****************************************************************
  WH_JOURNALRECORD hook procedure - TODO
 ****************************************************************/
LRESULT CALLBACK HookDll_JournalRecordProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_JOURNALRECORD;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
}

/****************************************************************
  WH_JOURNALPLAYBACK hook procedure - TODO
 ****************************************************************/
LRESULT CALLBACK HookDll_JournalPlaybackProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_JOURNALPLAYBACK;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
}

/****************************************************************
  WH_SYSMSGFILTER hook procedure - TODO
 ****************************************************************/
LRESULT CALLBACK HookDll_SysMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_SYSMSGFILTER;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
}

/****************************************************************
  WH_SHELL hook procedure - TODO: wParam
 ****************************************************************/
LRESULT CALLBACK HookDll_ShellProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_SHELL;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
}

/****************************************************************
  WH_FOREGROUNDIDLE hook procedure - TODO
 ****************************************************************/
LRESULT CALLBACK HookDll_ForegroundIdleProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_FOREGROUNDIDLE;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
}

/****************************************************************
  WH_CALLWNDPROCRET hook procedure
 ****************************************************************/
LRESULT CALLBACK HookDll_CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_CALLWNDPROCRET;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.m.wpr = *(CWPRETSTRUCT*)lParam;
    if (!Event_Ignored(Event.Info.Hook.m.wpr.hwnd))
    {
        Event_Push(Event);
    }

    return CallNextHookEx(0, nCode, wParam, lParam);
}

/****************************************************************
  WH_KEYBOARD_LL hook procedure - TODO
 ****************************************************************/
LRESULT CALLBACK HookDll_LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_KEYBOARD_LL;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
}

/****************************************************************
  WH_MOUSE_LL hook procedure - TODO
 ****************************************************************/
LRESULT CALLBACK HookDll_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_MOUSE_LL;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.wParam = wParam;
    Event.Info.Hook.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
}

