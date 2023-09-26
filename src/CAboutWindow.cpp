//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// UI Framework: Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#include "Wizard-2020.h"
#include "version.h"
#include <Richedit.h>

static const int iHeaderHeight = 55;
static const int iMinWindowHeight = 300;
static const int iMinWindowWidth = 400;
static ATOM g_AboutAtom = 0;

#define IDC_OK              500
#define IDC_LICENSE         501


CAboutWindow::CAboutWindow(CMainWindow* pMainWindow)
    : m_pMainWindow(pMainWindow)
    , m_Active(true)
    , m_OwnerWasActive(false)
{
}

LRESULT CALLBACK
CAboutWindow::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CAboutWindow* pAboutWindow = InstanceFromWndProc<CAboutWindow, CAboutWindow, &CAboutWindow::m_hWnd>(hWnd, uMsg, lParam);

    // The first WM_GETMINMAXINFO comes before WM_NCCREATE, before we got our CAboutWindow pointer.
    if (pAboutWindow)
    {
        switch (uMsg)
        {
            case WM_COMMAND: return pAboutWindow->_OnCommand(wParam, lParam);
            case WM_SYSCOMMAND: return pAboutWindow->_OnSysCommand(wParam, lParam);
            case WM_NOTIFY: return pAboutWindow->_OnNotify(wParam, lParam);
            case WM_CREATE: return pAboutWindow->_OnCreate();
            case WM_DESTROY: return pAboutWindow->_OnDestroy();
            case WM_DPICHANGED: return pAboutWindow->_OnDpiChanged(wParam, lParam);
            case WM_GETMINMAXINFO: return pAboutWindow->_OnGetMinMaxInfo(lParam);
            case WM_PAINT: return pAboutWindow->_OnPaint();
            case WM_SIZE: return pAboutWindow->_OnSize();
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void
CAboutWindow::_OnOkButton()
{
    if (m_OwnerWasActive)
    {
        EnableWindow(m_pMainWindow->GetHwnd(), TRUE);
        m_OwnerWasActive = false;
    }

    DestroyWindow(m_hWnd);
}

LRESULT
CAboutWindow::_OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
        case IDC_OK: _OnOkButton(); return 0;
    }

    return DefWindowProcW(m_hWnd, WM_SYSCOMMAND, wParam, lParam);
}

LRESULT
CAboutWindow::_OnSysCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
        case SC_CLOSE:
            if (m_OwnerWasActive)
            {
                EnableWindow(m_pMainWindow->GetHwnd(), TRUE);
                m_OwnerWasActive = false;
            }
            break;
    }

    return DefWindowProcW(m_hWnd, WM_SYSCOMMAND, wParam, lParam);
}

LRESULT
CAboutWindow::_OnNotify(WPARAM wParam, LPARAM lParam)
{
    LPNMHDR hdr = (LPNMHDR)lParam;
    if (hdr->idFrom == IDC_LICENSE)
    {
        ENLINK* penLink;
        switch (hdr->code)
        {
        case EN_LINK:
            penLink = (ENLINK*)lParam;
            if (penLink->msg == WM_SETCURSOR)
            {
                SetCursor(LoadCursorW(NULL, IDC_HAND));
                return TRUE;
            }
            else if (penLink->msg == WM_LBUTTONUP)
            {
                if (penLink->chrg.cpMin != penLink->chrg.cpMax)
                {
                    std::wstring buf;
                    TEXTRANGEW txtRange;
                    txtRange.chrg = penLink->chrg;

                    size_t size = max(txtRange.chrg.cpMin, txtRange.chrg.cpMax) - min(txtRange.chrg.cpMin, txtRange.chrg.cpMax) + 1;
                    buf.resize(size);
                    txtRange.lpstrText = buf.data();
                    LRESULT lr = SendMessageW(m_hAbout, EM_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&txtRange));
                    buf.resize(lr);
                    if (!buf.empty())
                    {
                        ShellExecuteW(m_hWnd, L"open", buf.c_str(), NULL, NULL, SW_SHOW);
                    }
                }
                return TRUE;
            }
            else if (penLink->msg != WM_MOUSEMOVE)
            {
                OutputDebugStringA("WTF\n");
            }
            break;
        }
    }

    return DefWindowProcW(m_hWnd, WM_NOTIFY, wParam, lParam);
}


