#pragma once
#include "list"

class CFuncProxy
{

public:
	CFuncProxy();
	~CFuncProxy();
public:
	BOOL  Hook(DWORD lpAddresSrc,DWORD lpAddressDest,char* lpVerify = NULL,int nSize = 0,bool bForceHook = true);
	BOOL  UnHook();
	void* CallOldFunc();
protected:
	int FuncHeadMatchLen(char* lpFuncAddr,char* lpVerify,int nSize);
	BOOL RealHook(char* lpAddrSrc,char* lpAddrDest);
	unsigned int modrmbytes(unsigned char a, unsigned char b);
	char* cloneCode(char* lpAddrSrc,char* lpAddrDest);
protected:
	char* m_lpSrc;
	char* m_lpDest;
	char* m_lpCallSrc;
	CHAR  m_szHeadCode[64];
	int   m_nHeadCodeLen;
};

#include "proxyHook.hpp"