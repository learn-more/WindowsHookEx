//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020-2023 Mark Jansen
// UI Framework: Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#pragma once

class CSelectHookPage : public CPage
{
public:
    static std::unique_ptr<CSelectHookPage> Create(CMainWindow* pMainWindow) { return CPage::Create<CSelectHookPage>(pMainWindow); }
    virtual void SwitchTo() override;
    virtual void OnBack() override;
    virtual void OnNext() override;
    virtual void UpdateDPI() override;

    LRESULT _OnCommand(WPARAM wParam, LPARAM lParam);

private:
    friend class CPage;
    static constexpr WCHAR _wszWndClass[] = L"FirstPageWndClass";

    HWND m_hComboHookType;
    HWND m_hRadioGlobal;
    HWND m_hRadioLocal;
    HWND m_hHostHookproc;

    HWND m_hIgnoreOwnMsg;
    HWND m_hBreakOnDllLoad;
    HWND m_hBreakOnDllUnload;
    HWND m_hBreakOnFirstMsg;

    std::wstring m_wstrHeader;
    std::wstring m_wstrSubHeader;

    SHARED_SETTINGS* m_Settings;

    CSelectHookPage(CMainWindow* pMainWindow) : CPage(pMainWindow) {}
    LRESULT _OnCreate();
    LRESULT _OnDestroy();
    LRESULT _OnSize();
    static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
