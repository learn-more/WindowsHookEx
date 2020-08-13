//
// Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#include "Wizard-2020.h"
#include <cassert>
#include "HookDll/Shared.h"

static int g_ColumnWidths[] = {
    80,
    60,
    130
};


LRESULT
CSecondPage::_OnCreate()
{
    // Load resources.
    //m_wstrHeader = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_SECONDPAGE_HEADER);
    m_wstrSubHeaderFmt = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_SECONDPAGE_SUBHEADER);

    // Set up the ListView.
    m_hList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL, 0, 0, 0, 0, m_hWnd, nullptr, nullptr, nullptr);
    ListView_SetExtendedListViewStyle(m_hList, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);

    SetTimer(m_hWnd, 0x1234, 200, nullptr);

    LVCOLUMNW lvColumn = { 0 };
    lvColumn.mask = LVCF_TEXT;

    std::wstring wstrColumn = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_PROCESS);
    lvColumn.pszText = wstrColumn.data();
    ListView_InsertColumn(m_hList, m_nColumns++, &lvColumn);

    wstrColumn = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_THREAD);
    lvColumn.pszText = wstrColumn.data();
    ListView_InsertColumn(m_hList, m_nColumns++, &lvColumn);

    wstrColumn = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_HOOK);
    lvColumn.pszText = wstrColumn.data();
    ListView_InsertColumn(m_hList, m_nColumns++, &lvColumn);

    wstrColumn = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_INFO);
    lvColumn.pszText = wstrColumn.data();
    ListView_InsertColumn(m_hList, m_nColumns++, &lvColumn);



    return 0;
}

LRESULT
CSecondPage::OnDestroy()
{

    HookDll_UninstallHook();

    return 0;
}

LRESULT
CSecondPage::_OnSize()
{
    // Get the window size.
    RECT rcWindow;
    GetClientRect(m_hWnd, &rcWindow);

    // Move the list control.
    HDWP hDwp = BeginDeferWindowPos(1);

    int iListX = 0;
    int iListY = 0;
    int iListHeight = rcWindow.bottom;
    int iListWidth = rcWindow.right;
    hDwp = DeferWindowPos(hDwp, m_hList, nullptr, iListX, iListY, iListWidth, iListHeight, 0);

    EndDeferWindowPos(hDwp);

    // Adjust the list column widths.
    LONG lColumnWidth = iListWidth - (GetSystemMetrics(SM_CXVSCROLL) + 4);
    for (int n = 0; n < m_nColumns - 1; ++n)
    {
        assert(n < _countof(g_ColumnWidths));
        ListView_SetColumnWidth(m_hList, n, g_ColumnWidths[n]);
        lColumnWidth -= g_ColumnWidths[n];
    }
    ListView_SetColumnWidth(m_hList, m_nColumns-1, lColumnWidth);

    return 0;
}

LRESULT CALLBACK
CSecondPage::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CSecondPage* pPage = InstanceFromWndProc<CSecondPage, CPage, &CSecondPage::CPage::m_hWnd>(hWnd, uMsg, lParam);

    if (pPage)
    {
        switch (uMsg)
        {
            case WM_CREATE: return pPage->_OnCreate();
            case WM_DESTROY: return pPage->OnDestroy();
            case WM_SIZE: return pPage->_OnSize();
            case WM_TIMER: return pPage->OnTimer(wParam);
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void
CSecondPage::SwitchTo()
{
    SHARED_SETTINGS* Settings = HookDll_GetSettings();
    int idHook = Settings->idHook;
    m_wstrHeader = HookDll_HookName(idHook);

    m_wstrHookType = LoadStringAsWstr(m_pMainWindow->GetHInstance(), Settings->GlobalHook ? IDS_GLOBAL_HOOK : IDS_LOCAL_HOOK);

    WCHAR Buffer[512];
    StringCchPrintfW(Buffer, _countof(Buffer), m_wstrSubHeaderFmt.data(), m_wstrHookType.data(), Settings->NumberOfDllsLoaded);
    m_wstrSubHeader = Buffer;

    m_pMainWindow->SetHeader(&m_wstrHeader, &m_wstrSubHeader);
    m_pMainWindow->EnableBackButton(TRUE);
    m_pMainWindow->EnableNextButton(FALSE);
    ShowWindow(m_hWnd, SW_SHOW);

    HookDll_InstallHook();
}

void
CSecondPage::OnBack()
{
    HookDll_UninstallHook();
}

void
CSecondPage::OnNext()
{

}

void
CSecondPage::UpdateDPI()
{
}

LRESULT
CSecondPage::OnTimer(WPARAM wParam)
{
    if (wParam == 0x1234)
    {
        HOOK_EVENT Event = { 0 };
        WCHAR Buffer[512];
        int last = -1;
        for (UINT n = 0; n < 20; ++n)
        {
            if (!HookDll_GetEvent(&Event))
            {
                break;
            }

            LVITEM lvI = { 0 };

            lvI.pszText = Buffer;
            lvI.iItem = INT_MAX;
            lvI.mask = LVIF_TEXT;

            StringCchPrintfW(Buffer, _countof(Buffer), L"%05x", Event.ProcessId);
            lvI.iItem = ListView_InsertItem(m_hList, &lvI);
            last = lvI.iItem;
            lvI.iSubItem++;

            StringCchPrintfW(Buffer, _countof(Buffer), L"%05x", Event.ThreadId);
            ListView_SetItem(m_hList, &lvI);
            lvI.iSubItem++;

            StringCchPrintfW(Buffer, _countof(Buffer), L"%s", HookDll_HookName(Event.HookType));
            ListView_SetItem(m_hList, &lvI);
            lvI.iSubItem++;

            switch (Event.HookType)
            {
            case WH_KEYBOARD:
                StringCchPrintfW(Buffer, _countof(Buffer), L"nCode=%d, vk=%d, lParam=%p", Event.nCode, Event.wParam, Event.lParam);
                break;
            default:
                StringCchPrintfW(Buffer, _countof(Buffer), L"nCode=%d, wParam=%p, lParam=%p", Event.nCode, Event.wParam, Event.lParam);
                break;
            }

            ListView_SetItem(m_hList, &lvI);
        }
        if (last >= 0)
        {
            ListView_EnsureVisible(m_hList, last, TRUE);
        }
    }

    return 0;
}
