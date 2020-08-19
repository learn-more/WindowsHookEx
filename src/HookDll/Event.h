//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2020 Mark Jansen
// SPDX-License-Identifier: MIT
//

#pragma once

bool Event_Ignored(HWND hWnd);
void Event_Push(struct HOOK_EVENT& event);
void Event_Pop(struct HOOK_EVENT* event, size_t& NumEvents);
