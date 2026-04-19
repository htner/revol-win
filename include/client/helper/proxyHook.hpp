

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline CFuncProxy::CFuncProxy()
{
	m_lpSrc = NULL;
	m_lpDest = NULL;
	m_lpCallSrc = NULL;
	m_nHeadCodeLen = 0;
}
inline CFuncProxy::~CFuncProxy()
{

}

inline BOOL CFuncProxy::Hook(DWORD lpAddresSrc,DWORD lpAddressDest,char* lpVerify,int nSize,bool bForceHook)
{	
	ATLASSERT( m_lpDest == NULL );
	ATLASSERT( m_lpCallSrc == NULL );
	
	if( IsBadReadPtr((LPCVOID)lpAddresSrc,nSize) || IsBadCodePtr((FARPROC)&lpAddresSrc) )
	{
		return FALSE;
	}

	if( lpVerify && nSize )
	{
		int nOffset = FuncHeadMatchLen((char*)lpAddresSrc,lpVerify,nSize);
		if( nOffset!= nSize && !bForceHook )
		{
			return FALSE;
		}
		if( nOffset != nSize && bForceHook && nOffset < 5)
		{
			DWORD dwOldProected = 0;
			if(VirtualProtect((void*)lpAddresSrc ,nSize,PAGE_EXECUTE_READWRITE,&dwOldProected))
			{
				memcpy((void*)lpAddresSrc,lpVerify,nSize);
				VirtualProtect((void*)lpAddresSrc ,nSize,dwOldProected,NULL);			
			}
		}
	}
	return RealHook((char*)lpAddresSrc,(char*)lpAddressDest);
}

inline BOOL CFuncProxy::UnHook()
{
	if( m_lpSrc )
	{
		DWORD dwOldProected = 0;
		if(VirtualProtect((LPVOID)m_lpSrc,m_nHeadCodeLen,PAGE_EXECUTE_READWRITE,&dwOldProected))
		{
			memcpy(m_lpSrc,m_szHeadCode,m_nHeadCodeLen);			
			VirtualProtect((LPVOID)m_lpSrc,m_nHeadCodeLen,dwOldProected,NULL);
			FlushInstructionCache(GetCurrentProcess(),m_lpSrc,m_nHeadCodeLen);
		}else
			return FALSE;
		

		//////////////////don't empty m_lpSrc ptr,CallOldFunc will use it//////////
		// m_lpSrc = NULL;
		//////////////////////////////////////////////////////////////////////////
	}
	if( m_lpCallSrc )
	{
		BOOL bSuc = VirtualFree(m_lpCallSrc,0,MEM_RELEASE);
		ATLASSERT( bSuc );		
		m_lpCallSrc = NULL;
	}
	m_lpDest = NULL;

	return TRUE;
}

inline void* CFuncProxy::CallOldFunc()
{
	if( m_lpCallSrc == NULL )
	{
		return m_lpSrc;
	}
	return m_lpCallSrc;
}

inline int CFuncProxy::FuncHeadMatchLen(char* lpFuncAddr,char* lpVerify,int nSize)
{
	
	if( lpVerify == NULL || nSize <=0 )
		return nSize;

	if( lpFuncAddr == NULL )
		return 0;

	int nIndex = -1;
	DWORD dwOldProected = 0;

	if(VirtualProtect((LPVOID)lpFuncAddr,nSize,PAGE_EXECUTE_READWRITE,&dwOldProected))
	{
		for (int i=0; i < nSize;i++)
		{
			if( lpFuncAddr[i] != lpVerify[i] )
			{
				nIndex = i;
				break;
			}
		}

		VirtualProtect((LPVOID)lpFuncAddr,nSize,dwOldProected,NULL);
	}else
		return 0;

	if( nIndex < 0 )
		return nSize;

	return nIndex;
}

#define TRUNK_SIZE 256

