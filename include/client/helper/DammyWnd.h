#pragma once
#include <atlwin.h>
#include <list>

struct IDammyWndEvent
{
	virtual void OnDamyyWndEvent(HWND hwnd,UINT nMsg,WPARAM wParam,LPARAM lParam) = 0;
};

class CDammyWnd : public CWindowImpl<CDammyWnd>
{
public:
	CDammyWnd(){
		_nMsg = NULL;
		_lpListener = NULL;		
	};
	~CDammyWnd(){
		if( IsWindow() )
			DestroyWindow();
	};
	BEGIN_MSG_MAP(CDammyWnd)
		if( uMsg == _nMsg && _lpListener )
			_lpListener->OnDamyyWndEvent(hWnd,uMsg,wParam,lParam);

	END_MSG_MAP()
public:
	void Create()
	{
		if( !IsWindow())
			__super::Create(NULL,NULL,NULL,WS_POPUP);
	}
	void Destroy()
	{
		if(IsWindow())
			DestroyWindow();
	}
	void Fire(WPARAM w,LPARAM l,BOOL bPost = FALSE)
	{
		if(bPost)
			PostMessage(_nMsg,w,l);
		else
			SendMessage(_nMsg,w,l);
	}
	void Watch(IDammyWndEvent* lpListener,UINT nMsg)
	{
		_nMsg = nMsg;
		_lpListener = lpListener;
	}
	void Revoke()
	{
		_nMsg = NULL;
		_lpListener = NULL;
	}
protected:
	UINT _nMsg;
	IDammyWndEvent* _lpListener;
};