#include "pch.h"
#include "Callbacks.h"
#include "Shared.h"
#include "Event.h"


/****************************************************************
  WH_CALLWNDPROC hook procedure
 ****************************************************************/

LRESULT WINAPI CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
#if 1
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_CALLWNDPROC;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.msg.wParam = wParam;
    Event.Info.Hook.msg.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
#else
    CHAR szCWPBuf[256];
    CHAR szMsg[16];
    HDC hdc;
    static int c = 0;
    size_t cch;
    HRESULT hResult;

    if (nCode < 0)  // do not process message 
        return CallNextHookEx(myhookdata[IDM_CALLWNDPROC].hhook, nCode, wParam, lParam);

    // Call an application-defined function that converts a message 
    // constant to a string and copies it to a buffer. 

    LookUpTheMessage((PMSG)lParam, szMsg);

    hdc = GetDC(gh_hwndMain);

    switch (nCode)
    {
    case HC_ACTION:
        hResult = StringCchPrintf(szCWPBuf, 256 / sizeof(TCHAR),
            "CALLWNDPROC - tsk: %ld, msg: %s, %d times   ",
            wParam, szMsg, c++);
        if (FAILED(hResult))
        {
            // TODO: writer error handler
        }
        hResult = StringCchLength(szCWPBuf, 256 / sizeof(TCHAR), &cch);
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        TextOut(hdc, 2, 15, szCWPBuf, cch);
        break;

    default:
        break;
    }

    ReleaseDC(gh_hwndMain, hdc);

    return CallNextHookEx(myhookdata[IDM_CALLWNDPROC].hhook, nCode, wParam, lParam);
#endif
}

/****************************************************************
  WH_GETMESSAGE hook procedure
 ****************************************************************/

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
#if 1
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_GETMESSAGE;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.msg.wParam = wParam;
    Event.Info.Hook.msg.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
#else
    CHAR szMSGBuf[256];
    CHAR szRem[16];
    CHAR szMsg[16];
    HDC hdc;
    static int c = 0;
    size_t cch;
    HRESULT hResult;

    if (nCode < 0) // do not process message 
        return CallNextHookEx(myhookdata[IDM_GETMESSAGE].hhook, nCode,
            wParam, lParam);

    switch (nCode)
    {
    case HC_ACTION:
        switch (wParam)
        {
        case PM_REMOVE:
            hResult = StringCchCopy(szRem, 16 / sizeof(TCHAR), "PM_REMOVE");
            if (FAILED(hResult))
            {
                // TODO: write error handler
            }
            break;

        case PM_NOREMOVE:
            hResult = StringCchCopy(szRem, 16 / sizeof(TCHAR), "PM_NOREMOVE");
            if (FAILED(hResult))
            {
                // TODO: write error handler
            }
            break;

        default:
            hResult = StringCchCopy(szRem, 16 / sizeof(TCHAR), "Unknown");
            if (FAILED(hResult))
            {
                // TODO: write error handler
            }
            break;
        }

        // Call an application-defined function that converts a 
        // message constant to a string and copies it to a 
        // buffer. 

        LookUpTheMessage((PMSG)lParam, szMsg);

        hdc = GetDC(gh_hwndMain);
        hResult = StringCchPrintf(szMSGBuf, 256 / sizeof(TCHAR),
            "GETMESSAGE - wParam: %s, msg: %s, %d times   ",
            szRem, szMsg, c++);
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        hResult = StringCchLength(szMSGBuf, 256 / sizeof(TCHAR), &cch);
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        TextOut(hdc, 2, 35, szMSGBuf, cch);
        break;

    default:
        break;
    }

    ReleaseDC(gh_hwndMain, hdc);

    return CallNextHookEx(myhookdata[IDM_GETMESSAGE].hhook, nCode, wParam, lParam);
#endif
}

/****************************************************************
  WH_DEBUG hook procedure
 ****************************************************************/

LRESULT CALLBACK DebugProc(int nCode, WPARAM wParam, LPARAM lParam)
{
#if 1
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_DEBUG;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.msg.wParam = wParam;
    Event.Info.Hook.msg.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
#else

    CHAR szBuf[128];
    HDC hdc;
    static int c = 0;
    size_t cch;
    HRESULT hResult;

    if (nCode < 0)  // do not process message 
        return CallNextHookEx(myhookdata[IDM_DEBUG].hhook, nCode,
            wParam, lParam);

    hdc = GetDC(gh_hwndMain);

    switch (nCode)
    {
    case HC_ACTION:
        hResult = StringCchPrintf(szBuf, 128 / sizeof(TCHAR),
            "DEBUG - nCode: %d, tsk: %ld, %d times   ",
            nCode, wParam, c++);
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        hResult = StringCchLength(szBuf, 128 / sizeof(TCHAR), &cch);
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        TextOut(hdc, 2, 55, szBuf, cch);
        break;

    default:
        break;
    }

    ReleaseDC(gh_hwndMain, hdc);

    return CallNextHookEx(myhookdata[IDM_DEBUG].hhook, nCode, wParam, lParam);
#endif
}

