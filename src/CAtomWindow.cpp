//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// UI Framework: Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#include "Wizard-2020.h"
#include <cassert>

static const int iMinWindowHeight = 400;
static const int iMinWindowWidth = 500;
static ATOM g_AtomWndAtom = 0;

static int g_ColumnWidths[] = {
    60,
    60,
};

CAtomWindow::CAtomWindow(CMainWindow* pMainWindow)
    : m_pMainWindow(pMainWindow)
{
}

LRESULT CALLBACK
CAtomWindow::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CAtomWindow* pAtomWindow = InstanceFromWndProc<CAtomWindow, CAtomWindow, &CAtomWindow::m_hWnd>(hWnd, uMsg, lParam);

    // The first WM_GETMINMAXINFO comes before WM_NCCREATE, before we got our CAtomWindow pointer.
    if (pAtomWindow)
    {
        switch (uMsg)
        {
            case WM_CREATE: return pAtomWindow->_OnCreate();
            case WM_DESTROY: return pAtomWindow->_OnDestroy();
            case WM_DPICHANGED: return pAtomWindow->_OnDpiChanged(lParam);
            case WM_GETMINMAXINFO: return pAtomWindow->_OnGetMinMaxInfo(lParam);
            case WM_SIZE: return pAtomWindow->_OnSize();
            case WM_TIMER: return pAtomWindow->_OnTimer(wParam);
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT
CAtomWindow::_OnCreate()
{
    m_wstrGlobal = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_GLOBAL_ATOM);
    m_wstrRegisterWindowMessage = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_RWM_ATOM);

    // Set up the ListView.
    m_hList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL, 0, 0, 0, 0, m_hWnd, nullptr, nullptr, nullptr);
    ListView_SetExtendedListViewStyle(m_hList, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);

    LVCOLUMNW lvColumn = { 0 };
    lvColumn.mask = LVCF_TEXT;

    std::wstring wstrColumn = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_ATOM_ID);
    lvColumn.pszText = wstrColumn.data();
    ListView_InsertColumn(m_hList, m_nColumns++, &lvColumn);

    wstrColumn = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_ATOM_TYPE);
    lvColumn.pszText = wstrColumn.data();
    ListView_InsertColumn(m_hList, m_nColumns++, &lvColumn);

    wstrColumn = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_ATOM_NAME);
    lvColumn.pszText = wstrColumn.data();
    ListView_InsertColumn(m_hList, m_nColumns++, &lvColumn);

    // Set the window size.
    int iHeight = MulDiv(iMinWindowHeight, m_pMainWindow->GetCurrentDPI(), iWindowsReferenceDPI);
    int iWidth = MulDiv(iMinWindowWidth, m_pMainWindow->GetCurrentDPI(), iWindowsReferenceDPI);
    SetWindowPos(m_hWnd, nullptr, 0, 0, iWidth, iHeight, SWP_NOMOVE);

    // Finally, show the window.
    ShowWindow(m_hWnd, SW_SHOW);

    SetTimer(m_hWnd, 0x1234, 2000, nullptr);
    _OnTimer(0x1234);

    return 0;
}

LRESULT
CAtomWindow::_OnDestroy()
{
    // Drop our class instance from the window
    SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));

    PostMessageW(m_pMainWindow->GetHwnd(), WM_APP, 0, 0);

    return 0;
}

LRESULT
CAtomWindow::_OnDpiChanged(LPARAM lParam)
{
    // Redraw the entire window on every DPI change.
    RECT rcWindow;
    GetClientRect(m_hWnd, &rcWindow);
    InvalidateRect(m_hWnd, &rcWindow, FALSE);

    // Use the suggested new window size.
    RECT* const prcNewWindow = reinterpret_cast<RECT*>(lParam);
    int iWindowX = prcNewWindow->left;
    int iWindowY = prcNewWindow->top;
    int iWindowWidth = prcNewWindow->right - prcNewWindow->left;
    int iWindowHeight = prcNewWindow->bottom - prcNewWindow->top;
    SetWindowPos(m_hWnd, nullptr, iWindowX, iWindowY, iWindowWidth, iWindowHeight, SWP_NOZORDER | SWP_NOACTIVATE);

    return 0;
}

LRESULT
CAtomWindow::_OnGetMinMaxInfo(LPARAM lParam)
{
    PMINMAXINFO pMinMaxInfo = reinterpret_cast<PMINMAXINFO>(lParam);

    pMinMaxInfo->ptMinTrackSize.x = MulDiv(iMinWindowWidth, m_pMainWindow->GetCurrentDPI(), iWindowsReferenceDPI);
    pMinMaxInfo->ptMinTrackSize.y = MulDiv(iMinWindowHeight, m_pMainWindow->GetCurrentDPI(), iWindowsReferenceDPI);

    return 0;
}

