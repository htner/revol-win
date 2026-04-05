#pragma once
#ifndef _WIN_PING_MANAGER_H
#define _WIN_PING_MANAGER_H
#include <list>
#include <string>
#include "timer/TimerWrap.h"

struct CWinPingCallBack;
struct CWinPingTask
{
	int		id_;
	char	host_[128];
	int		timeout_;
	int		pingfrm_;
	int		ping_;
	int		seq_;
	CWinPingCallBack* callback_;
};


struct CWinPingCallBack
{
	virtual void OnPingCallBack(const CWinPingTask& task)	= 0;
};


class PingServerManager
{
public:
	static PingServerManager* inst();	
	~PingServerManager();
	bool  BeginPing(LPCSTR hostAddr,CWinPingCallBack* cb,int timeout,int id = -1);
	bool  EndPing(CWinPingCallBack* cb,int id = -1);
protected:	
	PingServerManager();	
	void CreateSocketWnd();
	void InitSock();
	void CleanSock();
	void DestroyWnd();
	LRESULT  WndPingProcInt(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	static LRESULT CALLBACK  WndPingProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
private:
	int sock_;
	HWND   hwnd_;
	std::list<CWinPingTask>	pings_;
};


#endif