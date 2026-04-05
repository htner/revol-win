#include <winsock2.h>
//#include <Ws2tcpip.h>
#include <tchar.h>
#include <macros.h>
#include "ping/pingcallback.h"
#include "ping/ping.h"
#include "WinPingMgr.h"
#include <cstring>

#define WM_SOCKET_DATA	(WM_USER + 1)

#define TIME_SLAPSE	100
#define DT_PING_TIMEOUT	1



PingServerManager* PingServerManager::inst()
{
	static PingServerManager g_ping;
	return &g_ping;
}

PingServerManager::PingServerManager()
{
	sock_ = INVALID_SOCKET;
	hwnd_ = NULL;
};

PingServerManager::~PingServerManager()
{
	CleanSock();
	DestroyWnd();
}

bool  PingServerManager::BeginPing(LPCSTR host,CWinPingCallBack* callback,int timeout,int id )
{
	if(strlen(host) < 3 )
		return false;

	CreateSocketWnd();
	InitSock();

	if(INVALID_SOCKET == sock_) {
		return false;
	}
			
	sockaddr_in  desAddr;  
	ZeroMemory(&desAddr, sizeof(desAddr));
	desAddr.sin_family = AF_INET;

	if(((host[0] >='a' && host[0] <= 'z') || (host[0] >='A' && host[0] <= 'Z')) ||
		((host[1] >='a' && host[1] <= 'z') || (host[1] >='A' && host[1] <= 'Z')) ||
		((host[2] >='a' && host[2] <= 'z') || (host[2] >='A' && host[2] <= 'Z')) )
	{
		addrinfo* addOut = NULL;
		getaddrinfo(host, "", NULL, &addOut);
		if(NULL == addOut) {
			return FALSE;
		}
		desAddr.sin_addr.s_addr= ((sockaddr_in *)addOut->ai_addr)->sin_addr.s_addr;
	}else
	{
		desAddr.sin_addr.s_addr= inet_addr(host);
	}
	
	
	static int g_nseq = 0;
	
	g_nseq++;

	CWinPingTask task;
	task.id_ = id;
	task.ping_ = -1;
	task.callback_ = callback;
	task.pingfrm_ = GetTickCount();
	task.timeout_ = timeout;
	task.seq_ = g_nseq;
	if (strlen(host) > 120) {
		return false;
	}
	strcpy(task.host_, host);
	WSAAsyncSelect(sock_, hwnd_, WM_SOCKET_DATA, FD_READ);
	if(!CPing::SendPing(sock_,host,g_nseq))
	{
		if(callback)
			callback->OnPingCallBack(task);
		return false;
	}
	pings_.push_back(task);
	return true;
}

bool  PingServerManager::EndPing(CWinPingCallBack* callback,int id)
{
	if(INVALID_SOCKET == sock_)
	{
		return false;
	}
	std::list<CWinPingTask>::iterator it =	pings_.begin();	
	for (; it != pings_.end(); )
	{
		if( it->callback_ == callback && (it->id_ == id || id == -1) )
			it = pings_.erase( it );
		else 
			it++;
	}
	if( pings_.empty() )
	{
		CleanSock();
		DestroyWnd();
	}
	//WSAAsyncSelect(m_sock, NULL, -1, FD_READ);
	return true;
}


LRESULT PingServerManager::WndPingProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return PingServerManager::inst()->WndPingProcInt(hWnd,uMsg,wParam,lParam);
}

