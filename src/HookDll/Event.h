#pragma once

void Event_Push(struct HOOK_EVENT& event);
void Event_Pop(struct HOOK_EVENT* event, size_t& NumEvents);
