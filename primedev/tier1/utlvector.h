#pragma once

#include "tier1/utlmemory.h"

template <class T, class A = CUtlMemory<T>>
class CUtlVector
{
	typedef A CAllocator;
  public:
	CAllocator m_Memory;
	int m_Size;
};


template <class T, size_t MAX_SIZE>
class CUtlVectorFixed : public CUtlVector<T, CUtlMemoryFixed<T, MAX_SIZE>>
{
};


template <class T, size_t MAX_SIZE>
class CCopyableUtlVectorFixed : public CUtlVectorFixed<T, MAX_SIZE>
{
	typedef CUtlVectorFixed<T, MAX_SIZE> BaseClass;

  public:
	virtual ~CCopyableUtlVectorFixed() {}
};
