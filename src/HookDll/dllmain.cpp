//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// SPDX-License-Identifier: MIT
//

#include "pch.h"
#include "Shared.h"
#include "SharedMem.h"
#include "Event.h"
#include <string>
#include <strsafe.h>

static std::wstring g_ProcessName;


std::wstring getModuleFilename(HMODULE Module)
{
    std::wstring Buffer;

    DWORD returned = 0;
    do
    {
        Buffer.resize(Buffer.size() + MAX_PATH);
        returned = GetModuleFileNameW(Module, Buffer.data(), static_cast<DWORD>(Buffer.size()));
    } while (returned >= Buffer.size());

    Buffer.resize(returned);

    return Buffer;
}


BOOL APIENTRY DllMain( HMODULE /*hModule*/,
                       DWORD  ul_reason_for_call,
                       LPVOID /*lpReserved*/
                     )
{
    BOOL fIsFirst = FALSE;
    HOOK_EVENT Event = { 0 };

    SHARED_MEM* Mem;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        SharedMem_Open(fIsFirst);
        Mem = SharedMem_Pointer();
        InterlockedIncrement(&Mem->Common.NumberOfDllsLoaded);

        if (fIsFirst)
        {
            Mem->Settings.idHook = WH_CBT;
            Mem->Common.HostProcess = GetCurrentProcessId();
        }

        g_ProcessName = getModuleFilename(NULL);
        {
            std::wstring::size_type slash = g_ProcessName.find_last_of(L"\\/");
            if (slash != std::wstring::npos)
                g_ProcessName = g_ProcessName.substr(slash + 1);
        }

        Event.HookType = EVENT_DLL_LOAD;
        StringCchCopy(Event.Info.Buffer, _countof(Event.Info.Buffer), g_ProcessName.c_str());
        Event_Push(Event);

        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        Mem = SharedMem_Pointer();
        if (Mem->Common.HostProcess == GetCurrentProcessId())
        {
            Mem->Common.HostProcess = 0;
        }

        InterlockedDecrement(&Mem->Common.NumberOfDllsLoaded);

        Event.HookType = EVENT_DLL_UNLOAD;
        StringCchCopy(Event.Info.Buffer, _countof(Event.Info.Buffer), g_ProcessName.c_str());
        Event_Push(Event);

        SharedMem_Close();
        break;
    }
    return TRUE;
}

