#pragma	  once

#include <Urlmon.h>
#include <wininet.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <fstream>
#include <mlang.h>
#include <ShellAPI.h>
#ifndef xstring
#include <string>
#define xstring  std::wstring
#endif
#include <algorithm>

#include "location/default.h"


#ifdef _VS_2008
#define SELECT_ANY	__declspec (selectany)
#else
#define SELECT_ANY	
#endif

#ifndef _countof
	#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

typedef HRESULT (WINAPI *GetKnownFolderPathFunc)(const GUID& rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath); // {A520A1A4-1780-4FF6-BD18-167343C5AF16}
#ifdef VS_2003
//static const GUID SELECT_ANY FOLDERID_LocalAppDataLow = {0xA520A1A4, 0x1780, 0x4FF6, {0xBD, 0x18, 0x16, 0x73, 0x43, 0xC5, 0xAF, 0x16}};
#endif
typedef HRESULT (WINAPI *SHGetFolderPathFn)(
    HWND hwndOwner,
    int nFolder,
    HANDLE hToken,
    DWORD dwFlags,
    LPTSTR pszPath
    );

namespace helper
{

    class ushell
    {

    public:
        static xstring getSysAppDataLowPath()
        {
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



        static xstring getSysAppDataPath()
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




        static xstring getAppPath()
        {
			static xstring strAppPath;
			if (strAppPath.empty())
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
				strAppPath = tmp;
			}
           
            return strAppPath;
        }

		static xstring getFullPath(LPCTSTR lpszFileRelaty)
		{
			xstring strTemp;
			if (lpszFileRelaty)
			{
				if (lstrlen(lpszFileRelaty) > 1 && lpszFileRelaty[1] == ':')
					return lpszFileRelaty;

				strTemp = getAppPath();
				strTemp += _T("\\");
				strTemp += lpszFileRelaty;
			}
			return strTemp;
		}
		
		static BOOL IsUnlegalPath(LPCTSTR lpszPath)
		{
			return ( PathFileExists( lpszPath ) && 
				   !PathIsDirectory( lpszPath ) );
		}

        static const xstring& getRexConfPath()
        {			
			static xstring g_appConfPath;
			if( g_appConfPath.empty() )
			{
				g_appConfPath = getSysAppDataLowPath();

				if(g_appConfPath.empty())
				{
					g_appConfPath = getSysAppDataPath();
					if(g_appConfPath.empty())
					{
						g_appConfPath = getAppPath();
					}
				}
				g_appConfPath += _T("\\");
				g_appConfPath += REX_CONFIG_FOLDER;
				//�����Ƿ��ļ��tҪ��delete����Ȼ���������ɹ�
				if( IsUnlegalPath(g_appConfPath.c_str()) ) {				
					DeleteFile( g_appConfPath.c_str() );
				}
				if(g_appConfPath.at(g_appConfPath.size()-1) != '\\')
					g_appConfPath += _T("\\");
			}

            CreateDirectory(g_appConfPath.c_str(), NULL);
            return g_appConfPath;
        }
		static xstring resetRexConfPath(const xstring& user )
		{
			xstring newpath;
			xstring usertmp = user;
			xstring path = getRexConfPath();

			xstring strCurUser = helper::ushell::GetCurProcessUserName();
			
			std::transform(path.begin(), path.end(), path.begin(), tolower);
			std::transform(usertmp.begin(), usertmp.end(), usertmp.begin(), tolower);
			std::transform(strCurUser.begin(), strCurUser.end(), strCurUser.begin(), tolower);

			if( user != strCurUser )
			{
				std::wstring::size_type pos = path.find(strCurUser.c_str());
				if(pos > 0 )
				{
					xstring newTemp = path.substr(0,pos);
					newTemp += user;
					newTemp += path.substr(pos+strCurUser.size());
					newpath = newTemp;
				}
			}
			if( !newpath.empty() )
			{
				const xstring& cp = getRexConfPath();
				xstring* p = (xstring*)&cp;
				*p = newpath;
			}
			return getRexConfPath();
		}

		static BOOL isWindowsVistaAbove()
		{
			static int s_isVista = -1;

			if(-1 == s_isVista)
			{	
				OSVERSIONINFO versionInfo;

				versionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

				::GetVersionEx(&versionInfo); 

				s_isVista = 0;
				if(versionInfo.dwMajorVersion >= 6)
					s_isVista = 1;
			}

			return s_isVista;
		}

