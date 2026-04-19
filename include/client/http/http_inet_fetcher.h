#pragma once
#include "helper/DammyWnd.h"
#include <vector>
#include "neohttp.h"
#include "http_fetcher_cb.h"

class CSingleFetcher;

class http_inet_fetcher : public IHttp_fetcher
						, public IDammyWndEvent
{
public:
	typedef	struct tag_ThreadContext{
		tag_ThreadContext(http_inet_fetcher	*pFetch, int nIndex)
		{
			_pFetch = pFetch;
			_nIndex = nIndex;
		}
		http_inet_fetcher	*_pFetch;
		int					_nIndex;
	}ThreadContext;

	http_inet_fetcher();
	virtual ~http_inet_fetcher();

	//IHttp_fetcher
	virtual int  get(LPCTSTR url,int type, http_fetcher_cb*,bool bIgnoreCache = false);
	virtual int  post(LPCTSTR url, http_post_data& params, int type, http_fetcher_cb*);
	virtual int  postJson(LPCTSTR url, Json::Value& params, int type,  http_fetcher_cb*);
	virtual void remove(http_fetcher_cb * cb);
	virtual void remove(int nContext);
	virtual void clear();

	//IDammyWndEvent
	virtual void OnDamyyWndEvent(HWND hwnd,UINT nMsg,WPARAM wParam,LPARAM lParam);

	static DWORD ThreadProc(LPVOID lpParam);
protected:
	void Start(LPCTSTR lpUrl);
	void GetProc(int nIndex);
	BOOL IsInList(int nContext);
	CSingleFetcher* GetNextTask();
	void DoCloseHandle(int nIndex);
private:
	typedef	 std::vector<CSingleFetcher *>	DowndLoadList;
	typedef	 std::map<int, HANDLE>			HandleMap;

	DowndLoadList				m_downloadList;
	HandleMap					m_handleMap;
	int							m_nContext;
	BOOL						m_bRuning;
	CRITICAL_SECTION			m_cs;
	CDammyWnd					m_dammyWnd;
};

#include "http_inet_fetcher.hpp"