LRESULT
CAboutWindow::_OnCreate()
{
    // Get the DPI setting for the monitor where the window is shown.
    m_wCurrentDPI = GetWindowDPI(m_hWnd);

    HINSTANCE hInstance = m_pMainWindow->GetHInstance();

    // Load resources.
    m_wstrHeader = LoadStringAsWstr(hInstance, IDS_ABOUT_HEADER);
    m_wstrSubHeader = LoadStringAsWstr(hInstance, IDS_ABOUT_SUBHEADER);

    // Create the line above the buttons.
    m_hLine = CreateWindowExW(0, WC_STATICW, L"", WS_CHILD | WS_VISIBLE | SS_SUNKEN, 0, 0, 0, 0, m_hWnd, nullptr, nullptr, nullptr);

    // Create the bottom buttons.
    std::wstring wstrOk = LoadStringAsWstr(hInstance, IDS_OK);
    m_hOk = CreateWindowExW(0, WC_BUTTONW, wstrOk.c_str(), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, reinterpret_cast<HMENU>(IDC_OK), nullptr, nullptr);
    SendMessageW(m_hOk, WM_SETFONT, reinterpret_cast<WPARAM>(m_pMainWindow->GetGuiFont()), MAKELPARAM(TRUE, 0));

    // Create the about textbox
    DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_TABSTOP;
    m_hAbout = CreateWindowExW(0, MSFTEDIT_CLASS, L"", dwStyle, 0, 0, 0, 0, m_hWnd, reinterpret_cast<HMENU>(IDC_LICENSE), nullptr, nullptr);
    SendMessageW(m_hAbout, WM_SETFONT, reinterpret_cast<WPARAM>(m_pMainWindow->GetGuiFont()), MAKELPARAM(TRUE, 0));
    SendMessageW(m_hAbout, EM_AUTOURLDETECT, (WPARAM)1, (LPARAM)0);
    SendMessageW(m_hAbout, EM_SETEVENTMASK, 0, ENM_LINK | ENM_MOUSEEVENTS);

    Edit_SetText(m_hAbout, L"https://github.com/learn-more/WindowsHookEx\r\n"
    L"Version: " TEXT(GIT_VERSION_STR) L"\r\n"
    L"License: MIT\r\n"
    L"\r\n"
    L"UI Framework: Wizard-2020 Example\r\n"
    L"Copyright © 2020 Colin Finck, ENLYZE GmbH\r\n"
    L"https://github.com/enlyze/Wizard-2020\r\n"
    L"License: MIT\r\n");

    // Set the window size.
    int iHeight = MulDiv(iMinWindowHeight, m_wCurrentDPI, iWindowsReferenceDPI);
    int iWidth = MulDiv(iMinWindowWidth, m_wCurrentDPI, iWindowsReferenceDPI);
    SetWindowPos(m_hWnd, nullptr, 0, 0, iWidth, iHeight, SWP_NOMOVE);

    // Finally, show the window.
    ShowWindow(m_hWnd, SW_SHOW);

    return 0;
}

LRESULT
CAboutWindow::_OnDestroy()
{
    // Drop our class instance from the window
    SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));

    // Let's abort our window loop
    m_Active = false;

    return 0;
}

