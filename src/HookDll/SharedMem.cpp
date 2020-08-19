//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// SPDX-License-Identifier: MIT
//

#include "pch.h"
#include "Shared.h"
#include "SharedMem.h"


static HANDLE hMapObject = NULL;
static SHARED_MEM* lpMem = NULL;


void SharedMem_Open(BOOL& fIsFirst)
{
    hMapObject = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SHARED_MEM), L"WindowsHookEx-F88C997F-BCDC-4bc2-B54D-419919489FDA");
    fIsFirst = (GetLastError() != ERROR_ALREADY_EXISTS);

    lpMem = (SHARED_MEM*)MapViewOfFile(hMapObject, FILE_MAP_WRITE, 0, 0, 0);

    if (lpMem == NULL)
        return;

    if (fIsFirst)
    {
        memset(lpMem, 0, sizeof(SHARED_MEM));
    }
}

void SharedMem_Close(void)
{
    if (lpMem)
        UnmapViewOfFile(lpMem);

    if (hMapObject)
        CloseHandle(hMapObject);
}

SHARED_MEM* SharedMem_Pointer(void)
{
    return lpMem;
}

