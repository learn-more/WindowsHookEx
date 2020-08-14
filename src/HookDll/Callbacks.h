#pragma once

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DebugProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MessageProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK JournalRecordProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK JournalPlaybackProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SysMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ShellProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ForegroundIdleProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
