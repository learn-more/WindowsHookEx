#include "pch.h"
#include "Shared.h"

static SHARED_SETTINGS g_Settings;

HOOKDLL_EXPORT
SHARED_SETTINGS*
HOOKDLL_DECLSPEC
HookDll_GetSettings(void)
{
    return &g_Settings;
}


HOOKDLL_EXPORT
void HOOKDLL_DECLSPEC HookDll_InstallHook(void)
{

}
