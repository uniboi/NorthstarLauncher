#pragma once

// this magic only works under win32
// under linux this malloc() overrides the libc malloc() and so we
// end up in a recursion (as MemAlloc_Alloc() calls malloc)
#if _MSC_VER >= 1400

#if _MSC_VER >= 1900
#define _CRTNOALIAS
#endif

#define ALLOC_CALL _CRTNOALIAS _CRTRESTRICT
#define FREE_CALL _CRTNOALIAS
#else
#define ALLOC_CALL
#define FREE_CALL
#endif

class IMemAlloc
{
  public:
	/* struct VTable
	{
		void* unknown[1]; // alloc debug
		void* (*Alloc)(IMemAlloc* memAlloc, size_t nSize);
		void* unknown2[1]; // realloc debug
		void* (*Realloc)(IMemAlloc* memAlloc, void* pMem, size_t nSize);
		void* unknown3[1]; // free #1
		void (*Free)(IMemAlloc* memAlloc, void* pMem);
		void* unknown4[2]; // nullsubs, maybe CrtSetDbgFlag
		size_t (*GetSize)(IMemAlloc* memAlloc, void* pMem);
		void* unknown5[9]; // they all do literally nothing
		void (*DumpStats)(IMemAlloc* memAlloc);
		void (*DumpStatsFileBase)(IMemAlloc* memAlloc, const char* pchFileBase);
		void* unknown6[4];
		int (*heapchk)(IMemAlloc* memAlloc);
	};*/
	virtual void* InternalAlloc(size_t nSize, const char* pFileName, int nLine) = 0;
	virtual void* Alloc(size_t nSize) = 0;
	virtual void* InternalRealloc(void* pMem, size_t nSize, const char* pFileName, int nLine) = 0;
	virtual void* Realloc(void* pMem, size_t nSize) = 0;
	virtual void InternalFree(void* pMem, const char* pFileName, int nLine) = 0;
	virtual void Free(void* pMem) = 0;
	virtual void sub_unk6();
	virtual void sub_unk7();
	virtual size_t GetSize(void* pMem);
	virtual void sub_unk9();
	virtual void sub_unk10();
	virtual void sub_unk11();
	virtual void sub_unk12();
	virtual void sub_unk13();
	virtual void sub_unk14();
	virtual void sub_unk15();
	virtual void sub_unk16();
	virtual void sub_unk17();
	virtual void DumpStats();
	virtual void DumpStatsFileBase(const char* pchFileBase);
	virtual void sub_unk20();
	virtual void sub_unk21();
	virtual void sub_unk22();
	virtual void sub_unk23();
	virtual int heapchk();

	//VTable* m_vtable;
};

inline IMemAlloc* g_pMemAllocSingleton = nullptr;
inline IMemAlloc* (*CreateGlobalMemAlloc)() = nullptr;

//-----------------------------------------------------------------------------
//
IMemAlloc* MemAllocSingleton();
