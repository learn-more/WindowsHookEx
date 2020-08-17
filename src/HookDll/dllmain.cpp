// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Shared.h"
#include "SharedMem.h"
#include "Event.h"
#include <string>
#include <strsafe.h>

std::wstring getModuleFilename(HMODULE Module)
{
    std::wstring Buffer;

    DWORD returned = 0;
    do
    {
        Buffer.resize(Buffer.size() + MAX_PATH);
        returned = GetModuleFileNameW(Module, Buffer.data(), Buffer.size());
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
    static std::wstring processName;

    SHARED_SETTINGS* Settings;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        SharedMem_Open(fIsFirst);
        Settings = &SharedMem_Pointer()->Settings;
        InterlockedIncrement(&Settings->NumberOfDllsLoaded);

        if (fIsFirst)
        {
            Settings->idHook = WH_CBT;
            Settings->HostProcess = GetCurrentProcessId();
        }

        processName = getModuleFilename(NULL);
        {
            std::wstring::size_type slash = processName.find_last_of(L"\\/");
            if (slash != std::wstring::npos)
                processName = processName.substr(slash + 1);
        }

        Event.HookType = EVENT_DLL_LOAD;
        StringCchCopy(Event.Info.Buffer, _countof(Event.Info.Buffer), processName.c_str());
        Event_Push(Event);

        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        Settings = &SharedMem_Pointer()->Settings;
        if (Settings->HostProcess == GetCurrentProcessId())
        {
            Settings->HostProcess = 0;
        }

        InterlockedDecrement(&Settings->NumberOfDllsLoaded);

        Event.HookType = EVENT_DLL_UNLOAD;
        StringCchCopy(Event.Info.Buffer, _countof(Event.Info.Buffer), processName.c_str());
        Event_Push(Event);

        SharedMem_Close();
        break;
    }
    return TRUE;
}

