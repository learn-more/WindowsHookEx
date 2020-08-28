//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// UI Framework: Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#include "Wizard-2020.h"


static LRESULT CALLBACK
Host_CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_CallWndProc(nCode, wParam, lParam);
}

static LRESULT CALLBACK
Host_GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_GetMsgProc(nCode, wParam, lParam);
}

static LRESULT CALLBACK
Host_CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_CBTProc(nCode, wParam, lParam);
}

static LRESULT CALLBACK
Host_DebugProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_DebugProc(nCode, wParam, lParam);
}

static LRESULT CALLBACK
Host_KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_KeyboardProc(nCode, wParam, lParam);
}

static LRESULT CALLBACK
Host_MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_MouseProc(nCode, wParam, lParam);
}

static LRESULT CALLBACK
Host_MessageProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_MessageProc(nCode, wParam, lParam);
}

static LRESULT CALLBACK
Host_JournalRecordProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_JournalRecordProc(nCode, wParam, lParam);
}

static LRESULT CALLBACK
Host_JournalPlaybackProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_JournalPlaybackProc(nCode, wParam, lParam);
}

static LRESULT CALLBACK
Host_SysMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_SysMsgProc(nCode, wParam, lParam);
}

static LRESULT CALLBACK
Host_ShellProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_ShellProc(nCode, wParam, lParam);
}

static LRESULT CALLBACK
Host_ForegroundIdleProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_ForegroundIdleProc(nCode, wParam, lParam);
}

static LRESULT CALLBACK
Host_CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_CallWndRetProc(nCode, wParam, lParam);
}

static LRESULT CALLBACK
Host_LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_LowLevelKeyboardProc(nCode, wParam, lParam);
}

static LRESULT CALLBACK
Host_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return HookDll_LowLevelMouseProc(nCode, wParam, lParam);
}

HOOKPROC
GetHostProc(int idHook)
{
    switch (idHook)
    {
        case WH_MSGFILTER: return Host_MessageProc;
        case WH_JOURNALRECORD: return Host_JournalRecordProc;
        case WH_JOURNALPLAYBACK: return Host_JournalPlaybackProc;
        case WH_KEYBOARD: return Host_KeyboardProc;
        case WH_GETMESSAGE: return Host_GetMsgProc;
        case WH_CALLWNDPROC: return Host_CallWndProc;
        case WH_CBT: return Host_CBTProc;
        case WH_SYSMSGFILTER: return Host_SysMsgProc;
        case WH_MOUSE: return Host_MouseProc;
        case WH_DEBUG: return Host_DebugProc;
        case WH_SHELL: return Host_ShellProc;
        case WH_FOREGROUNDIDLE: return Host_ForegroundIdleProc;
        case WH_CALLWNDPROCRET: return Host_CallWndRetProc;
        case WH_KEYBOARD_LL: return Host_LowLevelKeyboardProc;
        case WH_MOUSE_LL: return Host_LowLevelMouseProc;
        default:
            DebugBreak();
            return nullptr;
    }
}