		static BOOL CreateDirectoryEx(LPCTSTR lpPath)
		{
			xstring path  = lpPath;

			TCHAR* lpPtr = (TCHAR*)path.data();
			//�Ƿ��ļ��Ȅh��
			if( IsUnlegalPath(lpPath) ) {
				DeleteFile(lpPath);
			}

			if( PathFileExists(lpPath) )
				return TRUE;
			while( lpPtr[0] )
			{
				if( lpPtr[0] == '\\' )
				{
					lpPtr[0] = NULL;
					if(!::PathFileExists(path.c_str())){
						CreateDirectory(path.c_str(),NULL);
					}
					lpPtr[0] = '\\';
				}
				lpPtr++;
			}

			return CreateDirectory(path.c_str(),NULL);
		}

		static xstring GetTempFile()
		{
			TCHAR szTempPath[MAX_PATH];
			TCHAR szTempName[MAX_PATH];

			::GetTempPath(_countof(szTempPath), szTempPath); 
			::GetTempFileName(szTempPath,_T("rc"),0, szTempName);
			return szTempName;
		}
		static xstring GetTempPath()
		{
			TCHAR szTempPath[MAX_PATH];
			::GetTempPath(MAX_PATH, szTempPath); 
			return szTempPath;
		}

		static xstring GetRCTempFile(LPCTSTR lpPreFix = _T("rc"))
		{
			xstring path = helper::ushell::getRexConfPath() + _T("RCTemp\\");
			CreateDirectoryEx(path.c_str());

			SYSTEMTIME st; 
			GetLocalTime(&st);

			TCHAR szTempName[MAX_PATH] = {0};
			//ǰ�Y + ������ + tickcount
			swprintf(szTempName, _T("%s%s_%d%d%d%ld.tmp"),path.c_str(), lpPreFix, st.wYear, st.wMonth, st.wDay, GetTickCount());
			return szTempName;
		}

		static xstring GetRCTempPath()
		{
			xstring path = helper::ushell::getRexConfPath() + _T("RCTemp\\");
			CreateDirectoryEx(path.c_str());
			return path;
		}

		static xstring GetTempHtmlPath()
		{
			xstring path = helper::ushell::getRexConfPath() + _T("html\\");
			CreateDirectoryEx(path.c_str());
			return path;
		}
		static xstring GetTempJsPath()
		{
			xstring path = helper::ushell::GetTempHtmlPath() + _T("js\\");
			CreateDirectoryEx(path.c_str());
			return path;
		}
		static xstring GetTempCssPath()
		{
			xstring path = helper::ushell::GetTempHtmlPath() + _T("css\\");
			CreateDirectoryEx(path.c_str());
			return path;
		}

		static xstring GetRCChatPath(const xstring& account)
		{
			xstring path = helper::ushell::getRexConfPath();
			path +=  account;
			path +=  _T("\\ChatImage\\");
			CreateDirectoryEx(path.c_str());
			return path;
		}

