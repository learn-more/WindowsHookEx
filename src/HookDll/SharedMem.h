//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// SPDX-License-Identifier: MIT
//

#pragma once

const auto MAX_EVENTS = 4096 * 8;

struct SHARED_MEM_QUEUE
{
    UINT Busy;
    UINT Head;
    UINT Tail;
    UINT Dropped;
    UINT WriteAbort;
    UINT ReadAbort;
    HOOK_EVENT Events[MAX_EVENTS];
};

struct SHARED_MEM
{
    SHARED_SETTINGS Settings;
    SHARED_MEM_QUEUE Queue;
};


void SharedMem_Open(BOOL& fIsFirst);
void SharedMem_Close(void);

SHARED_MEM* SharedMem_Pointer(void);