/****************************************************************
  WH_CBT hook procedure
 ****************************************************************/

LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
#if 1
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_CBT;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.msg.wParam = wParam;
    Event.Info.Hook.msg.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
#else

    CHAR szBuf[128];
    CHAR szCode[128];
    HDC hdc;
    static int c = 0;
    size_t cch;
    HRESULT hResult;

    if (nCode < 0)  // do not process message 
        return CallNextHookEx(myhookdata[IDM_CBT].hhook, nCode, wParam,
            lParam);

    hdc = GetDC(gh_hwndMain);

    switch (nCode)
    {
    case HCBT_ACTIVATE:
        hResult = StringCchCopy(szCode, 128 / sizeof(TCHAR), "HCBT_ACTIVATE");
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;

    case HCBT_CLICKSKIPPED:
        hResult = StringCchCopy(szCode, 128 / sizeof(TCHAR), "HCBT_CLICKSKIPPED");
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;

    case HCBT_CREATEWND:
        hResult = StringCchCopy(szCode, 128 / sizeof(TCHAR), "HCBT_CREATEWND");
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;

    case HCBT_DESTROYWND:
        hResult = StringCchCopy(szCode, 128 / sizeof(TCHAR), "HCBT_DESTROYWND");
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;

    case HCBT_KEYSKIPPED:
        hResult = StringCchCopy(szCode, 128 / sizeof(TCHAR), "HCBT_KEYSKIPPED");
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;

    case HCBT_MINMAX:
        hResult = StringCchCopy(szCode, 128 / sizeof(TCHAR), "HCBT_MINMAX");
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;

    case HCBT_MOVESIZE:
        hResult = StringCchCopy(szCode, 128 / sizeof(TCHAR), "HCBT_MOVESIZE");
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;

    case HCBT_QS:
        hResult = StringCchCopy(szCode, 128 / sizeof(TCHAR), "HCBT_QS");
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;

    case HCBT_SETFOCUS:
        hResult = StringCchCopy(szCode, 128 / sizeof(TCHAR), "HCBT_SETFOCUS");
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;

    case HCBT_SYSCOMMAND:
        hResult = StringCchCopy(szCode, 128 / sizeof(TCHAR), "HCBT_SYSCOMMAND");
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;

    default:
        hResult = StringCchCopy(szCode, 128 / sizeof(TCHAR), "Unknown");
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;
    }
    hResult = StringCchPrintf(szBuf, 128 / sizeof(TCHAR), "CBT -  nCode: %s, tsk: %ld, %d times   ",
        szCode, wParam, c++);
    if (FAILED(hResult))
    {
        // TODO: write error handler
    }
    hResult = StringCchLength(szBuf, 128 / sizeof(TCHAR), &cch);
    if (FAILED(hResult))
    {
        // TODO: write error handler
    }
    TextOut(hdc, 2, 75, szBuf, cch);
    ReleaseDC(gh_hwndMain, hdc);

    return CallNextHookEx(myhookdata[IDM_CBT].hhook, nCode, wParam, lParam);
#endif
}

/****************************************************************
  WH_MOUSE hook procedure
 ****************************************************************/

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
#if 1
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_MOUSE;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.msg.wParam = wParam;
    Event.Info.Hook.msg.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
#else

    CHAR szBuf[128];
    CHAR szMsg[16];
    HDC hdc;
    static int c = 0;
    size_t cch;
    HRESULT hResult;

    if (nCode < 0)  // do not process the message 
        return CallNextHookEx(myhookdata[IDM_MOUSE].hhook, nCode,
            wParam, lParam);

    // Call an application-defined function that converts a message 
    // constant to a string and copies it to a buffer. 

    LookUpTheMessage((PMSG)lParam, szMsg);

    hdc = GetDC(gh_hwndMain);
    hResult = StringCchPrintf(szBuf, 128 / sizeof(TCHAR),
        "MOUSE - nCode: %d, msg: %s, x: %d, y: %d, %d times   ",
        nCode, szMsg, LOWORD(lParam), HIWORD(lParam), c++);
    if (FAILED(hResult))
    {
        // TODO: write error handler
    }
    hResult = StringCchLength(szBuf, 128 / sizeof(TCHAR), &cch);
    if (FAILED(hResult))
    {
        // TODO: write error handler
    }
    TextOut(hdc, 2, 95, szBuf, cch);
    ReleaseDC(gh_hwndMain, hdc);

    return CallNextHookEx(myhookdata[IDM_MOUSE].hhook, nCode, wParam, lParam);
