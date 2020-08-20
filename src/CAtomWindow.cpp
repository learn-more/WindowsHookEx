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
            case WM_NOTIFY: return pAtomWindow->_OnNotify(wParam, lParam);
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT
CAtomWindow::_OnCreate()
{
    m_wstrGlobalAtom = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_GLOBAL_ATOM);
    m_wstrUserAtom = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_USER_ATOM);

    // Set up the ListView.
    m_hList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL, 0, 0, 0, 0, m_hWnd, nullptr, nullptr, nullptr);
    ListView_SetExtendedListViewStyle(m_hList, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);

    LVCOLUMNW lvColumn = { 0 };
    lvColumn.mask = LVCF_TEXT;

    std::wstring wstrColumn = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_ATOM_ID);
    lvColumn.pszText = wstrColumn.data();
    ListView_InsertColumn(m_hList, m_nColumns++, &lvColumn);

    wstrColumn = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_ATOM_TABLE);
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

    if (wParam == 0x1234)
    {
        SetWindowRedraw(m_hList, FALSE);
        bool invalidate = false;

        for (UINT n = 0xC000; n <= 0xffff; ++n)
        {
            UINT res = GlobalGetAtomNameW((ATOM)n, Buffer, _countof(Buffer));
            if (res)
            {
                UpdateAtom((ATOM)n, AtomTable::Global, std::wstring(Buffer, res), invalidate);
            }
        }
        for (UINT n = 0xC000; n <= 0xffff; ++n)
        {
            UINT res = GetClipboardFormatNameW(n, Buffer, _countof(Buffer));
            if (res)
            {
                UpdateAtom((ATOM)n, AtomTable::User, std::wstring(Buffer, res), invalidate);
            }
        }

        for (auto it = m_Atoms.begin(); it != m_Atoms.end();)
        {
            AtomInfo* Info = it->get();
            if (Info->Updated != m_Update)
            {
                INT index = FindAtomIndex(Info);
                if (Info->State == AtomState::Removed)
                {
                    if (index >= 0)
                    {
                        ListView_DeleteItem(m_hList, index);
                    }
                    OutputDebugStringW(L"Removed: ");
                    OutputDebugStringW(Info->Name.c_str());
                    OutputDebugStringW(L"\n");
                    it = m_Atoms.erase(it);
                    continue;
                }
                else
                {
                    Info->State = AtomState::Removed;
                    if (index >= 0)
                    {
                        ListView_SetItemState(m_hList, index, LVIS_GLOW, LVIS_GLOW);
                    }
                }
            }
            it++;
        }

        SetWindowRedraw(m_hList, TRUE);
        if (invalidate)
        {
            InvalidateRect(m_hList, NULL, FALSE);
        }

        m_Update++;
    }

    return 0;
}

AtomInfo*
CAtomWindow::FindAtomInfo(ATOM Id, AtomTable Table) const
{
    for (const auto& atom : m_Atoms)
    {
        if (atom->Id == Id && atom->Table == Table)
            return atom.get();
    }
    return nullptr;
}

int
CAtomWindow::FindAtomIndex(const AtomInfo* Info)
{
    LVFINDINFOW lvfi = { 0 };
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = reinterpret_cast<LPARAM>(Info);
    INT index = ListView_FindItem(m_hList, -1, &lvfi);
    assert(index >= 0);
    return index;
}

void
CAtomWindow::UpdateAtom(ATOM Id, AtomTable Table, const std::wstring& Name, bool& invalidate)
{
    AtomInfo* Info = FindAtomInfo(Id, Table);
    if (Info)
    {
        if (Info->Name != Name)
        {
            Info->Name = Name;
            Info->State = AtomState::Changed;
            invalidate = true;
            OutputDebugStringW(L"Changed: ");
            OutputDebugStringW(Info->Name.c_str());
            OutputDebugStringW(L"\n");

        }
        else if (Info->State != AtomState::Present)
        {
            Info->State = AtomState::Present;
            invalidate = true;
        }
        Info->Updated = m_Update;
    }
    else
    {
        m_Atoms.push_back(std::unique_ptr<AtomInfo>(new AtomInfo{ Id, Table, Name, AtomState::Added, m_Update }));
        OutputDebugStringW(L"Added: ");
        OutputDebugStringW(Name.c_str());
        OutputDebugStringW(L"\n");

        LVITEMW lvI = { 0 };
        lvI.mask = LVIF_TEXT | LVIF_PARAM;
        lvI.pszText = LPSTR_TEXTCALLBACK;
        lvI.lParam = reinterpret_cast<LPARAM>(m_Atoms.back().get());
        lvI.iItem = INT_MAX;
        lvI.iItem = ListView_InsertItem(m_hList, &lvI);
    }
}

