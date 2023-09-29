#pragma once

//-----------------------------------------------------------------------------
// Purpose: Standard functions for handling endian-ness
//-----------------------------------------------------------------------------

#define LittleShort(val) (val)
#define LittleWord(val) (val)
#define LittleLong(val) (val)
#define LittleDWord(val) (val)
#define LittleQWord(val) (val)

// If a swapped float passes through the fpu, the bytes may get changed.
// Prevent this by swapping floats as DWORDs.
#define SafeSwapFloat(pOut, pIn) (*((uint*)pOut) = DWordSwap(*((uint*)pIn)))

#define BigFloat(pOut, pIn) SafeSwapFloat(pOut, pIn)
#define LittleFloat(pOut, pIn) (*pOut = *pIn)
#define SwapFloat(pOut, pIn) BigFloat(pOut, pIn)

//-------------------------------------
// Basic swaps
//-------------------------------------

template <typename T>
inline T WordSwapC(T w)
{
	uint16 temp;

	static_assert(sizeof(T) == sizeof(uint16));

	temp = ((*((uint16*)&w) & 0xff00) >> 8);
	temp |= ((*((uint16*)&w) & 0x00ff) << 8);

	return *((T*)&temp);
}

template <typename T>
inline T DWordSwapC(T dw)
{
	uint32 temp;

	static_assert(sizeof(T) == sizeof(uint32));

	temp = *((uint32*)&dw) >> 24;
	temp |= ((*((uint32*)&dw) & 0x00FF0000) >> 8);
	temp |= ((*((uint32*)&dw) & 0x0000FF00) << 8);
	temp |= ((*((uint32*)&dw) & 0x000000FF) << 24);

	return *((T*)&temp);
}

template <typename T>
inline T QWordSwapC(T dw)
{
	// Assert sizes passed to this are already correct, otherwise
	// the cast to uint64 * below is unsafe and may have wrong results
	// or even crash.
	static_assert(sizeof(dw) == sizeof(uint64));

	uint64 temp;

	temp = *((uint64*)&dw) >> 56;
	temp |= ((*((uint64*)&dw) & 0x00FF000000000000ull) >> 40);
	temp |= ((*((uint64*)&dw) & 0x0000FF0000000000ull) >> 24);
	temp |= ((*((uint64*)&dw) & 0x000000FF00000000ull) >> 8);
	temp |= ((*((uint64*)&dw) & 0x00000000FF000000ull) << 8);
	temp |= ((*((uint64*)&dw) & 0x0000000000FF0000ull) << 24);
	temp |= ((*((uint64*)&dw) & 0x000000000000FF00ull) << 40);
	temp |= ((*((uint64*)&dw) & 0x00000000000000FFull) << 56);

	return *((T*)&temp);
}

inline uint32 LoadLittleDWord(uint32* base, unsigned int dwordIndex)
{
	return LittleDWord(base[dwordIndex]);
}

inline void StoreLittleDWord(uint32* base, unsigned int dwordIndex, uint32 dword)
{
	base[dwordIndex] = LittleDWord(dword);
}
