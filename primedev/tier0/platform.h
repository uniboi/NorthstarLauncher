#pragma once

//-----------------------------------------------------------------------------
// Time stamp counter
//-----------------------------------------------------------------------------
inline uint64_t Plat_Rdtsc()
{
#if defined(_X360)
	return (uint64)__mftb32();
#elif defined(_WIN64)
	return (uint64_t)__rdtsc();
#elif defined(_WIN32)
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	return (uint64_t)__rdtsc();
#else
	__asm rdtsc;
	__asm ret;
#endif
#elif defined(__i386__)
	uint64 val;
	__asm__ __volatile__("rdtsc" : "=A"(val));
	return val;
#elif defined(__x86_64__)
	uint32 lo, hi;
	__asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
	return (((uint64)hi) << 32) | lo;
#else
#error
#endif
}

inline double (*Plat_FloatTime)();
