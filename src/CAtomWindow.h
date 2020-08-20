//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// UI Framework: Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#pragma once

enum class AtomTable
{
    Global,
    User,
};

enum class SortOrder
{
    Neutral,
    Ascending,
    Descending
};

enum class AtomState
{
    Added,
    Present,
    Changed,
    Removed
};

struct AtomInfo
{
    ATOM Id;
    AtomTable Table;
    std::wstring Name;
    AtomState State;
    UINT Updated;
};

class CAtomWindow
{
public:
    HWND GetHwnd() const { return m_hWnd; }
    AtomInfo* FindAtomInfo(ATOM Id, AtomTable Table) const;
    void UpdateAtom(ATOM Id, AtomTable Table, const std::wstring& Name, bool& invalidate);
    int FindAtomIndex(const AtomInfo* Info);

    static std::unique_ptr<CAtomWindow> Create(CMainWindow* pMainWindow);

private:
    static constexpr WCHAR _wszWndClass[] = L"AtomWndClass";
    CMainWindow* m_pMainWindow;
    HWND m_hWnd;
    HWND m_hList;
    int m_nColumns = 0;
    int m_nLastHeaderID = -1;
    SortOrder m_SortOrder = SortOrder::Ascending;
    std::vector<std::unique_ptr<AtomInfo>> m_Atoms;
    UINT m_Update = 1;

    std::wstring m_wstrGlobalAtom;
    std::wstring m_wstrUserAtom;

    CAtomWindow(CMainWindow* pMainWindow);
    static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnNotify(WPARAM wParam, LPARAM lParam);
    LRESULT _OnCreate();
    LRESULT _OnDestroy();
    LRESULT _OnDpiChanged(LPARAM lParam);
    LRESULT _OnGetMinMaxInfo(LPARAM lParam);
    LRESULT _OnTimer(WPARAM wParam);
    LRESULT _OnSize();

    LRESULT _OnListViewColumnClick(NMLISTVIEW* pnmv);
    LRESULT _OnListViewGetDispInfo(NMLVDISPINFOW* pnmv);
    LRESULT _OnListViewCustomDraw(NMLVCUSTOMDRAW* plvcd);
};