//struct SortContext
//{
//    int nHeader;
//    bool bAscending;
//};
//
//static
//int CALLBACK
//s_CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
//{
//
//}

LRESULT
CAtomWindow::_OnNotify(WPARAM wParam, LPARAM lParam)
{
    LPNMHDR data = reinterpret_cast<LPNMHDR>(lParam);
    switch (data->code)
    {
    case LVN_COLUMNCLICK: return _OnListViewColumnClick(reinterpret_cast<NMLISTVIEW*>(lParam));
    case LVN_GETDISPINFOW: return _OnListViewGetDispInfo(reinterpret_cast<NMLVDISPINFOW*>(lParam));
    case NM_CUSTOMDRAW: return _OnListViewCustomDraw(reinterpret_cast<NMLVCUSTOMDRAW*>(lParam));
    }

    return DefWindowProcW(m_hWnd, WM_NOTIFY, wParam, lParam);
}

LRESULT
CAtomWindow::_OnListViewColumnClick(NMLISTVIEW* pnmv)
{
    HWND hHeader = (HWND)SendMessageW(m_hList, LVM_GETHEADER, 0, 0);
    HDITEMW hColumn = { 0 };

    INT nHeaderID = pnmv->iSubItem;
    // If the sorting column changed, remove the sorting style from the old column
    if ((m_nLastHeaderID != -1) && (m_nLastHeaderID != nHeaderID))
    {
        hColumn.mask = HDI_FORMAT;
        Header_GetItem(hHeader, m_nLastHeaderID, &hColumn);
        hColumn.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
        Header_SetItem(hHeader, m_nLastHeaderID, &hColumn);
        m_SortOrder = SortOrder::Ascending;
    }

    // Set the sorting style to the new column
    hColumn.mask = HDI_FORMAT;
    Header_GetItem(hHeader, nHeaderID, &hColumn);

    hColumn.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
    if (m_SortOrder == SortOrder::Ascending)
        hColumn.fmt |= HDF_SORTDOWN;
    else if (m_SortOrder == SortOrder::Descending)
        hColumn.fmt |= HDF_SORTUP;
    Header_SetItem(hHeader, nHeaderID, &hColumn);

    // Sort the list
    //SortContext ctx = { nHeaderID, m_bIsAscending };
    //ListView_SortItems(m_hList, s_CompareFunc, &ctx);

    // Save the new state
    m_nLastHeaderID = nHeaderID;
    if (m_SortOrder == SortOrder::Descending)
        m_SortOrder = SortOrder::Neutral;
    else
        m_SortOrder = static_cast<SortOrder>(static_cast<int>(m_SortOrder) + 1);

    return 0;
}

LRESULT
CAtomWindow::_OnListViewGetDispInfo(NMLVDISPINFOW* plvdi)
{
    AtomInfo* Info = reinterpret_cast<AtomInfo*>(plvdi->item.lParam);
    switch (plvdi->item.iSubItem)
    {
    case 0:
        StringCchPrintfW(plvdi->item.pszText, plvdi->item.cchTextMax, L"0x%04X", Info->Id);
        break;
    case 1:
        if (Info->Table == AtomTable::Global)
            plvdi->item.pszText = m_wstrGlobalAtom.data();
        else if (Info->Table == AtomTable::User)
            plvdi->item.pszText = m_wstrUserAtom.data();
        else
            __debugbreak();
        break;
    case 2:
        plvdi->item.pszText = Info->Name.data();
        break;
        DEFAULT_UNREACHABLE;
    }
    return 0;
}

LRESULT
CAtomWindow::_OnListViewCustomDraw(NMLVCUSTOMDRAW* plvcd)
{
    AtomInfo* Info = reinterpret_cast<AtomInfo*>(plvcd->nmcd.lItemlParam);
    switch (plvcd->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
        return CDRF_NOTIFYITEMDRAW;

    case CDDS_ITEMPREPAINT:
        if (!Info)
            return CDRF_DODEFAULT;

        switch (Info->State)
        {
        case AtomState::Added:
            plvcd->clrTextBk = RGB(0, 200, 0);
            return CDRF_NEWFONT;
        case AtomState::Changed:
            plvcd->clrTextBk = RGB(200, 200, 0);
            return CDRF_NEWFONT;
        case AtomState::Removed:
            plvcd->clrTextBk = RGB(200, 0, 0);
            return CDRF_NEWFONT;
        default:
            return CDRF_DODEFAULT;
        }
    }
    return CDRF_DODEFAULT;
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

