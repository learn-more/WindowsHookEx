//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// UI Framework: Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#pragma once

class CAtomWindow
{
public:
    HWND GetHwnd() const { return m_hWnd; }

    static std::unique_ptr<CAtomWindow> Create(CMainWindow* pMainWindow);

private:
    static constexpr WCHAR _wszWndClass[] = L"AtomWndClass";
    CMainWindow* m_pMainWindow;
    HWND m_hWnd;
    HWND m_hList;
    int m_nColumns = 0;

    std::wstring m_wstrGlobal;
    std::wstring m_wstrRegisterWindowMessage;

    CAtomWindow(CMainWindow* pMainWindow);
    static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnCreate();
    LRESULT _OnDestroy();
    LRESULT _OnDpiChanged(LPARAM lParam);
    LRESULT _OnGetMinMaxInfo(LPARAM lParam);
    LRESULT _OnTimer(WPARAM wParam);
    LRESULT _OnSize();
};
