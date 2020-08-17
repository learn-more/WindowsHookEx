#include "pch.h"
#include "Shared.h"
#include "SharedMem.h"

bool Event_Ignored(HWND hWnd)
{
    HWND hIgnore = SharedMem_Pointer()->Settings.IgnoreWnd;
    return hWnd == hIgnore ||
        IsChild(hIgnore, hWnd);
}

bool Queue_Lock(SHARED_MEM_QUEUE* queue, UINT* FailureCounter)
{
    UINT Counter = 0;

    DWORD pid = GetCurrentProcessId();

    while (InterlockedCompareExchange(&queue->Busy, pid, 0) != 0)
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

void Queue_Unlock(SHARED_MEM_QUEUE* queue)
{
    InterlockedExchange(&queue->Busy, 0);
}

void Event_Push(HOOK_EVENT& event)
{
    event.ProcessId = GetCurrentProcessId();
    event.ThreadId = GetCurrentThreadId();

    SHARED_MEM_QUEUE* queue = &SharedMem_Pointer()->Queue;

    if (!Queue_Lock(queue, &queue->WriteAbort))
        return;

    if (((queue->Head + 1) % MAX_EVENTS) == queue->Tail)
    {
        queue->Dropped++;
        Queue_Unlock(queue);
        return;
    }

    queue->Events[queue->Head] = event;
    queue->Head = (queue->Head + 1) % MAX_EVENTS;

    Queue_Unlock(queue);
}

void Event_Pop(HOOK_EVENT* event, size_t& NumEvents)
{
    SHARED_MEM_QUEUE* queue = &SharedMem_Pointer()->Queue;

    size_t readEvents = NumEvents;
    NumEvents = 0;

    if (queue->Tail != queue->Head)
    {
        if (!Queue_Lock(queue, &queue->ReadAbort))
            return;

        while (queue->Tail != queue->Head && readEvents)
        {
            *event = queue->Events[queue->Tail];
            queue->Tail = (queue->Tail + 1) % MAX_EVENTS;

            event++;
            NumEvents++;
            readEvents--;
        }
        Queue_Unlock(queue);
    }
}
