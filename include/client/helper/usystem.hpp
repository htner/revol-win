#include "usystem.h"
#include <ShellAPI.h>
#include "macros.h"
#include "LXThread.h"
#include "commctrl.h"
#include "tchar.h"
#include "Iphlpapi.h"

#pragma comment(lib, "IPHLPAPI.lib")

#define BYTETOMBYTE (1024*1024)

class OpenUrlThread : public LXThread
{
public:
	OpenUrlThread(LPCTSTR url){				
				__url += url;
	}
	virtual void Run(){				
				ShellExecute(NULL, L"open", __url.c_str(), NULL, NULL, SW_SHOW);		
				delete this;
	}
protected:
	std::wstring __url;
};

namespace helper
{
	LOGFONT usystem::GetDefaultLF(void)
	{
		LOGFONT lf;
		ZeroMemory(&lf, sizeof(LOGFONT));

		HFONT hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);

		if(NULL == hFont)
			return lf;

		::GetObject(hFont, sizeof(LOGFONT), &lf);

		::DeleteObject(hFont);
		hFont = NULL;

		return lf;
	}

	void usystem::AnalyseHotKey(DWORD dwKey, WORD& wVirtualKeyCode, WORD&	wModifiers)
	{
		wVirtualKeyCode			=LOWORD(dwKey);
		wModifiers				=HIWORD(dwKey);
	}

	void usystem::ToHotKey(DWORD dwKey, WORD& wVirtualKeyCode, WORD&	wModifiers)
	{
		wVirtualKeyCode			=LOWORD(dwKey);
		DWORD	wTemp			=HIWORD(dwKey);
		wModifiers				=0;
		if(include(wTemp, HOTKEYF_ALT))
			wModifiers |= MOD_ALT;
		if(include(wTemp, HOTKEYF_SHIFT))
			wModifiers |= MOD_SHIFT;
		if(include(wTemp, HOTKEYF_CONTROL))
			wModifiers |= MOD_CONTROL;
	}

	void usystem::MakeHotKey(DWORD& dwKey, WORD& wVirtualKeyCode, WORD&	wModifiers)
	{
		dwKey = MAKELONG(wVirtualKeyCode, wModifiers);
	}

	void usystem::ForceWindowShowOnTop(HWND hWnd)
	{
		::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
		::SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
	}

	WAVEFORMATEX usystem::GetWaveFormat(WORD wFormatTag, int nChannel, int nSample, int nBits)
	{
		WAVEFORMATEX wfx;
		wfx.wFormatTag		=   wFormatTag;   
		wfx.nChannels		=   nChannel;   
		wfx.nSamplesPerSec  =   nSample;   
		wfx.nAvgBytesPerSec =   nSample   *   nChannel   *   nBits/8;   
		wfx.nBlockAlign		=   nChannel   *   nBits/8;   
		wfx.wBitsPerSample  =   nBits;   
		wfx.cbSize   =   sizeof(WAVEFORMATEX);   
		return wfx;
	}


	bool usystem::IsKeyPressed(int nKey)
	{
		const	int KEYPRESSMASK	=0x80000000;

		SHORT	nVal	=::GetAsyncKeyState(nKey);

		/*
		The return value specifies the status of the specifed virtual key, as follows: 
		If the high-order bit is 1, the key is down; otherwise, it is up. 
		If the low-order bit is 1, the key is toggled. A key, such as the CAPS LOCK key, is toggled if it is turned on. The key is off and untoggled if the low-order bit is 0. A toggle key’s indicator light (if any) on the keyboard will be on when the key is toggled, and off when the key is untoggled. 
		*/
		if((nVal & KEYPRESSMASK) == KEYPRESSMASK)
		{
			return true;
		}

		return false;
	}

	void usystem::OpenURL(LPCTSTR lpszURL)
	{
		OpenUrlThread* xx = new OpenUrlThread(lpszURL);
		xx->Resume();
	}
	xstring	usystem::GetKeyNameText(int nkey)
	{

#ifndef VK_XBUTTON1
	#define VK_XBUTTON1       0x05    
#endif

#ifndef VK_XBUTTON2
	#define VK_XBUTTON2       0x06
#endif

		switch(nkey){
			case VK_LBUTTON:
				{
					return _T("Left Mouse Button");
				}
				break;
			case VK_RBUTTON:
				{
					return  _T("Right Mouse Button");
				}
				break;
			case VK_MBUTTON:
				{
					return  _T("Middle Mouse Button");
				}
				break;
			case VK_XBUTTON1:
				{
					return  _T("Mouse XButton1");
				}
				break;
			case VK_XBUTTON2:
				{
					return  _T("Mouse XButton2");
				}
				break;
			case VK_UP:
				{
					return  _T("Up");
				}
				break;
			case VK_DOWN:
				{
					return  _T("Down");
				}
				break;
			case VK_LEFT:
				{
					return  _T("Left");
				}
				break;
			case VK_RIGHT:
				{
					return  _T("Right");
				}
				break;
			default:
				{
					xstring strPrefix;
					if( nkey == VK_LMENU ||
						nkey == VK_LSHIFT ||
						nkey == VK_LCONTROL)
						strPrefix = _T("Left ");
					else if( nkey == VK_RMENU ||
						 nkey == VK_RCONTROL)
						strPrefix = _T("Right ");

					TCHAR szKeyName[MAX_PATH] = { 0 };
					::GetKeyNameText((::MapVirtualKey(nkey, 0) << 16), szKeyName, MAX_PATH);
					xstring xFullkey = strPrefix + szKeyName;
					return xFullkey;
				}
		}
	}

	xstring	usystem::GetHotKeyName(int nKey)
	{
		int nHigh = HIBYTE(nKey);
		int nLow = LOBYTE(nKey);
		xstring strKeyName;
		if( HOTKEYF_CONTROL & nHigh ) {
			strKeyName += GetKeyNameText(VK_CONTROL) + _T("+");
		}
		if( HOTKEYF_SHIFT & nHigh ) {
			strKeyName += GetKeyNameText(VK_SHIFT) + _T("+");
		}
		if( HOTKEYF_ALT & nHigh ) {
			strKeyName += GetKeyNameText(VK_MENU) + _T("+");
		}
		strKeyName += GetKeyNameText(nLow);
		return strKeyName;
	}

	xstring usystem::GetSysInfo()
	{
		//计算机用户名
		TCHAR szUserName[1024] = {0};
		DWORD nSize = sizeof(szUserName)/sizeof(TCHAR);
		GetUserName(szUserName, &nSize);
		//操作系统版本
		xstring strOsVersion;
		OSVERSIONINFO versionInfo;
		versionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (!::GetVersionEx(&versionInfo)) {
			strOsVersion = (_T ("Not able to get OS information"));
		}
		ATL::CString strVerNum;
		strVerNum.Format(_T("%d.%d"), versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);
		xstring strUnKnowVer = _T("UnKnow Version ") + xstring(LPCTSTR(strVerNum));
		if (versionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
			if(versionInfo.dwMajorVersion <= 4)  {
				strOsVersion =  _T ("Windows NT ");
			} else if((versionInfo.dwMajorVersion==5) && (versionInfo.dwMinorVersion==0)) {//5.0是Windows2000 
				strOsVersion =  _T ("Windows 2000 ");
			} else if((versionInfo.dwMajorVersion==5) && (versionInfo.dwMinorVersion==1)) {//5.1几WindowsXP
				strOsVersion =  _T ("Windows XP ");
			} else if((versionInfo.dwMajorVersion==5) && (versionInfo.dwMinorVersion==2)) {//5.2Windows XP Professional x64 Edition或Windows 2003
				strOsVersion =  _T ("Windows XP Professional x64 Edition or Windows 2003 ");
			}
			else if(versionInfo.dwMajorVersion==6) {
				if (versionInfo.dwMinorVersion == 0) 
					strOsVersion = _T("Windows Vista ");
				else if (versionInfo.dwMinorVersion == 1)
					strOsVersion = _T("Windows 7 ");
				else { 
					strOsVersion = strUnKnowVer;
				}
			} else  {
				strOsVersion = strUnKnowVer;
			}
		}
		strOsVersion += versionInfo.szCSDVersion;
		//系统信息
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		xstring strPrcoessType;
		switch (sysInfo.dwProcessorType) 
		{
		case PROCESSOR_INTEL_386: strPrcoessType = _T("386"); break;
		case PROCESSOR_INTEL_486: strPrcoessType = _T("486"); break;
		case PROCESSOR_INTEL_PENTIUM: strPrcoessType = _T("586"); break;
		case PROCESSOR_INTEL_IA64 : strPrcoessType = _T("2200"); break;
		case PROCESSOR_AMD_X8664:   strPrcoessType = _T("8664"); break;
		default: break;

		}
		//内存信息
		MEMORYSTATUS memStatus;
		ZeroMemory(&memStatus, sizeof(MEMORYSTATUS));
		memStatus.dwLength = sizeof(MEMORYSTATUS);
		GlobalMemoryStatus(&memStatus);
		int nInUse = memStatus.dwMemoryLoad;
		int nPhyTotal = memStatus.dwTotalPhys/BYTETOMBYTE;
		int nPyhFree = memStatus.dwAvailPhys/BYTETOMBYTE;
		int nPageTotal = memStatus.dwTotalPageFile/BYTETOMBYTE;
		int nPageFree = memStatus.dwAvailPageFile/BYTETOMBYTE;
		int nVirtualTotal = memStatus.dwTotalVirtual/BYTETOMBYTE;
		int nVirtualFree = memStatus.dwAvailVirtual/BYTETOMBYTE;
		//分辨率
		int cx = GetSystemMetrics(SM_CXSCREEN);
		int cy = GetSystemMetrics(SM_CYSCREEN);

		ATL::CString strData;
		strData.Format(_T("UserName: %s;\n \
						  Operating system: %s;\n \
						  %d processor(s), type %s;\n \
						  %%%d memory in use;\n \
						  %d MBytes physical memory;\n \
						  %d MBytes physical memory free;\n \
						  %d MBytes paging file;\n \
						  %d MBytes paging file free;\n \
						  %d MBytes user address space;\n \
						  %d MBytes user address space free;\n \
						  Screen width: %d, height :%d\n"), 
						  szUserName, strOsVersion.c_str(), sysInfo.dwNumberOfProcessors, strPrcoessType.c_str(),
						  nInUse, nPhyTotal, nPyhFree, nPageTotal, nPageFree, nVirtualTotal, nVirtualFree,
						  cx, cy);
		return (LPCTSTR)strData;
	}

	xstring usystem::GetMacAddress()
	{
		xstring csMacAddress;
		ULONG BufferLength = 0;
		BYTE* pBuffer = 0;
		if( ERROR_BUFFER_OVERFLOW == GetAdaptersInfo( 0, &BufferLength ))
		{
			pBuffer = new BYTE[ BufferLength ];

			// Get the Adapter Information.
			PIP_ADAPTER_INFO pAdapterInfo =
				reinterpret_cast<PIP_ADAPTER_INFO>(pBuffer);
			GetAdaptersInfo( pAdapterInfo, &BufferLength );


			while( pAdapterInfo )
			{
				if( !pAdapterInfo->Address[0] &&
					!pAdapterInfo->Address[1] &&
					!pAdapterInfo->Address[2] &&
					!pAdapterInfo->Address[3] &&
					!pAdapterInfo->Address[4] &&
					!pAdapterInfo->Address[5])
				{
					pAdapterInfo = pAdapterInfo->Next;
					continue;
				}
				TCHAR szMac[128] = {0};
				_stprintf(szMac,
							_T("%02x:%02x:%02x:%02x:%02x:%02x"),
							pAdapterInfo->Address[0],
							pAdapterInfo->Address[1],
							pAdapterInfo->Address[2],
							pAdapterInfo->Address[3],
							pAdapterInfo->Address[4],
							pAdapterInfo->Address[5]);
				csMacAddress = szMac;
				break;
			}	
			delete[] pBuffer;
		}			
		return csMacAddress;		
	}

	//* Union to facilitate converting from FILETIME to unsigned __int64
	typedef union {
		unsigned __int64 ft_scalar;
		FILETIME ft_struct;
	} FT;

	// * Number of 100 nanosecond units from 1/1/1601 to 1/1/1970
	#define EPOCH_BIAS  116444736000000000i64
	#define _MAX__TIME32_T     0x7fffd27f           /* number of seconds from
                                                   00:00:00, 01/01/1970 UTC to
                                                   23:59:59, 01/18/2038 UTC */
	INT32 usystem::time32 ()
	{

		INT64 tim;
		FT nt_time;

		GetSystemTimeAsFileTime( &(nt_time.ft_struct) );

		
		tim = (INT64)((nt_time.ft_scalar - EPOCH_BIAS) / 10000000i64);

		if (tim > (INT64)(_MAX__TIME32_T))
			tim = (INT64)(-1);

		return (INT32)(tim);
	}
}

