//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020-2023 Mark Jansen
// UI Framework: Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#pragma once

// Enable Visual Styles
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <string>

#include "targetver.h"
#include <Windows.h>
#include <CommCtrl.h>
#include <WindowsX.h>
#include <strsafe.h>

#pragma warning(push)
#pragma warning(disable:4458)
#include <GdiPlus.h>
#pragma warning(pop)

#if !defined(WM_DPICHANGED)
#define WM_DPICHANGED 0x02E0
#endif

#include "unique_resource.h"
#include "win32_wrappers.h"
#include "csv_export.h"

#include "resource.h"
#include "utils.h"

// Forward declarations
class CSelectHookPage;
class CMainWindow;
class CPage;
class CHookOutputPage;

#include "HookDll/Shared.h"
#include "CPage.h"
#include "CSelectHookPage.h"
#include "CHookOutputPage.h"
#include "CAboutWindow.h"
#include "CAtomWindow.h"
#include "CMainWindow.h"

// Wizard-2020.cpp
extern const int iFontReferenceDPI;
extern const int iWindowsReferenceDPI;
extern const int iUnifiedControlPadding;
extern const int iUnifiedButtonHeight;
extern const int iUnifiedButtonWidth;
extern const WCHAR wszAppName[];

// AlternateCallbacks.cpp
HOOKPROC
GetHostProc(int idHook);

