#pragma once

FORCEINLINE bool IsPowerOfTwo(unsigned int x)
{
	return (x & (x - 1)) == 0;
}
