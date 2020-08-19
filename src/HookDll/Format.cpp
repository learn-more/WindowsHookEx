//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// SPDX-License-Identifier: MIT
//

#include "pch.h"
#include <strsafe.h>
#include "Format.h"

#define CASE_RETURN(x)  case x: return L ## #x;

LPCWSTR Format_HSHELL(int nCode)
{
    static WCHAR Buf[15];
    switch (nCode)
    {
        CASE_RETURN(HSHELL_WINDOWCREATED);
        CASE_RETURN(HSHELL_WINDOWDESTROYED);
        CASE_RETURN(HSHELL_ACTIVATESHELLWINDOW);
        CASE_RETURN(HSHELL_WINDOWACTIVATED);
        CASE_RETURN(HSHELL_GETMINRECT);
        CASE_RETURN(HSHELL_REDRAW);
        CASE_RETURN(HSHELL_TASKMAN);
        CASE_RETURN(HSHELL_LANGUAGE);
        CASE_RETURN(HSHELL_SYSMENU);
        CASE_RETURN(HSHELL_ENDTASK);
        CASE_RETURN(HSHELL_ACCESSIBILITYSTATE);
        CASE_RETURN(HSHELL_APPCOMMAND);
        CASE_RETURN(HSHELL_WINDOWREPLACED);
        CASE_RETURN(HSHELL_WINDOWREPLACING);
        CASE_RETURN(HSHELL_HIGHBIT);
        CASE_RETURN(HSHELL_FLASH);
        CASE_RETURN(HSHELL_RUDEAPPACTIVATED);
    default:
        StringCchPrintfW(Buf, ARRAYSIZE(Buf), L"?%d?", nCode);
        return Buf;
    }
}

LPCWSTR Format_CBT(int nCode)
{
    static WCHAR Buf[15];
    switch (nCode)
    {
        CASE_RETURN(HCBT_MOVESIZE);
        CASE_RETURN(HCBT_MINMAX);
        CASE_RETURN(HCBT_QS);
        CASE_RETURN(HCBT_CREATEWND);
        CASE_RETURN(HCBT_DESTROYWND);
        CASE_RETURN(HCBT_ACTIVATE);
        CASE_RETURN(HCBT_CLICKSKIPPED);
        CASE_RETURN(HCBT_KEYSKIPPED);
        CASE_RETURN(HCBT_SYSCOMMAND);
        CASE_RETURN(HCBT_SETFOCUS);
    default:
        StringCchPrintfW(Buf, ARRAYSIZE(Buf), L"?%d?", nCode);
        return Buf;
    }
}

LPCWSTR Format_Action(int nCode)
{
    static WCHAR Buf[15];
    switch (nCode)
    {
        CASE_RETURN(HC_ACTION);
        CASE_RETURN(HC_GETNEXT);
        CASE_RETURN(HC_SKIP);
        CASE_RETURN(HC_NOREMOVE);
        CASE_RETURN(HC_SYSMODALON);
        CASE_RETURN(HC_SYSMODALOFF);
    default:
        StringCchPrintfW(Buf, ARRAYSIZE(Buf), L"?%d?", nCode);
        return Buf;
    }
}

LPCWSTR Format_PM(int nCode)
{
    static WCHAR Buf[15];
    switch (nCode)
    {
        CASE_RETURN(PM_NOREMOVE);
        CASE_RETURN(PM_REMOVE);
        CASE_RETURN(PM_NOYIELD);
        CASE_RETURN(PM_QS_INPUT);
        CASE_RETURN(PM_QS_POSTMESSAGE);
        CASE_RETURN(PM_QS_PAINT);
        CASE_RETURN(PM_QS_SENDMESSAGE);
    default:
        StringCchPrintfW(Buf, ARRAYSIZE(Buf), L"?%d?", nCode);
        return Buf;
    }
}



