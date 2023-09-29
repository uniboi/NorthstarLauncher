#pragma once

#include <assert.h>

// [Fifty]: assert macro already has a NDEBUG check, keeping this if we ever do our own asserts
#ifdef _DEBUG
#define Assert(con, ...) assert(con)
#else
#define Assert(con, ...)
#endif
