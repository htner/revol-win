#pragma once

class CInjector
{
public:
	//static TCHAR* GetProcessPath(TCHAR* szBuf,int nLen);
	static void AdjustPrivilege(int pid, BOOL bEnable);
	static BOOL InjectProcessByPid(DWORD pid,LPCTSTR lpDll);
	static BOOL UnInjectProcessByPid(DWORD dwPID,LPCTSTR lpDll);
	static BOOL InjectProcessByHandle( HANDLE hProcess ,LPCTSTR lpDllName);
	static BOOL InjectProcessByExeName(LPCTSTR lpName,LPCTSTR lpDll);
	static DWORD GetProcessIDByExeName( LPCTSTR lpName);
	static BOOL IsExistProcess(DWORD pid);
	static BOOL IsProcessExistsDll( HANDLE hProcess,LPCTSTR lpDll );	
	static BOOL InjectProcessByPidEx(DWORD pid,LPCTSTR lpDll);
};

#include "injector.hpp"