LRESULT
CAboutWindow::_OnDpiChanged(WPARAM wParam, LPARAM lParam)
{
    m_wCurrentDPI = LOWORD(wParam);

    // Redraw the entire window on every DPI change.
    InvalidateRect(m_hWnd, nullptr, FALSE);

    // Update the control fonts.
    SendMessageW(m_hOk, WM_SETFONT, reinterpret_cast<WPARAM>(m_pMainWindow->GetGuiFont()), MAKELPARAM(TRUE, 0));
    SendMessageW(m_hAbout, WM_SETFONT, reinterpret_cast<WPARAM>(m_pMainWindow->GetGuiFont()), MAKELPARAM(TRUE, 0));

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
CAboutWindow::_OnGetMinMaxInfo(LPARAM lParam)
{
    PMINMAXINFO pMinMaxInfo = reinterpret_cast<PMINMAXINFO>(lParam);

    pMinMaxInfo->ptMinTrackSize.x = MulDiv(iMinWindowWidth, m_wCurrentDPI, iWindowsReferenceDPI);
    pMinMaxInfo->ptMinTrackSize.y = MulDiv(iMinWindowHeight, m_wCurrentDPI, iWindowsReferenceDPI);

    return 0;
}

LRESULT
CAboutWindow::_OnPaint()
{
    // Get the window size.
    RECT rcWindow;
    GetClientRect(m_hWnd, &rcWindow);

    // Begin a double-buffered paint.
    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(m_hWnd, &ps);
    HDC hMemDC = CreateCompatibleDC(hDC);
    HBITMAP hMemBitmap = CreateCompatibleBitmap(hDC, rcWindow.right, rcWindow.bottom);
    SelectObject(hMemDC, hMemBitmap);

    // Draw a white rectangle completely filling the header of the window.
    RECT rcHeader = rcWindow;
    rcHeader.bottom = MulDiv(iHeaderHeight, m_wCurrentDPI, iWindowsReferenceDPI);
    FillRect(hMemDC, &rcHeader, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));

    // Draw the header text.
    RECT rcHeaderText = rcHeader;
    rcHeaderText.left = MulDiv(15, m_wCurrentDPI, iWindowsReferenceDPI);
    rcHeaderText.top = MulDiv(15, m_wCurrentDPI, iWindowsReferenceDPI);
    SelectObject(hMemDC, m_pMainWindow->GetBoldGuiFont());
    DrawTextW(hMemDC, m_wstrHeader.c_str(), static_cast<int>(m_wstrHeader.size()), &rcHeaderText, 0);

    // Draw the subheader text.
    RECT rcSubHeaderText = rcHeader;
    rcSubHeaderText.left = MulDiv(20, m_wCurrentDPI, iWindowsReferenceDPI);
    rcSubHeaderText.top = MulDiv(32, m_wCurrentDPI, iWindowsReferenceDPI);
    SelectObject(hMemDC, m_pMainWindow->GetGuiFont());
    DrawTextW(hMemDC, m_wstrSubHeader.c_str(), static_cast<int>(m_wstrSubHeader.size()), &rcSubHeaderText, 0);

    // Draw the logo into the upper right corner.
    const int iLogoPadding = MulDiv(5, m_wCurrentDPI, iWindowsReferenceDPI);
    int iDestBitmapHeight = rcHeader.bottom - 2 * iLogoPadding;
    Gdiplus::Bitmap* bitmap = m_pMainWindow->GetLogoBitmap();
    int iDestBitmapWidth = bitmap->GetWidth() * iDestBitmapHeight / bitmap->GetHeight();
    int iDestBitmapX = rcWindow.right - iLogoPadding - iDestBitmapWidth;
    int iDestBitmapY = iLogoPadding;

    Gdiplus::Graphics g(hMemDC);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQuality);
    g.DrawImage(bitmap, iDestBitmapX, iDestBitmapY, iDestBitmapWidth, iDestBitmapHeight);

    // Fill the rest of the window with the window background color.
    RECT rcBackground = rcWindow;
    rcBackground.top = rcHeader.bottom;
    FillRect(hMemDC, &rcBackground, GetSysColorBrush(COLOR_BTNFACE));

    // End painting by copying the in-memory DC.
    BitBlt(hDC, 0, 0, rcWindow.right, rcWindow.bottom, hMemDC, 0, 0, SRCCOPY);
    DeleteObject(hMemBitmap);
    DeleteDC(hMemDC);
    EndPaint(m_hWnd, &ps);

    return 0;
}

