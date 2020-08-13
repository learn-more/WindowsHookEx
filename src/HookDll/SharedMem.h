#pragma once


void SharedMem_Open(BOOL& fIsFirst);
void SharedMem_Close(void);

struct SHARED_SETTINGS* SharedMem_Settings();


void SharedMem_Push(struct HOOK_EVENT& event);
bool SharedMem_Pop(struct HOOK_EVENT& event);

