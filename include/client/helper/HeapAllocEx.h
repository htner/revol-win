#pragma once

#ifndef HEAP_CREATE_ENABLE_EXECUTE
	#define HEAP_CREATE_ENABLE_EXECUTE 0x00040000
#endif

#ifndef CRASH_ME
#define CRASH_ME( con ) if( !(con) ){int *p = NULL; *p = NULL;}
#endif

template<bool LFH>
class CHeapAllocEx
{
public:
	CHeapAllocEx():_hHeapHandle(NULL),_bAutoDestroyHeap(TRUE)
	{		
		CreateHeap();		 
	}
	~CHeapAllocEx()
	{
		if( _bAutoDestroyHeap )
			DestroyHeap();
	}
public:
	inline operator HANDLE(){
		return _hHeapHandle;
	} 
	LPVOID AllocMemory( SIZE_T  nByteSize)
	{
		LPVOID pvAddr = HeapAlloc(_hHeapHandle,0,nByteSize);
		CRASH_ME( pvAddr != NULL );
		return pvAddr;
	}
	BOOL FreeMemory(LPVOID pvAddr)
	{
		BOOL bSuc = HeapFree(_hHeapHandle,0,pvAddr);
		ATLASSERT(bSuc);
		return bSuc;
	}
	inline void DestroyHeap()
	{
		if( _hHeapHandle )
		{
			HeapDestroy(_hHeapHandle);
			_hHeapHandle = NULL;
		}
	}
	inline void CreateHeap()
	{
		if( _hHeapHandle == NULL )
		{
			if(_hHeapHandle = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE,1024 * 4, 0))
			{
				if( LFH )
				{
					ULONG  lfh = 2;
					HeapSetInformation(_hHeapHandle,HeapCompatibilityInformation,&lfh,sizeof(lfh));\
				}
			}
		}
	}
	inline void EnableDestroyHeap(BOOL bEnable)
	{
		_bAutoDestroyHeap = bEnable;
	}

protected:
	HANDLE _hHeapHandle;
	BOOL   _bAutoDestroyHeap;
};


typedef CHeapAllocEx<true>	CHeapLFH;
typedef CHeapAllocEx<false>	CHeapNormal;