		static xstring GetIePath()
		{
			xstring strIePath ;

			HKEY hRoot = NULL, hSubKey = NULL;
			if(RegOpenKey(HKEY_LOCAL_MACHINE, NULL, &hRoot) == ERROR_SUCCESS)
			{
				if(RegOpenKeyEx(hRoot, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE"),
					0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
				{
					DWORD dwType = 0;
					TCHAR path[MAX_PATH];
					DWORD cbData = MAX_PATH;
					if(RegQueryValueEx(hSubKey, NULL, NULL, &dwType, (LPBYTE)path, &cbData) == ERROR_SUCCESS)
					{
						strIePath = path;
					}
					RegCloseKey(hSubKey);
				}
				RegCloseKey(hRoot);
			}
			return strIePath;
		}

		static BOOL GotoUrlInIE(const xstring& lpURL)
		{
			xstring strIePath = GetIePath();
			if(strIePath.empty())
				return FALSE;

			ShellExecute(0,_T("open"),strIePath.c_str(),lpURL.c_str(),0,SW_SHOWNORMAL);
			return TRUE;
		}

		static void GoToURL(const xstring& lpURL)
		{
			::ShellExecute(0, _T("open"), lpURL.c_str(), 0, 0, SW_SHOWNORMAL);
		}

		//before use this function, you must initial com
		static BOOL OpenFolderAndSelectFile( LPCTSTR lpszFilePath)
		{
			if (!lpszFilePath)
				return FALSE;

			xstring strFile = lpszFilePath;
			int nPos = strFile.rfind('\\');
			xstring strFolder = (nPos > 0) ? strFile.substr(0,nPos) : strFile;
			// Get a pointer to the Desktop's IShellFolder interface.
			LPSHELLFOLDER pDesktopFolder;
			if ( SUCCEEDED(SHGetDesktopFolder( &pDesktopFolder)))
			{
				// Convert the path to an ITEMIDLIST.
				LPITEMIDLIST     pidl;
				ULONG             chEaten;
				ULONG             dwAttributes;
				HRESULT             hr;

				TCHAR tcFolder[MAX_PATH];
				TCHAR tcFile[MAX_PATH];

				lstrcpy(tcFolder, strFolder.c_str());
				lstrcpy(tcFile, strFile.c_str());

				//folder
				hr = pDesktopFolder->ParseDisplayName(
					NULL, NULL, tcFolder, &chEaten, &pidl, &dwAttributes);
				if (FAILED(hr))
				{
					pDesktopFolder->Release();
					return FALSE;
				}
				LPCITEMIDLIST pidlFolder = pidl;

				//file
				hr = pDesktopFolder->ParseDisplayName(
					NULL, NULL, tcFile, &chEaten, &pidl, &dwAttributes);
				if (FAILED(hr))
				{
					pDesktopFolder->Release();
					return FALSE;
				}
				LPCITEMIDLIST pidlFile = pidl;

				hr = SHOpenFolderAndSelectItems( pidlFolder, 1, &pidlFile, 0);
				pDesktopFolder->Release();

				if ( hr == S_OK)
				{
					return TRUE;
				}
			}
			return FALSE;
		}



		static xstring GetCurProcessUserName()
		{
			HANDLE hToken = NULL;
			BOOL bFuncReturn = FALSE;	
			PTOKEN_USER pToken_User = NULL;
			DWORD dwTokenUser = 0;
			TCHAR g_szAccName[MAX_PATH] = {0};
			TCHAR szDomainName[MAX_PATH] = {0};
			HANDLE hProcessToken = NULL;

			xstring strUserName;
			
			HANDLE hProcess = GetCurrentProcess();
			if(hProcess != NULL)
			{
				// ���������̵�Ȩ��

				bFuncReturn = ::OpenProcessToken(hProcess,TOKEN_QUERY,&hToken);

				if( bFuncReturn == 0) // ʧ��
				{
					return g_szAccName;
				}

				if(hToken != NULL)
				{
					::GetTokenInformation(hToken, TokenUser, NULL,0L, &dwTokenUser);

					if(dwTokenUser>0)
					{
						pToken_User = (PTOKEN_USER)::GlobalAlloc( GPTR, dwTokenUser );
					}

					if(pToken_User != NULL)
					{
						bFuncReturn = ::GetTokenInformation(hToken, TokenUser, pToken_User, dwTokenUser, &dwTokenUser);
					}

					if(bFuncReturn != FALSE && pToken_User != NULL)
					{
						SID_NAME_USE eUse = SidTypeUnknown;

						DWORD dwAccName = 0L;  
						DWORD dwDomainName = 0L;

						PSID pSid = pToken_User->User.Sid;

						bFuncReturn = ::LookupAccountSid(NULL, pSid, NULL, &dwAccName,
							NULL,&dwDomainName,&eUse );
						if(dwAccName>0 && dwAccName < MAX_PATH && dwDomainName>0 && dwDomainName <= MAX_PATH)
						{
							bFuncReturn = ::LookupAccountSid(NULL,pSid,g_szAccName,&dwAccName,
								szDomainName,&dwDomainName,&eUse );
							if( bFuncReturn )
								strUserName = g_szAccName;
						}
					}
					if (pToken_User != NULL)
					{
						::GlobalFree( pToken_User );
					}
					::CloseHandle(hToken);
				}
			}

			return strUserName;
		}  

    };
}