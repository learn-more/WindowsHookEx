// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Shared.h"
#include "SharedMem.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    BOOL fIsFirst = FALSE;
    SHARED_SETTINGS* Settings;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        SharedMem_Open(fIsFirst);
        Settings = SharedMem_Settings();
        InterlockedIncrement(&Settings->NumberOfDllsLoaded);

        if (fIsFirst)
        {
            Settings->idHook = WH_KEYBOARD;
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        Settings = SharedMem_Settings();
        InterlockedDecrement(&Settings->NumberOfDllsLoaded);

        SharedMem_Close();
        break;
    }
    return TRUE;
}

