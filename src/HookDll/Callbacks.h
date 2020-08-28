//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// SPDX-License-Identifier: MIT
//

#pragma once

HOOKDLL_EXPORT LRESULT CALLBACK HookDll_CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOKDLL_EXPORT LRESULT CALLBACK HookDll_GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOKDLL_EXPORT LRESULT CALLBACK HookDll_CBTProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOKDLL_EXPORT LRESULT CALLBACK HookDll_DebugProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOKDLL_EXPORT LRESULT CALLBACK HookDll_KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOKDLL_EXPORT LRESULT CALLBACK HookDll_MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOKDLL_EXPORT LRESULT CALLBACK HookDll_MessageProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOKDLL_EXPORT LRESULT CALLBACK HookDll_JournalRecordProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOKDLL_EXPORT LRESULT CALLBACK HookDll_JournalPlaybackProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOKDLL_EXPORT LRESULT CALLBACK HookDll_SysMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOKDLL_EXPORT LRESULT CALLBACK HookDll_ShellProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOKDLL_EXPORT LRESULT CALLBACK HookDll_ForegroundIdleProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOKDLL_EXPORT LRESULT CALLBACK HookDll_CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOKDLL_EXPORT LRESULT CALLBACK HookDll_LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOKDLL_EXPORT LRESULT CALLBACK HookDll_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
