#pragma once

inline const char* (*GetCurrentPlaylistName)();

bool SetCurrentPlaylist(const char* pPlaylistName);
void SetPlaylistVarOverride(const char* pVarName, const char* pValue);
const char* GetCurrentPlaylistVar(const char* pVarName, bool bUseOverrides);
