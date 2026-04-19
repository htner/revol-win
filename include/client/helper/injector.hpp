#include "stdafx.h"
#include "psapi.h"
#include "tlhelp32.h"
#include "nsox/nulog.h"

#pragma comment(lib,"Psapi.lib")

#ifndef MYString
	#ifdef _AFX
		#define MYString CStringW
	#else
		#include <atlstr.h>
		#define MYString ATL::CStringW
	#endif
#endif
//
//TCHAR* GetProcessPath(TCHAR* szBuf,int nLen)
//{
//	GetModuleFileName(NULL,szBuf,nLen);
//
//	TCHAR* p = _tcsrchr( szBuf,'\\' );
//	if( p )
//	{
//		p[1] = 0;
//	}
//
//	return szBuf;
//}

inline void CInjector::AdjustPrivilege(int pid, BOOL bEnable)
{
	HANDLE hProcess;
	HANDLE hToken=0;
	TOKEN_PRIVILEGES tkp;
	tkp.PrivilegeCount = 1; 
	tkp.Privileges[0].Attributes = 0;
	if (bEnable)
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (LookupPrivilegeValue(NULL, _T("SeDebugPrivilege"), &tkp.Privileges[0].Luid))
	{
		if (hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid))
		{
			if (OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
			{
				if (AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
				{
					CloseHandle(hToken);
				}
			}
			CloseHandle(hProcess);
		}
	}
}

inline DWORD CInjector::GetProcessIDByExeName( LPCTSTR lpName)
{
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	MYString strtarger = lpName;
	strtarger.MakeLower();
	if ( EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
	{
		cProcesses = cbNeeded / sizeof(DWORD);
		for ( int i = 0; i < cProcesses; i++ )
		{
			if( aProcesses[i] != 0 )
			{
				TCHAR szProcessName[MAX_PATH] ={0};
				HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
					FALSE, aProcesses[i] );

				if (NULL != hProcess )
				{
					HMODULE hMod;
					DWORD cbNeeded;

					if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded) )
					{
						GetModuleBaseName( hProcess, hMod, szProcessName, _countof(szProcessName) );						
						MYString str = szProcessName;
						str.MakeLower();
						if( str.Find( strtarger ) >=0 )
						{
							CloseHandle( hProcess );
							return aProcesses[i];
						}
					}
					CloseHandle( hProcess );
				}
			}
		}
	}
	return 0;
}

inline BOOL CInjector::InjectProcessByExeName(LPCTSTR lpName,LPCTSTR lpDll)
{
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	BOOL bSuc = FALSE;
	if ( EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
	{
		cProcesses = cbNeeded / sizeof(DWORD);
		for ( int i = 0; i < cProcesses; i++ )
		{
			if( aProcesses[i] != 0 )
			{

				TCHAR szProcessName[MAX_PATH] ={0};

				HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
												FALSE, aProcesses[i] );

				if (NULL != hProcess )
				{
					HMODULE hMod;
					DWORD cbNeeded;

					if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded) )
					{
						GetModuleBaseName( hProcess, hMod, szProcessName, _countof(szProcessName) );
						if( _tcsicmp( lpName,szProcessName ) == 0 )
						{
							bSuc = InjectProcessByHandle( hProcess, lpDll );
						}
					}
				}else
				{
					USES_CONVERSION;
					nsox::xlog(NsoxError,"overlay->InjectProcessByExeName -OpenProcess: %s Failed!",CT2CA(lpName));
				}
			}
		}
	}
	return bSuc;
}

inline BOOL CInjector::InjectProcessByPid(DWORD pid,LPCTSTR lpDll)
{
	BOOL bSuc = FALSE;
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	
	AdjustPrivilege( GetCurrentProcessId(), TRUE);

	if ( EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
	{
		cProcesses = cbNeeded / sizeof(DWORD);
		for ( int i = 0; i < cProcesses; i++ )
		{
			if( aProcesses[i] == pid )
			{

				HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
					FALSE, aProcesses[i] );
				if( hProcess )
				{
					bSuc = InjectProcessByHandle(hProcess,lpDll);
				}
			}
		}
	}
	return bSuc;
}

