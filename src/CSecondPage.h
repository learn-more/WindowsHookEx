//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// UI Framework: Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#pragma once

class CSecondPage : public CPage
{
public:
    static std::unique_ptr<CSecondPage> Create(CMainWindow* pMainWindow) { return CPage::Create<CSecondPage>(pMainWindow); }

    HWND GetList() const { return m_hList; }
    virtual void SwitchTo() override;
    virtual void OnBack() override;
    virtual void OnNext() override;
    virtual void UpdateDPI() override;

    LRESULT OnTimer(WPARAM wParam);

private:
    friend class CPage;
    static constexpr WCHAR _wszWndClass[] = L"SecondPageWndClass";

    HWND m_hList;
    std::wstring m_wstrHeader;
    std::wstring m_wstrHookType;
    std::wstring m_wstrSubHeader;
    int m_nColumns = 0;
    int m_nLastHeaderUpdate = 0;
    bool m_Active = false;
    std::vector<HOOK_EVENT> m_Events;
    std::unordered_map<DWORD, std::wstring> m_Processes;

    CSecondPage(CMainWindow* pMainWindow) : CPage(pMainWindow) {}
    void _UpdateSubHeader();
    LRESULT _OnCreate();
    LRESULT _OnDestroy();
    LRESULT _OnSize();
    static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
