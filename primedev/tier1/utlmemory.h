#pragma once

template <class T>
class CUtlMemory
{
  public:
	T* m_pMemory;
	int64_t m_nAllocationCount;
	int64_t m_nGrowSize;
};

template <typename T, size_t SIZE, int nAlignment = 0>
class CUtlMemoryFixed
{
  public:
	char m_Memory[SIZE * sizeof(T) + nAlignment];
};
