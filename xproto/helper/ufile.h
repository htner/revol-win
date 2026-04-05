#pragma	  once

#include <Urlmon.h>
#include <wininet.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <fstream>
#include <mlang.h>
#include <proto/iurls.h>

typedef HRESULT (WINAPI *GetKnownFolderPathFunc)(const GUID& rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath); // {A520A1A4-1780-4FF6-BD18-167343C5AF16}
//static const GUID FOLDERID_LocalAppDataLow = {0xA520A1A4, 0x1780, 0x4FF6, {0xBD, 0x18, 0x16, 0x73, 0x43, 0xC5, 0xAF, 0x16}};
typedef HRESULT (WINAPI *SHGetFolderPathFn)(
	HWND hwndOwner,
	int nFolder,
	HANDLE hToken,
	DWORD dwFlags,
	LPTSTR pszPath
	);


inline xstring getSysAppDataLowPath()
{
	//const GUID FOLDERID_LocalAppDataLow = {0xA520A1A4, 0x1780, 0x4FF6, {0xBD, 0x18, 0x16, 0x73, 0x43, 0xC5, 0xAF, 0x16}};
	xstring val;
	HMODULE hShell32DLL = LoadLibrary(_T("shell32.dll"));
	if(hShell32DLL) {
		GetKnownFolderPathFunc fnSHGetFolderPath = 
			(GetKnownFolderPathFunc)GetProcAddress(hShell32DLL, "SHGetKnownFolderPath");

		if(fnSHGetFolderPath) {
			PWSTR path;
			if (SUCCEEDED( fnSHGetFolderPath(FOLDERID_LocalAppDataLow, 0, NULL, &path))) {
				val = path;
				CoTaskMemFree(path);
			}
		}
		FreeLibrary(hShell32DLL);
	}
	return val;
}



inline xstring getSysAppDataPath()
{
	TCHAR szPath[MAX_PATH];
	szPath[0] = 0;
	bool bOK = false;
	HMODULE hShell32DLL = LoadLibrary(_T("shell32.dll"));
	if(hShell32DLL) {
		SHGetFolderPathFn fnSHGetFolderPath = 
			(SHGetFolderPathFn)GetProcAddress(hShell32DLL, "SHGetFolderPathW");
		if(fnSHGetFolderPath) {
			// Get path for each computer, non-user specific and non-roaming data.
			//if (SUCCEEDED( fnSHGetFolderPath( NULL, CSIDL_COMMON_APPDATA, 
			if (SUCCEEDED( fnSHGetFolderPath( NULL, CSIDL_APPDATA, 
				NULL, 0, szPath ) ) ) {
					bOK = true;
				}
		}
		FreeLibrary(hShell32DLL);
	}
	if(!bOK) {
		if(GetWindowsDirectory(szPath, MAX_PATH)) {
			//lstrcat(szPath, "\\All Users\\Application Data");
			lstrcat(szPath, _T("\\Application Data"));
		}
	}
	return szPath;
}




inline xstring getAppPath()
{
	TCHAR tmp[1024];
	tmp[0] = 0;
	if(GetModuleFileName(NULL, tmp, sizeof(tmp)) > 0) {
		TCHAR* dst = NULL;
		TCHAR* p = tmp;
		while (*p) {
			if (*p == _T('\\'))
				dst = p;
			p = CharNext(p);
		}
		if(dst) *dst = 0;
	}
	return tmp;
}


inline xstring getRexConfPath()
{
	xstring appPath = getSysAppDataLowPath();

	if(appPath.empty())
	{
		appPath = getSysAppDataPath();
		if(appPath.empty())
		{
			appPath = getAppPath();
		}
	}
	appPath += _T("\\");
	appPath += REX_CONFIG_FOLDER;
	appPath += _T("\\");

	CreateDirectory(appPath.c_str(), NULL);
	return appPath;
}



#ifdef WIN32
/*
CString GetFileDirFromPath(LPCTSTR filepath)
{
		unsigned int orgLen = _tcslen(filepath);

		TCHAR end = filepath[ orgLen - 1];

		if(end !=_T('\\') && end != _T(':'))
		{
				LPCTSTR FN = PathFindFileName(filepath);

				return CString(filepath, orgLen -  _tcslen(FN));
		} 
		else
		{
				return CString(filepath);
		}
}


CString GetCurrentPathDir()
{
		TCHAR	buffer[MAX_PATH];
		ZeroMemory(buffer, sizeof(TCHAR) * MAX_PATH);
		::GetModuleFileName(NULL, buffer, MAX_PATH);

		return GetFileDirFromPath(buffer);
}
*/
inline bool isFileExist(LPCTSTR file)
{
		WIN32_FIND_DATA data;
		HANDLE handle;
		if(!file) return false;
		handle = FindFirstFile(file, &data);
		if(handle == INVALID_HANDLE_VALUE)
				return false;
		FindClose(handle);
		return true;
}


static inline  xstring GetAppPath()
{
	TCHAR tmp[1024];
	tmp[0] = 0;
	if(GetModuleFileName(NULL, tmp, sizeof(tmp)) > 0) {
		TCHAR* dst = NULL;
		TCHAR* p = tmp;
		while (*p) {
			if (*p == _T('\\'))
				dst = p;
			p = CharNext(p);
		}
		if(dst) *dst = 0;
	}
	return tmp;
}

inline std::wstring GetFileDirFromPath(LPCTSTR filepath)
{
	unsigned int orgLen = _tcslen(filepath);

	TCHAR end = filepath[ orgLen - 1];

	if(end !=_T('\\') && end != _T(':'))
	{
		LPCTSTR FN = PathFindFileName(filepath);

		return std::wstring(filepath, orgLen -  _tcslen(FN));
	} 
	else
	{
		return std::wstring(filepath);
	}
}

inline std::wstring GetCurrentPathDir()
{
	TCHAR	buffer[MAX_PATH];
	ZeroMemory(buffer, sizeof(TCHAR) * MAX_PATH);
	::GetModuleFileName(NULL, buffer, MAX_PATH);

	return GetFileDirFromPath(buffer);
}
#endif