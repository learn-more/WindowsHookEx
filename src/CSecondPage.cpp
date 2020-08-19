//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// UI Framework: Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#include "Wizard-2020.h"
#include <cassert>
#include "HookDll/Shared.h"

static int g_ColumnWidths[] = {
    120,
    60,
    150
};


LRESULT
CSecondPage::_OnCreate()
{
    // Set up the ListView.
    m_hList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL, 0, 0, 0, 0, m_hWnd, nullptr, nullptr, nullptr);
    ListView_SetExtendedListViewStyle(m_hList, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);


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

    m_Events.resize(100);
    SetTimer(m_hWnd, 0x1234, 200, nullptr);

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

    m_wstrSubHeader.clear();
    _UpdateSubHeader();

    m_pMainWindow->EnableBackButton(TRUE);
    m_pMainWindow->EnableNextButton(TRUE, IDS_FINISH);
    ShowWindow(m_hWnd, SW_SHOW);

    Settings->IgnoreWnd = m_hWnd;
    HookDll_InstallHook();
    m_Active = true;
}

void
CSecondPage::OnBack()
{
    HookDll_UninstallHook();
    m_Active = false;
}

void
CSecondPage::OnNext()
{
    HookDll_UninstallHook();
    m_pMainWindow->EnableNextButton(FALSE, 0);
}

void
CSecondPage::_UpdateSubHeader()
{
    WCHAR Info[512 * 4];

    STRSAFE_LPWSTR pszDestEnd = nullptr;
    size_t cchRemaining = 0;

    StringCchPrintfExW(Info, _countof(Info), &pszDestEnd, &cchRemaining, 0, L"%s, ", m_wstrHookType.data());
    HookDll_FormatMiscInfo(pszDestEnd, cchRemaining);

    pszDestEnd = Info + wcslen(Info);
    cchRemaining = _countof(Info) - (pszDestEnd - Info);

    LPCWSTR Separator = L"\n";

    DWORD pid = GetCurrentProcessId();

    for (const auto& it : m_Processes)
    {
        if (it.first != pid)
        {
            StringCchCatExW(pszDestEnd, cchRemaining, Separator, &pszDestEnd, &cchRemaining, 0);
            Separator = L", ";

            StringCchCatExW(pszDestEnd, cchRemaining, it.second.c_str(), &pszDestEnd, &cchRemaining, 0);
        }
    }

    if (m_wstrSubHeader != Info)
    {
        m_wstrSubHeader = Info;
        m_pMainWindow->SetHeader(&m_wstrHeader, &m_wstrSubHeader);
    }
}

void
CSecondPage::UpdateDPI()
{
}

LRESULT
CSecondPage::OnTimer(WPARAM wParam)
{
    static WCHAR Buffer[512];

    if (m_Active && (m_nLastHeaderUpdate == 0 || (GetTickCount() - m_nLastHeaderUpdate) > 1000))
    {
        m_nLastHeaderUpdate = GetTickCount();
        _UpdateSubHeader();
    }

    if (wParam == 0x1234)
    {
        int last = -1;
        for (UINT n = 0; n < 20; ++n)
        {
            SIZE_T readItems = m_Events.size();
            HookDll_GetEvents(m_Events.data(), &readItems);
            if (readItems == 0)
            {
                break;
            }

            if (last == -1)
            {
                SetWindowRedraw(m_hList, FALSE);
            }

            for (SIZE_T j = 0; j < readItems; ++j)
            {
                const auto& Event = m_Events[j];

                if (Event.HookType == EVENT_DLL_LOAD)
                {
                    m_Processes[Event.ProcessId] = Event.Info.Buffer;
                }

                LVITEM lvI = { 0 };

                lvI.pszText = Buffer;
                lvI.iItem = INT_MAX;
                lvI.mask = LVIF_TEXT;

                const auto& it = m_Processes.find(Event.ProcessId);
                if (it != m_Processes.end())
                {
                    StringCchPrintfW(Buffer, _countof(Buffer), L"%s", it->second.c_str());
                }
                else
                {
                    StringCchPrintfW(Buffer, _countof(Buffer), L"%04x", Event.ProcessId);
                }
                lvI.iItem = ListView_InsertItem(m_hList, &lvI);
                last = lvI.iItem;
                lvI.iSubItem++;

                StringCchPrintfW(Buffer, _countof(Buffer), L"%04x", Event.ThreadId);
                ListView_SetItem(m_hList, &lvI);
                lvI.iSubItem++;

                StringCchPrintfW(Buffer, _countof(Buffer), L"%s", HookDll_HookName(Event.HookType));
                ListView_SetItem(m_hList, &lvI);
                lvI.iSubItem++;

                HookDll_FormatInfo(Buffer, _countof(Buffer), &Event);

                ListView_SetItem(m_hList, &lvI);

                if (Event.HookType == EVENT_DLL_UNLOAD)
                {
                    m_Processes.erase(Event.ProcessId);
                }
            }
        }
        if (last >= 0)
        {
            SetWindowRedraw(m_hList, TRUE);
            ListView_EnsureVisible(m_hList, last, TRUE);
        }
    }

    return 0;
}