LRESULT
CAboutWindow::_OnSize()
{
    // Get the window size.
    RECT rcWindow;
    GetClientRect(m_hWnd, &rcWindow);

    // Redraw the header on every size change.
    RECT rcHeader = rcWindow;
    rcHeader.bottom = MulDiv(iHeaderHeight, m_wCurrentDPI, iWindowsReferenceDPI);
    InvalidateRect(m_hWnd, &rcHeader, FALSE);

    // Move the button.
    HDWP hDwp = BeginDeferWindowPos(3);

    const int iControlPadding = MulDiv(iUnifiedControlPadding, m_wCurrentDPI, iWindowsReferenceDPI);
    const int iButtonHeight = MulDiv(iUnifiedButtonHeight, m_wCurrentDPI, iWindowsReferenceDPI);
    const int iButtonWidth = MulDiv(iUnifiedButtonWidth, m_wCurrentDPI, iWindowsReferenceDPI);
    int iButtonX = rcWindow.right - iControlPadding - iButtonWidth;
    int iButtonY = rcWindow.bottom - iControlPadding - iButtonHeight;
    if (hDwp)
        hDwp = DeferWindowPos(hDwp, m_hOk, nullptr, iButtonX, iButtonY, iButtonWidth, iButtonHeight, 0);

    // Move the line above the buttons.
    int iLineHeight = 2;
    int iLineWidth = rcWindow.right;
    int iLineX = 0;
    int iLineY = iButtonY - iControlPadding;
    if (hDwp)
        hDwp = DeferWindowPos(hDwp, m_hLine, nullptr, iLineX, iLineY, iLineWidth, iLineHeight, 0);

    int iEditX = iControlPadding;
    int iEditY = rcHeader.bottom + iControlPadding;
    int iEditHeight = iLineY - iEditY - iControlPadding;
    int iEditWidth = rcWindow.right - iEditX - iControlPadding;
    if (hDwp)
        hDwp = DeferWindowPos(hDwp, m_hAbout, nullptr, iEditX, iEditY, iEditWidth, iEditHeight, 0);

    if (hDwp)
        EndDeferWindowPos(hDwp);

    return 0;
}


std::unique_ptr<CAboutWindow>
CAboutWindow::Create(CMainWindow* pMainWindow)
{
    HINSTANCE hInstance = pMainWindow->GetHInstance();

    std::wstring wstrTitle = LoadStringAsWstr(hInstance, IDS_ABOUT_TITLE);
    WCHAR TitleBuffer[MAX_PATH];
    StringCchPrintfW(TitleBuffer, _countof(TitleBuffer), wstrTitle.c_str(), wszAppName);

    if (!g_AboutAtom)
    {
        // Register the about window class.
        WNDCLASSW wc = { 0 };
        wc.lpfnWndProc = CAboutWindow::_WndProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON));
        wc.lpszClassName = CAboutWindow::_wszWndClass;

        g_AboutAtom = RegisterClassW(&wc);

        SafeLoadSystemLibrary(L"msftedit.dll");
    }

    if (g_AboutAtom == 0)
    {
        std::wstring wstrMessage = L"RegisterClassW failed, last error is " + std::to_wstring(GetLastError());
        MessageBoxW(pMainWindow->GetHwnd(), wstrMessage.c_str(), TitleBuffer, MB_ICONERROR);
        return nullptr;
    }

    const int iControlPadding = MulDiv(iUnifiedControlPadding, pMainWindow->GetCurrentDPI(), iWindowsReferenceDPI);
    POINT pt{ iControlPadding, iControlPadding };
    ClientToScreen(pMainWindow->GetHwnd(), &pt);

    // Create the about window.
    auto pAboutWindow = std::unique_ptr<CAboutWindow>(new CAboutWindow(pMainWindow));
    HWND hWnd = CreateWindowExW(
        0,
        CAboutWindow::_wszWndClass,
        TitleBuffer,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        pt.x,
        pt.y,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        pMainWindow->GetHwnd(),
        nullptr,
        hInstance,
        pAboutWindow.get());
    if (hWnd == nullptr)
    {
        std::wstring wstrMessage = L"CreateWindowExW failed for CAboutWindow, last error is " + std::to_wstring(GetLastError());
        MessageBoxW(nullptr, wstrMessage.c_str(), TitleBuffer, MB_ICONERROR);
        return nullptr;
    }

    return pAboutWindow;
}

void
CAboutWindow::DoModal()
{
    BOOL fWasDisabled = EnableWindow(m_pMainWindow->GetHwnd(), FALSE);
    m_OwnerWasActive = !fWasDisabled;

    MSG msg;
    while (m_Active && GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}