inline BOOL CFuncProxy::RealHook(char* lpAddrSrc,char* lpAddrDest)
{

	DWORD dwOldProected = 0;
	if( !VirtualProtect(lpAddrSrc ,TRUNK_SIZE,PAGE_EXECUTE_READWRITE,&dwOldProected))
		return FALSE;
	

	m_lpCallSrc = cloneCode(lpAddrSrc,lpAddrDest);

	VirtualProtect(lpAddrSrc ,TRUNK_SIZE,dwOldProected,NULL);

	ATLTRACE(_T("src: 0x%0x,dest: 0x%0x,inline: 0x%0x\r\n"),m_lpSrc,m_lpDest,m_lpCallSrc);
	return (m_lpCallSrc != NULL);
}


inline unsigned int CFuncProxy::modrmbytes(unsigned char a, unsigned char b) 
{
	unsigned char lower = (a & 0x0f);
	if (a >= 0xc0) {
		return 0;
	} else if (a >= 0x80) {
		if ((lower == 4) || (lower == 12))
			return 5;
		else
			return 4;
	} else if (a >= 0x40) {
		if ((lower == 4) || (lower == 12))
			return 2;
		else
			return 1;

	} else {
		if ((lower == 4) || (lower == 12)) {
			if ((b & 0x07) == 0x05)
				return 5;
			else
				return 1;
		} else if ((lower == 5) || (lower == 13))
			return 4;
		return 0;
	}
}

inline char* CFuncProxy::cloneCode(char* lpAddrSrc,char* lpAddrDest) 
{

	char* porig = lpAddrSrc;
	char *o = porig;
	char JMP = 0xE9;
	while (*o == JMP) {
		int *iptr = reinterpret_cast<int *>(o+1);
		o += *iptr + 5;		
		porig = o;
	}

	DWORD dwOldProected = 0;
	if( !VirtualProtect(porig ,TRUNK_SIZE,PAGE_EXECUTE_READWRITE,&dwOldProected))
		return NULL;

	unsigned int idx = 0;
	char* pCode = (char*)VirtualAlloc(NULL, TRUNK_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	char *n = pCode;

	do {
		unsigned char opcode = o[idx];
		unsigned char a = o[idx+1];
		unsigned char b = o[idx+2];
		unsigned int extra = 0;

		n[idx] = opcode;
		idx++;

		switch (opcode) {
			case 0x50: // PUSH
			case 0x51:
			case 0x52:
			case 0x53:
			case 0x54:
			case 0x55:
			case 0x56:
			case 0x57:
			case 0x58: // POP
			case 0x59:
			case 0x5a:
			case 0x5b:
			case 0x5c:
			case 0x5d:
			case 0x5e:
			case 0x5f:
				break;
			case 0x6A: //PUSH Bv
				extra = 1;
				break;
			case 0x68: //PUSH Iv
			case 0xA1: // MOV EAX Iv
				extra = 4;
				break;
			case 0x81: // CMP immediate
				extra = modrmbytes(a,b) + 5;
				break;
			case 0x83:	// CMP
				extra = modrmbytes(a,b) + 2;
				break;
			case 0x8b:	// MOV
				extra = modrmbytes(a,b) + 1;
				break;
			case 0xb8: // MOV
				extra = 4;
				break;
			default:				
				return NULL;
		}
		for (unsigned int i=0;i<extra;++i)
			n[idx+i] = o[idx+i];
		idx += extra;

	} while (idx < 5);

	if( idx >= sizeof(m_szHeadCode) )
	{
		VirtualFree(pCode,0,MEM_RELEASE);
		return NULL;
	}

	m_nHeadCodeLen = idx;

	m_lpSrc = porig;
	m_lpDest = lpAddrDest;

	memcpy(m_szHeadCode,m_lpSrc,sizeof(m_szHeadCode));

	n[idx] = 0xE9;		
	*(DWORD*)(n + idx +1 ) = (DWORD)((o+idx) - (n+idx) - 5);
	
	*porig = 0xE9;	
	*(DWORD*)(porig+1) = (DWORD)(lpAddrDest - porig - 5);

	for (int i=5;i<idx;i++){
		porig[i] = 0x90;
	}

	
	VirtualProtect(porig ,TRUNK_SIZE,dwOldProected,NULL);
	

	FlushInstructionCache(GetCurrentProcess(), n, idx + 5);
	FlushInstructionCache(GetCurrentProcess(), porig, idx);
	
	return pCode;
}