//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020-2023 Mark Jansen
// UI Framework: Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#pragma once

class CMainWindow
{
public:
    WORD GetCurrentDPI() const { return m_wCurrentDPI; }
    HFONT GetGuiFont() const { return m_hGuiFont.get(); }
    HFONT GetBoldGuiFont() const { return m_hBoldGuiFont.get(); }
    HINSTANCE GetHInstance() const { return m_hInstance; }
    HWND GetHwnd() const { return m_hWnd; }
    Gdiplus::Bitmap* GetLogoBitmap() { return m_pLogoBitmap.get(); }

    static std::unique_ptr<CMainWindow> Create(HINSTANCE hInstance, int nShowCmd);
    void EnableBackButton(BOOL bEnable);
    void EnableNextButton(BOOL bEnable, UINT uCaptionResource);
    void SetHeader(std::wstring* pwstrHeader, std::wstring* pwstrSubHeader);
    int WorkLoop();

private:
    static constexpr WCHAR _wszWndClass[] = L"MainWndClass";

    sr::unique_resource<HFONT, decltype(DeleteObject)*> m_hBoldGuiFont;
    sr::unique_resource<HFONT, decltype(DeleteObject)*> m_hGuiFont;
    HINSTANCE m_hInstance;
    HWND m_hWnd;
    HWND m_hLine;
    HWND m_hBack;
    HWND m_hNext;
    HWND m_hCancel;
    HWND m_hMenu;
    int m_nShowCmd;
    LOGFONTW m_lfBoldGuiFont;
    LOGFONTW m_lfGuiFont;
    CPage* m_pCurrentPage;
    std::unique_ptr<CSelectHookPage> m_pFirstPage;
    std::unique_ptr<CHookOutputPage> m_pSecondPage;
    std::unique_ptr<CAtomWindow> m_pAtomWindow;
    std::unique_ptr<Gdiplus::Bitmap> m_pLogoBitmap;
    HICON m_hCogIcon;
    std::wstring* m_pwstrHeader;
    std::wstring* m_pwstrSubHeader;
    WORD m_wCurrentDPI;

    CMainWindow(HINSTANCE hInstance, int nShowCmd);
    static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnBackButton();
    void _OnCancelButton();
    void _OnMenuButton();
    void _OnShowAtoms();
    LRESULT _OnAtomWindowClosed();
    void _OnShowAbout();
    LRESULT _OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT _OnCreate();
    LRESULT _OnDestroy();
    LRESULT _OnDpiChanged(WPARAM wParam, LPARAM lParam);
    LRESULT _OnGetMinMaxInfo(LPARAM lParam);
    void _OnNextButton();
    LRESULT _OnPaint();
    LRESULT _OnSize();
    void _SwitchPage(CPage* pNewPage);
};
