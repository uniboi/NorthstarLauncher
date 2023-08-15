#pragma once

// TODO [Fifty]: Might be worth getting the handles themselves instead of relying on exports

inline bool (*ThreadCouldDoServerWork)();
inline bool (*ThreadInMainOrServerFrameThread)();
inline bool (*ThreadInMainThread)();
inline bool (*ThreadInServerFrameThread)();
