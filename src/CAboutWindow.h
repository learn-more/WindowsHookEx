//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// UI Framework: Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#pragma once

class CAboutWindow
{
public:
    HWND GetHwnd() const { return m_hWnd; }
    void DoModal();

    static std::unique_ptr<CAboutWindow> Create(CMainWindow* pMainWindow);

private:
    static constexpr WCHAR _wszWndClass[] = L"AboutWndClass";
    CMainWindow* m_pMainWindow;
    HWND m_hWnd;
    bool m_Active;
    bool m_OwnerWasActive;

    HWND m_hLine;
    HWND m_hOk;
    HWND m_hAbout;

    std::wstring m_wstrHeader;
    std::wstring m_wstrSubHeader;
    WORD m_wCurrentDPI;

    CAboutWindow(CMainWindow* pMainWindow);
    static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnOkButton();
    LRESULT _OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT _OnSysCommand(WPARAM wParam, LPARAM lParam);
    LRESULT _OnNotify(WPARAM wParam, LPARAM lParam);
    LRESULT _OnCreate();
    LRESULT _OnDestroy();
    LRESULT _OnDpiChanged(WPARAM wParam, LPARAM lParam);
    LRESULT _OnGetMinMaxInfo(LPARAM lParam);
    LRESULT _OnPaint();
    LRESULT _OnSize();
};
