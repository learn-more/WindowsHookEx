//
// Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#include "Wizard-2020.h"


LRESULT
CFirstPage::_OnCreate()
{
    // Load resources.
    m_wstrHeader = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_FIRSTPAGE_HEADER);
    m_wstrSubHeader = LoadStringAsWstr(m_pMainWindow->GetHInstance(), IDS_FIRSTPAGE_SUBHEADER);
 
    // Set up the ComboBox.
    m_hComboHookType = CreateWindowExW(WS_EX_CLIENTEDGE, WC_COMBOBOX, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 0, 0, 0, 0, m_hWnd, nullptr, nullptr, nullptr);
    SendMessageW(m_hComboHookType, WM_SETFONT, reinterpret_cast<WPARAM>(m_pMainWindow->GetGuiFont()), MAKELPARAM(TRUE, 0));

    m_Settings = HookDll_GetSettings();

    for (int n = WH_MIN; n <= WH_MAX; ++n)
    {
        LPCWSTR wstrText = HookDll_HookName(n);
        if (wstrText)
        {
            int index = ComboBox_AddString(m_hComboHookType, wstrText);
            ComboBox_SetItemData(m_hComboHookType, index, n);
            if (n == m_Settings->idHook)
            {
                ComboBox_SetCurSel(m_hComboHookType, index);
            }
        }
    }

    return 0;
}

LRESULT
CFirstPage::_OnSize()
{
    // Get the window size.
    RECT rcWindow;
    GetClientRect(m_hWnd, &rcWindow);

    // The text is drawn on most of the window, so invalidate that.
    InvalidateRect(m_hWnd, &rcWindow, TRUE);
    // Move the list control.
    HDWP hDwp = BeginDeferWindowPos(1);

    int iCboLeft = 0;
    int iCboTop = 0;
    int iCboWidth = rcWindow.right / 3;// = rcWindow.bottom;
    int iCboHeight = 3;// rcWindow.right;
    DeferWindowPos(hDwp, m_hComboHookType, nullptr, iCboLeft, iCboTop, iCboWidth, iCboHeight, 0);

    EndDeferWindowPos(hDwp);

    return 0;
}

LRESULT CALLBACK
CFirstPage::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CFirstPage* pPage = InstanceFromWndProc<CFirstPage, CPage, &CFirstPage::CPage::m_hWnd>(hWnd, uMsg, lParam);

    if (pPage)
    {
        switch (uMsg)
        {
            case WM_CREATE: return pPage->_OnCreate();
            case WM_SIZE: return pPage->_OnSize();
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void
CFirstPage::SwitchTo()
{
    m_pMainWindow->SetHeader(&m_wstrHeader, &m_wstrSubHeader);
    m_pMainWindow->EnableBackButton(FALSE);
    m_pMainWindow->EnableNextButton(TRUE);
    ShowWindow(m_hWnd, SW_SHOW);
}

void
CFirstPage::UpdateDPI()
{
    // Update the control fonts.
    SendMessageW(m_hComboHookType, WM_SETFONT, reinterpret_cast<WPARAM>(m_pMainWindow->GetGuiFont()), MAKELPARAM(TRUE, 0));
}