LRESULT
CAtomWindow::_OnSize()
{
    // Get the window size.
    RECT rcWindow;
    GetClientRect(m_hWnd, &rcWindow);

    // Move the button.
    HDWP hDwp = BeginDeferWindowPos(1);

    const int iControlPadding = m_pMainWindow->DefaultControlPaddingPx();

    int iListX = iControlPadding;
    int iListY = iControlPadding;
    int iListWidth = rcWindow.right - 2 * iControlPadding;
    int iListHeight = rcWindow.bottom - 2 * iControlPadding;

    if (hDwp)
        hDwp = DeferWindowPos(hDwp, m_hList, nullptr, iListX, iListY, iListWidth, iListHeight, 0);

    if (hDwp)
        EndDeferWindowPos(hDwp);

    // Adjust the list column widths.
    LONG lColumnWidth = iListWidth - (GetSystemMetrics(SM_CXVSCROLL) + 4);
    for (int n = 0; n < m_nColumns - 1; ++n)
    {
        assert(n < _countof(g_ColumnWidths));
        ListView_SetColumnWidth(m_hList, n, g_ColumnWidths[n]);
        lColumnWidth -= g_ColumnWidths[n];
    }
    ListView_SetColumnWidth(m_hList, m_nColumns - 1, lColumnWidth);

    return 0;
}

LRESULT
CAtomWindow::_OnTimer(WPARAM wParam)
{
    WCHAR Buffer[512];
    WCHAR Id[20];

    if (wParam == 0x1234)
    {
        int tick = GetTickCount();

        SetWindowRedraw(m_hList, FALSE);
        ListView_DeleteAllItems(m_hList);

        for (UINT n = 0xC000; n <= 0xffff; ++n)
        {
            UINT res = GlobalGetAtomNameW((ATOM)n, Buffer, _countof(Buffer));
            if (res)
            {
                StringCchPrintfW(Id, _countof(Id), L"0x%04X", n);
                LVITEM lvI = { 0 };
                lvI.pszText = Id;
                lvI.iItem = INT_MAX;
                lvI.mask = LVIF_TEXT;
                lvI.iItem = ListView_InsertItem(m_hList, &lvI);

                lvI.iSubItem++;
                lvI.pszText = m_wstrGlobal.data();
                ListView_SetItem(m_hList, &lvI);

                lvI.iSubItem++;
                lvI.pszText = Buffer;
                ListView_SetItem(m_hList, &lvI);
            }
        }
        for (UINT n = 0xC000; n <= 0xffff; ++n)
        {
            UINT res = GetClipboardFormatNameW(n, Buffer, _countof(Buffer));
            if (res)
            {
                StringCchPrintfW(Id, _countof(Id), L"0x%04X", n);
                LVITEM lvI = { 0 };
                lvI.pszText = Id;
                lvI.iItem = INT_MAX;
                lvI.mask = LVIF_TEXT;
                lvI.iItem = ListView_InsertItem(m_hList, &lvI);

                lvI.iSubItem++;
                lvI.pszText = m_wstrRegisterWindowMessage.data();
                ListView_SetItem(m_hList, &lvI);

                lvI.iSubItem++;
                lvI.pszText = Buffer;
                ListView_SetItem(m_hList, &lvI);
            }
        }
        SetWindowRedraw(m_hList, TRUE);

        int end = GetTickCount();
        StringCchPrintfW(Buffer, _countof(Buffer), L"Took: %d ms\n", end - tick);
        OutputDebugStringW(Buffer);
        KillTimer(m_hWnd, 0x1234);
    }

    return 0;
}

std::unique_ptr<CAtomWindow>
CAtomWindow::Create(CMainWindow* pMainWindow)
{
    HINSTANCE hInstance = pMainWindow->GetHInstance();

    std::wstring wstrTitle = LoadStringAsWstr(hInstance, IDS_ATOM_TITLE);
    WCHAR TitleBuffer[MAX_PATH];
    StringCchPrintfW(TitleBuffer, _countof(TitleBuffer), wstrTitle.c_str(), wszAppName);

    if (!g_AtomWndAtom)
    {
        // Register the atom window class.
        WNDCLASSW wc = { 0 };
        wc.lpfnWndProc = CAtomWindow::_WndProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
        wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON));
        wc.lpszClassName = CAtomWindow::_wszWndClass;

        g_AtomWndAtom = RegisterClassW(&wc);
    }

    if (g_AtomWndAtom == 0)
    {
        std::wstring wstrMessage = L"RegisterClassW failed, last error is " + std::to_wstring(GetLastError());
        MessageBoxW(pMainWindow->GetHwnd(), wstrMessage.c_str(), TitleBuffer, MB_ICONERROR);
        return nullptr;
    }

    // Create the atom window.
    auto pAtomWindow = std::unique_ptr<CAtomWindow>(new CAtomWindow(pMainWindow));
    HWND hWnd = CreateWindowExW(
        0,
        CAtomWindow::_wszWndClass,
        TitleBuffer,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        pMainWindow->GetHwnd(),
        nullptr,
        hInstance,
        pAtomWindow.get());
    if (hWnd == nullptr)
    {
        std::wstring wstrMessage = L"CreateWindowExW failed for CAtomWindow, last error is " + std::to_wstring(GetLastError());
        MessageBoxW(nullptr, wstrMessage.c_str(), TitleBuffer, MB_ICONERROR);
        return nullptr;
    }

    return pAtomWindow;
}

