#pragma once



template<class T>
class CUtlMemory
{
  private:
	T* m_pMemory;
	int64_t m_nAllocationCount;
	int64_t m_nGrowSize;
};