inline BOOL CInjector::UnInjectProcessByPid(DWORD dwPID,LPCTSTR lpDll)
{
	//	BOOL __stdcall UnInjectLibToProcess(LPTSTR lpszModule,DWORD dwProcessID)
	if(lpDll==NULL ||	dwPID == 0)
		return false;

	DWORD dwProcessID = dwPID;
	USES_CONVERSION;
	std::string strDLLName = T2A(lpDll);
	
	// 打开目标进程   
	HANDLE hProcess = OpenProcess( PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, dwProcessID );   
	// 向目标进程地址空间写入DLL名称   

	DWORD dwSize, dwWritten;   
	dwSize = lstrlenA( strDLLName.c_str() ) + 1;   
	LPVOID lpBuf = VirtualAllocEx( hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE );   
	if ( NULL == lpBuf )   
	{   
		CloseHandle( hProcess );   
		return FALSE;   
	}   
	if ( WriteProcessMemory( hProcess, lpBuf, (LPVOID)strDLLName.data(), dwSize, &dwWritten ) )   
	{   
		// 要写入字节数与实际写入字节数不相等，仍属失败   
		if ( dwWritten != dwSize )   
		{   
			VirtualFreeEx( hProcess, lpBuf, dwSize, MEM_DECOMMIT );   
			CloseHandle( hProcess );   
			return FALSE;   
		}   
	}   
	else   
	{   
		CloseHandle( hProcess );   
		return FALSE;   
	}   

	// 使目标进程调用GetModuleHandle，获得DLL在目标进程中的句柄   
	DWORD dwHandle, dwID;   
	PTHREAD_START_ROUTINE pFunc = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32"),"GetModuleHandleA");   
	HANDLE hThread = CreateRemoteThread( hProcess, NULL, 0, pFunc, lpBuf, 0, &dwID );   

	// 等待GetModuleHandle运行完毕   
	WaitForSingleObject( hThread, INFINITE );   
	// 获得GetModuleHandle的返回值   
	GetExitCodeThread( hThread, &dwHandle );   
	// 释放目标进程中申请的空间   
	VirtualFreeEx( hProcess, lpBuf, dwSize, MEM_DECOMMIT );   
	CloseHandle( hThread );   
	// 使目标进程调用FreeLibrary，卸载DLL   

	pFunc=(PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32"),"FreeLibrary");   

	hThread = CreateRemoteThread( hProcess, NULL, 0, pFunc, (LPVOID)dwHandle, 0, &dwID );   
	// 等待FreeLibrary卸载完毕   
	WaitForSingleObject( hThread, INFINITE );   
	CloseHandle( hThread );   
	CloseHandle( hProcess );   

	return true;   
}

inline BOOL CInjector::IsProcessExistsDll( HANDLE hProcess,LPCTSTR lpDll )
{
	DWORD dwCount = 0;
	HMODULE hMods[1024];
	EnumProcessModules( hProcess,hMods,sizeof(hMods),&dwCount );
	for ( int i = 0; i < dwCount; i++ )
	{
		TCHAR szModName[MAX_PATH];

		if ( GetModuleFileNameEx(hProcess, hMods[i], szModName,
			sizeof(szModName)/sizeof(TCHAR)))
		{
			if( _tcsicmp( szModName,lpDll ) == 0 )
				return TRUE;
		}
	}

	return FALSE;
}

inline BOOL CInjector::InjectProcessByHandle( HANDLE hProcess ,LPCTSTR lpDllName)
{
	HANDLE hThread;
	void*   pLibRemote;   // 远程进程中的地址，szLibPath 将被拷贝到此处;
	DWORD   hLibModule;   // 要加载的模块的基地址（HMODULE）
	WCHAR szLibPath[ MAX_PATH ] = {0};

	if( IsProcessExistsDll( hProcess, lpDllName ) )
		return TRUE;

	HMODULE hKernel32 = ::GetModuleHandle(_T("Kernel32"));
	USES_CONVERSION;
	wcscpy(szLibPath,CT2CW(lpDllName));

	BOOL bSuc = FALSE;
	pLibRemote = ::VirtualAllocEx( hProcess, NULL, sizeof(szLibPath),
		MEM_COMMIT, PAGE_READWRITE );
	if( pLibRemote )
	{
		BOOL bRes = ::WriteProcessMemory( hProcess, pLibRemote, (void*)szLibPath,
										sizeof(szLibPath), NULL );
		if( bRes )
		{
			WCHAR szBuf[ MAX_PATH ] = {0};
			::ReadProcessMemory(hProcess,pLibRemote,szBuf,sizeof(szBuf),NULL);
			if( wcscmp(szBuf,szLibPath) == 0 )
			{
				LPTHREAD_START_ROUTINE fn = (LPTHREAD_START_ROUTINE) ::GetProcAddress( hKernel32,"LoadLibraryW" );
				hThread = ::CreateRemoteThread( hProcess, NULL, 0,
					fn,pLibRemote, 0, NULL );
				if( hThread )
				{
					::WaitForSingleObject( hThread, 2000/*INFINITE*/ );

					// 获取所加载的模块的句柄
					::GetExitCodeThread( hThread, &hLibModule );

					// 清除
					::CloseHandle( hThread );
					bSuc = TRUE;
				}
			}
			::VirtualFreeEx( hProcess, pLibRemote, sizeof(szLibPath), MEM_RELEASE );
		}
	}
	return bSuc;
}
inline BOOL CInjector::IsExistProcess(DWORD pid)
{
	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION,FALSE, pid);
	if( hProcess )
	{
		CloseHandle( hProcess );
		return TRUE;
	}else
	{
		if( GetLastError() == ERROR_ACCESS_DENIED )
		{
			return TRUE;
		}
		return FALSE;
	}
}

