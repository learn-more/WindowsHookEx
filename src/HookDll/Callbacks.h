#pragma once

LRESULT WINAPI CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI CBTProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI DebugProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI MessageProc(int nCode, WPARAM wParam, LPARAM lParam);
