#pragma once

#include "game/server/recipientfilter.h"

inline void(__fastcall* UserMessageBegin)(CRecipientFilter* filter, const char* messagename);
inline void(__fastcall* MessageEnd)();
inline void(__fastcall* MessageWriteByte)(int iValue);
inline void(__fastcall* MessageWriteString)(const char* sz);
inline void(__fastcall* MessageWriteBool)(bool bValue);