class xInjectorHelper
{
public:
	static void ModifyThread(BOOL bSuppend,DWORD* lpIDList,int nCount)
	{
		for ( int i = 0; i < nCount; i++)
		{
			HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, 0,lpIDList[i]); 
			if( bSuppend )
			{
				SuspendThread(hThread);
			}else
			{
				ResumeThread(hThread);
			}
			CloseHandle(hThread);
		}
	}

	static int GetThreadList(DWORD pid,DWORD* lpList,int nCount)
	{
		int nfount = 0;
		try
		{
			HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,pid);
			if( hSnapshot )
			{
				THREADENTRY32 entry;
				entry.dwSize = sizeof(THREADENTRY32 );
				if(Thread32First(hSnapshot,&entry))
				{
					do 
					{ 
						if( entry.th32OwnerProcessID == pid &&
							entry.th32ThreadID)
						{
							lpList[nfount++] = entry.th32ThreadID;
							ATLTRACE(_T("%d\r\n"),entry.th32ThreadID);
						}
					} while( nCount>nfount && Thread32Next(hSnapshot, &entry ) ); 
				}
				CloseHandle( hSnapshot );
			}
		}catch(...){};
		return nfount;
	}


	static void RestoreLoadLibrary(DWORD pid,LPCTSTR lpModuleName,LPCSTR lpFunction,int nSizeVerify)
	{
		if( pid )
		{
			HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
				FALSE,pid);
			if( h )
			{
				CHAR* lpSefl = new CHAR[nSizeVerify];
				CHAR* lpTarger = new CHAR[nSizeVerify];


				HMODULE hModule = GetModuleHandle(lpModuleName);
				LPVOID dwVerify = (LPVOID)GetProcAddress(hModule,lpFunction);

				memcpy(lpSefl,dwVerify,nSizeVerify);

				SIZE_T nRead = 0;
				BOOL bSuc = ReadProcessMemory(h,dwVerify,lpTarger,nSizeVerify,&nRead);
				if( bSuc )
				{
					int nRes = memcmp(lpSefl,lpTarger,nSizeVerify);
					if( nRes != 0 && lpSefl[0] != (CHAR)0xE9 )
					{
						WriteProcessMemory(h,dwVerify,lpSefl,nSizeVerify,NULL);
					}
				}
				CloseHandle( h );

				delete[] lpSefl;
				delete[] lpTarger;
			}
		}
	}

};


inline BOOL CInjector::InjectProcessByPidEx(DWORD pid,LPCTSTR lpDll)
{
	BOOL bSuc = FALSE;
	try
	{	
		DWORD lpThreadID[512] = {0};
		int nCount = xInjectorHelper::GetThreadList(pid,lpThreadID,_countof(lpThreadID));

		xInjectorHelper::ModifyThread(TRUE,lpThreadID,nCount);

		xInjectorHelper::RestoreLoadLibrary(pid,_T("kernel32"),"LoadLibraryW",5);
		xInjectorHelper::RestoreLoadLibrary(pid,_T("kernel32"),"LoadLibraryExW",5);

		bSuc = InjectProcessByPid(pid,lpDll);
		
		xInjectorHelper::ModifyThread(FALSE,lpThreadID,nCount);
	}
	catch (...){}
	return bSuc;

}

//
//BOOL  GetProcessPathFromPid(DWORD pid,TCHAR* szPath,int nCount)
//{
//	TCHAR szProcessName[MAX_PATH] ={0};
//	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE, pid );
//
//	if (NULL != hProcess )
//	{
//		HMODULE hMod;
//		DWORD cbNeeded;
//
//		if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded) )
//		{
//			GetModuleFileNameEx(hProcess,hMod,szPath,nCount);
//			CloseHandle( hProcess );
//			return TRUE;
//		}
//		CloseHandle( hProcess );
//	}
//	return FALSE;
//}
