//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// UI Framework: Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#include "Wizard-2020.h"

#define IDC_HOOKTYPE    600

LRESULT
CFirstPage::_OnCreate()
{
    HINSTANCE hInstance = m_pMainWindow->GetHInstance();

    // Load resources.
    m_wstrHeader = LoadStringAsWstr(hInstance, IDS_FIRSTPAGE_HEADER);
    m_wstrSubHeader = LoadStringAsWstr(hInstance, IDS_FIRSTPAGE_SUBHEADER);
 
    // Set up the ComboBox.
    m_hComboHookType = CreateWindowExW(0, WC_COMBOBOX, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, reinterpret_cast<HMENU>(IDC_HOOKTYPE), nullptr, nullptr);

    // Set up the radio boxes.
    std::wstring text = LoadStringAsWstr(hInstance, IDS_GLOBAL_HOOK);
    m_hRadioGlobal = CreateWindowExW(0, WC_BUTTON, text.c_str(), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, nullptr, nullptr, nullptr);
    text = LoadStringAsWstr(hInstance, IDS_LOCAL_HOOK);
    m_hRadioLocal = CreateWindowExW(0, WC_BUTTON, text.c_str(), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, nullptr, nullptr, nullptr);

    // Set up the check boxes (options).

    text = LoadStringAsWstr(hInstance, IDS_OPT_IGNORE_OWN_MSG);
    m_hIgnoreOwnMsg = CreateWindowExW(0, WC_BUTTON, text.c_str(), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, nullptr, nullptr, nullptr);
    text = LoadStringAsWstr(hInstance, IDS_OPT_BREAK_ON_LOAD);
    m_hBreakOnDllLoad = CreateWindowExW(0, WC_BUTTON, text.c_str(), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, nullptr, nullptr, nullptr);
    text = LoadStringAsWstr(hInstance, IDS_OPT_BREAK_ON_UNLOAD);
    m_hBreakOnDllUnload = CreateWindowExW(0, WC_BUTTON, text.c_str(), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, nullptr, nullptr, nullptr);
    text = LoadStringAsWstr(hInstance, IDS_OPT_BREAK_ON_FIRST_MSG);
    m_hBreakOnFirstMsg = CreateWindowExW(0, WC_BUTTON, text.c_str(), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, nullptr, nullptr, nullptr);

    // Set all fonts
    UpdateDPI();

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

    Button_SetCheck(m_hRadioGlobal, BST_CHECKED);
    Button_SetCheck(m_hIgnoreOwnMsg, BST_CHECKED);

    return 0;
}

LRESULT
CFirstPage::_OnDestroy()
{
    // Ensure we do not break while shutting down!
    m_Settings->BreakOnUnload = FALSE;

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

    const int iControlPadding = m_pMainWindow->DefaultControlPaddingPx();
    const int iButtonHeight = m_pMainWindow->DefaultButtonHeightPx();
    //const int iButtonWidth = m_pMainWindow->DefaultButtonWidthPx();

    HDWP hDwp = BeginDeferWindowPos(7);

    // Move the combobox.
    int iControlX = 0;
    int iControlY = 0;
    int iControlWidth = rcWindow.right / 3 - iControlPadding / 2;
    if (hDwp)
        hDwp = DeferWindowPos(hDwp, m_hComboHookType, nullptr, iControlX, iControlY, iControlWidth, iButtonHeight, 0);

    // Move the radio buttons.
    iControlX = rcWindow.right / 3 + iControlPadding / 2;
    iControlWidth = rcWindow.right / 3 - iControlPadding;
    if (hDwp)
        hDwp = DeferWindowPos(hDwp, m_hRadioGlobal, nullptr, iControlX, iControlY, iControlWidth, iButtonHeight, 0);

    iControlY = iButtonHeight;
    if (hDwp)
        hDwp = DeferWindowPos(hDwp, m_hRadioLocal, nullptr, iControlX, iControlY, iControlWidth, iButtonHeight, 0);

    // Move the checkboxes
    const int iCheckX = 0;
    int iCheckY = iControlY * 3;
    const int iCheckWidth = rcWindow.right;

    HWND hCheckboxes[] = { m_hIgnoreOwnMsg, m_hBreakOnDllLoad, m_hBreakOnDllUnload, m_hBreakOnFirstMsg };

    for (HWND hCheckbox : hCheckboxes)
    {
        if (hDwp)
            hDwp = DeferWindowPos(hDwp, hCheckbox, nullptr, iCheckX, iCheckY, iCheckWidth, iButtonHeight, 0);

        iCheckY += iButtonHeight;
    }

    if (hDwp)
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
            case WM_DESTROY: return pPage->_OnDestroy();
            case WM_SIZE: return pPage->_OnSize();
            case WM_COMMAND: return pPage->_OnCommand(wParam, lParam);
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT
CFirstPage::_OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
    if (LOWORD(wParam) == IDC_HOOKTYPE)
    {
        if (HIWORD(wParam) == CBN_SELCHANGE)
        {
            int index = ComboBox_GetCurSel(m_hComboHookType);
            int idHook = static_cast<int>(ComboBox_GetItemData(m_hComboHookType, index));
            BOOL fSupportsLocalHook = HookDll_CanBeSetLocal(idHook);
            Button_Enable(m_hRadioLocal, fSupportsLocalHook);
            if (!fSupportsLocalHook)
            {
                Button_SetCheck(m_hRadioGlobal, BST_CHECKED);
                Button_SetCheck(m_hRadioLocal, BST_UNCHECKED);
            }
        }
    }

    return 0;
}

void
CFirstPage::SwitchTo()
{
    m_pMainWindow->SetHeader(&m_wstrHeader, &m_wstrSubHeader);
    m_pMainWindow->EnableBackButton(FALSE);
    m_pMainWindow->EnableNextButton(TRUE, IDS_NEXT);
    ShowWindow(m_hWnd, SW_SHOW);
}

void
CFirstPage::OnBack()
{

}

void
CFirstPage::OnNext()
{
    // Configure hook type.
    int index = ComboBox_GetCurSel(m_hComboHookType);
    m_Settings->idHook = ComboBox_GetItemData(m_hComboHookType, index);
    m_Settings->GlobalHook = Button_GetCheck(m_hRadioGlobal) == BST_CHECKED;

    // We don't know the hWnd of the second page, so put a sentinel in place,
    // so that the second page can fill in the correct value
    if (Button_GetCheck(m_hIgnoreOwnMsg) == BST_CHECKED)
        m_Settings->IgnoreWnd = (HWND)TRUE;
    else
        m_Settings->IgnoreWnd = 0;

    m_Settings->BreakOnLoad = Button_GetCheck(m_hBreakOnDllLoad) == BST_CHECKED;
    m_Settings->BreakOnUnload = Button_GetCheck(m_hBreakOnDllUnload) == BST_CHECKED;
    m_Settings->BreakOnFirstEvent = Button_GetCheck(m_hBreakOnFirstMsg) == BST_CHECKED;
}

void
CFirstPage::UpdateDPI()
{
    HFONT hGuiFont = m_pMainWindow->GetGuiFont();

    // Update the control fonts.
    HWND hControls[] = {
        m_hComboHookType,
        m_hRadioGlobal,
        m_hRadioLocal,
        m_hIgnoreOwnMsg,
        m_hBreakOnDllLoad,
        m_hBreakOnDllUnload,
        m_hBreakOnFirstMsg,
    };

    for (HWND hControl : hControls)
    {
        SendMessageW(hControl, WM_SETFONT, reinterpret_cast<WPARAM>(hGuiFont), MAKELPARAM(TRUE, 0));
    }
}