#endif
}

/****************************************************************
  WH_KEYBOARD hook procedure
 ****************************************************************/

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
#if 1
    HOOK_EVENT Event = { 0 };
    Event.HookType = WH_KEYBOARD;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.msg.wParam = wParam;
    Event.Info.Hook.msg.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
#else

    CHAR szBuf[128];
    HDC hdc;
    static int c = 0;
    size_t cch;
    HRESULT hResult;

    if (nCode < 0)  // do not process message 
        return CallNextHookEx(myhookdata[IDM_KEYBOARD].hhook, nCode,
            wParam, lParam);

    hdc = GetDC(gh_hwndMain);
    hResult = StringCchPrintf(szBuf, 128 / sizeof(TCHAR), "KEYBOARD - nCode: %d, vk: %d, %d times ", nCode, wParam, c++);
    if (FAILED(hResult))
    {
        // TODO: write error handler
    }
    hResult = StringCchLength(szBuf, 128 / sizeof(TCHAR), &cch);
    if (FAILED(hResult))
    {
        // TODO: write error handler
    }
    TextOut(hdc, 2, 115, szBuf, cch);
    ReleaseDC(gh_hwndMain, hdc);

    return CallNextHookEx(myhookdata[IDM_KEYBOARD].hhook, nCode, wParam, lParam);
#endif
}

/****************************************************************
  WH_MSGFILTER hook procedure
 ****************************************************************/

LRESULT CALLBACK MessageProc(int nCode, WPARAM wParam, LPARAM lParam)
{
#if 1
    HOOK_EVENT Event = { 0 };
    Event.HookType = (DWORD)WH_MSGFILTER;
    Event.Info.Hook.nCode = nCode;
    Event.Info.Hook.msg.wParam = wParam;
    Event.Info.Hook.msg.lParam = lParam;
    Event_Push(Event);

    return CallNextHookEx(0, nCode, wParam, lParam);
#else

    CHAR szBuf[128];
    CHAR szMsg[16];
    CHAR szCode[32];
    HDC hdc;
    static int c = 0;
    size_t cch;
    HRESULT hResult;

    if (nCode < 0)  // do not process message 
        return CallNextHookEx(myhookdata[IDM_MSGFILTER].hhook, nCode,
            wParam, lParam);

    switch (nCode)
    {
    case MSGF_DIALOGBOX:
        hResult = StringCchCopy(szCode, 32 / sizeof(TCHAR), "MSGF_DIALOGBOX");
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;

    case MSGF_MENU:
        hResult = StringCchCopy(szCode, 32 / sizeof(TCHAR), "MSGF_MENU");
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;

    case MSGF_SCROLLBAR:
        hResult = StringCchCopy(szCode, 32 / sizeof(TCHAR), "MSGF_SCROLLBAR");
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;

    default:
        hResult = StringCchPrintf(szCode, 128 / sizeof(TCHAR), "Unknown: %d", nCode);
        if (FAILED(hResult))
        {
            // TODO: write error handler
        }
        break;
    }

    // Call an application-defined function that converts a message 
    // constant to a string and copies it to a buffer. 

    LookUpTheMessage((PMSG)lParam, szMsg);

    hdc = GetDC(gh_hwndMain);
    hResult = StringCchPrintf(szBuf, 128 / sizeof(TCHAR),
        "MSGFILTER  nCode: %s, msg: %s, %d times    ",
        szCode, szMsg, c++);
    if (FAILED(hResult))
    {
        // TODO: write error handler
    }
    hResult = StringCchLength(szBuf, 128 / sizeof(TCHAR), &cch);
    if (FAILED(hResult))
    {
        // TODO: write error handler
    }
    TextOut(hdc, 2, 135, szBuf, cch);
    ReleaseDC(gh_hwndMain, hdc);

    return CallNextHookEx(myhookdata[IDM_MSGFILTER].hhook, nCode, wParam, lParam);
#endif
}