void PingServerManager::CreateSocketWnd()
{
	if( IsWindow(hwnd_) )
		return;

	/*
	static TCHAR szAppName[] = _T("rex-ping-fast") ;
	WNDCLASS wndclass ;
	wndclass.style		  = CS_HREDRAW | CS_VREDRAW ;
	wndclass.lpfnWndProc  = WndPingProc;
	wndclass.cbClsExtra	  = 0 ;
	wndclass.cbWndExtra	  = 0 ;
	wndclass.hInstance	  = NULL;
	wndclass.hIcon		  = NULL ;
	wndclass.hCursor	  = NULL;
	wndclass.hbrBackground	= NULL;
	wndclass.lpszMenuName	= NULL ;
	wndclass.lpszClassName	= szAppName ;
	RegisterClass(&wndclass);

	hwnd_ = CreateWindow(szAppName,	// window class name
		NULL,	// window caption
		WS_DISABLED,	// window style
		0,	// initial x position
		0,	// initial y position
		0,	// initial x size
		0,	// initial y size
		NULL,			// parent window handle
		NULL,	        // window menu handle
		NULL,	    // program instance handle
		NULL) ; 	    // creation parameters
	*/
	WNDCLASSEX wndclass;
	wndclass.cbSize=sizeof wndclass;
	wndclass.style=0;
	wndclass.lpfnWndProc=WndPingProc;
	wndclass.cbClsExtra=0;
	wndclass.cbWndExtra=0;
	wndclass.hInstance=0;//GetModuleHandle(0);
	wndclass.hIcon=0;
	wndclass.hCursor=0;
	wndclass.hbrBackground=0;
	wndclass.lpszMenuName=0;
	wndclass.lpszClassName=_T("rc_ping_manager_wnd");
	wndclass.hIconSm=0;

	ATOM res = RegisterClassEx(&wndclass);
	DWORD ping_windCow_error = GetLastError();

	hwnd_=CreateWindow(_T("rc_ping_manager_wnd"), _T("rc_ping_manager_window"), 0, 0, 0, 0, 0, 0, 0, 0, 0);
	//ASSERT(hwnd_ != NULL);
	DWORD ping_window_error = GetLastError();

	SetTimer(hwnd_,DT_PING_TIMEOUT,TIME_SLAPSE,NULL);

	//TRACE("ping window created\n");
}
void PingServerManager::InitSock()
{
	if( sock_ == INVALID_SOCKET)
	{			
		int timeout = 1000;
		sock_ = ::socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
		//ASSERT(sock_ != INVALID_SOCKET);
		if ( sock_ != INVALID_SOCKET)
			setsockopt(sock_,SOL_SOCKET,SO_RCVTIMEO,(const char *)&timeout,sizeof(timeout));
	}
}

void PingServerManager::CleanSock()
{
	if( sock_ ) {
		closesocket(sock_);
		sock_ = INVALID_SOCKET;
	}
}

void PingServerManager::DestroyWnd()
{
	if(hwnd_)
	{
		DestroyWindow(hwnd_);
		hwnd_ = NULL;
		// ATRACE("ping window destroyed\n");
	}

}

LRESULT PingServerManager::WndPingProcInt(HWND hWnd,UINT uMsg,WPARAM wParam=0,LPARAM lParam=0)
{
	switch(uMsg)
	{
	case WM_TIMER:
		{
			if( wParam == DT_PING_TIMEOUT )
			{
				if( pings_.empty() )
				{					
					CleanSock();
					DestroyWnd();
					break;
				}
				DWORD dwTick = GetTickCount();
				std::list<CWinPingTask>::iterator it =	pings_.begin();
				for (; it != pings_.end(); it++)
				{
					int elapse = dwTick - it->pingfrm_;
					if( it->timeout_ > 0 && elapse >= it->timeout_)
					{
						it->ping_ = -1;
						if( it->callback_ )
							it->callback_->OnPingCallBack(*it);

						pings_.erase( it );
						break;
					}
				}
			}
		}
		break;
	case WM_SOCKET_DATA:
		{
			SOCKET ss = wParam;   
			if (WSAGETSELECTERROR(lParam)) 
			{ 					
				return 0; 
			} 
			int code = WSAGETSELECTEVENT(lParam);
			switch (code) 
			{ 
			case FD_READ:
				{ 
					int seq = CPing::RecvEchoReply(ss);
					if( seq >=0 )
					{
						DWORD dwTick = GetTickCount();
						std::list<CWinPingTask>::iterator it =	pings_.begin();
						for (; it != pings_.end(); it++)
						{
							if( it->seq_ == seq )
							{
								it->ping_ = dwTick - it->pingfrm_;
								if( it->callback_ )
									it->callback_->OnPingCallBack(*it);

								pings_.erase( it );
								break;
							}
						}
					}					
				}
				break; 
			}
		}
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return 0;
}