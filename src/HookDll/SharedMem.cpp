#include "pch.h"
#include "Shared.h"
#include "SharedMem.h"

// Actual capacity is one less!
const auto MAX_EVENTS = 4096;

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


static HANDLE hMapObject = NULL;
static SHARED_MEM* lpMem = NULL;
static DWORD g_CurrentProcessId = 0;
static __declspec(thread) DWORD g_CurrentThread = 0;


void SharedMem_Open(BOOL& fIsFirst)
{
    g_CurrentProcessId = GetCurrentProcessId();
    g_CurrentThread = GetCurrentThreadId();

    hMapObject = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SHARED_MEM), L"WindowsHookEx-F88C997F-BCDC-4bc2-B54D-419919489FDA");
    fIsFirst = (GetLastError() != ERROR_ALREADY_EXISTS);

    lpMem = (SHARED_MEM*)MapViewOfFile(hMapObject, FILE_MAP_WRITE, 0, 0, 0);

    if (lpMem == NULL)
        return;

    if (fIsFirst)
    {
        memset(lpMem, '\0', sizeof(SHARED_MEM));
    }
}

void SharedMem_Close(void)
{
    if (lpMem)
        UnmapViewOfFile(lpMem);

    if (hMapObject)
        CloseHandle(hMapObject);
}

SHARED_SETTINGS* SharedMem_Settings()
{
    return &lpMem->Settings;
}

bool SharedMem_Lock(SHARED_MEM_QUEUE* queue, UINT* FailureCounter)
{
    UINT Counter = 0;

    while (InterlockedCompareExchange(&queue->Busy, g_CurrentProcessId, 0) != 0)
    {
        Sleep(0);
        Counter++;
        if (Counter > 1000)
        {
            InterlockedIncrement(FailureCounter);
            return false;
        }
    }

    return true;
}

void SharedMem_Unlock(SHARED_MEM_QUEUE* queue)
{
    InterlockedExchange(&queue->Busy, 0);
}

void SharedMem_Push(HOOK_EVENT& event)
{
    if (g_CurrentThread == 0)
        g_CurrentThread = GetCurrentThreadId();

    event.ProcessId = g_CurrentProcessId;
    event.ThreadId = g_CurrentThread;

    SHARED_MEM_QUEUE* queue = &lpMem->Queue;

    if (!SharedMem_Lock(queue, &queue->WriteAbort))
        return;

    if (((queue->Head + 1) % MAX_EVENTS) == queue->Tail)
    {
        queue->Dropped++;
        SharedMem_Unlock(queue);
        return;
    }

    queue->Events[queue->Head] = event;
    queue->Head = (queue->Head + 1) % MAX_EVENTS;

    SharedMem_Unlock(queue);
}

bool SharedMem_Pop(HOOK_EVENT& event)
{
    SHARED_MEM_QUEUE* queue = &lpMem->Queue;

    if (queue->Tail != queue->Head)
    {
        if (!SharedMem_Lock(queue, &queue->ReadAbort))
            return false;

        if (queue->Tail != queue->Head)
        {
            event = queue->Events[queue->Tail];
            queue->Tail = (queue->Tail + 1) % MAX_EVENTS;

            SharedMem_Unlock(queue);
            return true;
        }
        SharedMem_Unlock(queue);
    }

    return false;
}
