//
// Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#pragma once

class CFirstPage : public CPage
{
public:
    static std::unique_ptr<CFirstPage> Create(CMainWindow* pMainWindow) { return CPage::Create<CFirstPage>(pMainWindow); }
    virtual void SwitchTo() override;
    virtual void OnBack() override;
    virtual void OnNext() override;
    virtual void UpdateDPI() override;

private:
    friend class CPage;
    static constexpr WCHAR _wszWndClass[] = L"FirstPageWndClass";

    HWND m_hComboHookType;
    std::wstring m_wstrHeader;
    std::wstring m_wstrSubHeader;

    SHARED_SETTINGS* m_Settings;

    CFirstPage(CMainWindow* pMainWindow) : CPage(pMainWindow) {}
    LRESULT _OnCreate();
    LRESULT _OnSize();
    static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
