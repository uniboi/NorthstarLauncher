#pragma once

// note: isIngameChat is an int64 because the whole register the arg is stored in needs to be 0'd out to work
// if isIngameChat is false, we use network chat instead
inline void (*CEngineClient__Saytext)(void* thisptr, const char* szMessage, uint64_t bIsIngameChat, bool bIsTeamChat);
