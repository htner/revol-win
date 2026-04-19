#include "http_inet_fetcher.h"
#include "SingleFetcher.h"

#define MAX_THREAD	8
#define WM_GETHTTPDATA	(WM_USER + 200)

class CAutoLock
{
public:
	CAutoLock(LPCRITICAL_SECTION cs)
	{
		_cs = cs;
		EnterCriticalSection(_cs);
	}
	~CAutoLock()
	{
		LeaveCriticalSection(_cs);
	}
	LPCRITICAL_SECTION _cs;
};

inline http_inet_fetcher::http_inet_fetcher()
{
	m_bRuning = TRUE;
	m_nContext = 0;

	InitializeCriticalSection(&m_cs);
	m_dammyWnd.Revoke();
	m_dammyWnd.Destroy();
}

inline http_inet_fetcher::~http_inet_fetcher()
{
	DeleteCriticalSection(&m_cs);
}

inline int http_inet_fetcher::get(LPCTSTR url,int type, http_fetcher_cb* pCallback, bool bIgnoreCache)
{
	m_nContext++;
	CSingleFetcher *task = new CSingleFetcher(url, type, m_nContext, pCallback, TRUE);
	{
		CAutoLock lock(&m_cs);
		m_downloadList.push_back(task);
	}
	Start(url);
	return m_nContext;
}

inline int http_inet_fetcher::post(LPCTSTR url, http_post_data& params, int type, http_fetcher_cb* pCallback)
{
	m_nContext++;

	std::string PostData;
	http_post_data::value_iterator it=		params.values.begin();
	http_post_data::value_iterator itend =	params.values.end();
	for (; it!=itend; it++)
	{
		PostData += it->first + "=" + it->second + "&";
	}

	CSingleFetcher *task = new CSingleFetcher(url, type, m_nContext, pCallback, FALSE, PostData);
	{
		CAutoLock lock(&m_cs);
		m_downloadList.push_back(task);
	}

	Start(url);
	return m_nContext;
}
inline int http_inet_fetcher::postJson(LPCTSTR url, Json::Value& params, int type, http_fetcher_cb* pCallback) {
	m_nContext++;

	std::string PostData;
	try {
		Json::FastWriter writer;
		params["tool"] = "inet";
		PostData = writer.write(params);
	}
	catch (...) {

	}

	CSingleFetcher *task = new CSingleFetcher(url, type, m_nContext, pCallback, FALSE, "", PostData);
	{
		CAutoLock lock(&m_cs);
		m_downloadList.push_back(task);
	}

	Start(url);
	return m_nContext;
}

inline void http_inet_fetcher::remove(http_fetcher_cb * cb)
{
	CAutoLock lock(&m_cs);

	DowndLoadList::iterator itr = m_downloadList.begin();
	for( ; itr!= m_downloadList.end(); )
	{
		if( CSingleFetcher *pUnit = *itr ) {
			if( pUnit->m_pCallback == cb ) {
				itr = m_downloadList.erase(itr);
				delete pUnit;
			} else {
				++itr;
			}
		} else {
			++itr;
		}
	}
}

inline void http_inet_fetcher::remove(int nContext)
{
	CAutoLock lock( &m_cs );

	DowndLoadList::iterator itr = m_downloadList.begin();
	DowndLoadList::iterator itrEnd = m_downloadList.end();
	for( ; itr != itrEnd; ++itr )
	{
		if( CSingleFetcher *pUnit = *itr ) {
			if( pUnit->m_nContext == nContext ) {
				m_downloadList.erase(itr);
				delete pUnit;
				break;
			}
		}
	}
}

inline void http_inet_fetcher::clear()
{
	{
		m_bRuning = FALSE;
		int i = 0;
		while( i < 10 ) 
		{
			Sleep(10);
			i++;
		}
		CAutoLock lock(&m_cs);
		HandleMap::iterator itr = m_handleMap.begin();
		HandleMap::iterator itrEnd = m_handleMap.end();
		ATLTRACE("**************The Terminate Size: %d****************\n", m_handleMap.size());
		for( ; itr != itrEnd; ++itr )
		{
			TerminateThread(itr->second, 0);
			CloseHandle(itr->second);
		}
		m_handleMap.clear();
	}

	DowndLoadList::iterator itr = m_downloadList.begin();
	DowndLoadList::iterator itrEnd = m_downloadList.end();
	for( ; itr != itrEnd; ++itr )
	{
		if( CSingleFetcher* pUnit = *itr ) {
			delete pUnit;
		}
	}
	m_downloadList.clear();
}

inline void http_inet_fetcher::OnDamyyWndEvent(HWND hwnd,UINT nMsg,WPARAM wParam,LPARAM lParam)
{
	if( WM_GETHTTPDATA == nMsg ) {
		if( ResultParam *pResult = (ResultParam*)wParam ) {
			if( !IsInList(pResult->_nContext) ) {
				ATLTRACE("*************Has been remove*************\n");
				return;
			}

			if( pResult->_pCallBack )
			{
				if( 200 == pResult->_dwStatusCode ) {
					pResult->_pCallBack->on_http_data(pResult->_nContext, pResult->_nType, pResult->_data);
				} else {
					pResult->_pCallBack->on_http_error(pResult->_nContext, pResult->_nType, pResult->_data, pResult->_dwStatusCode);
				}
			}

			remove( pResult->_nContext );
		}
	}
}

inline DWORD http_inet_fetcher::ThreadProc(LPVOID lpParam)
{
	if( ThreadContext *pContext = (ThreadContext *)lpParam ) {
		pContext->_pFetch->GetProc(pContext->_nIndex);
		delete pContext;
	}
	return 0;
}

inline void http_inet_fetcher::Start(LPCTSTR lpUrl)
{
	m_dammyWnd.Watch(this, WM_GETHTTPDATA);
	m_dammyWnd.Create();

	CAutoLock lock(&m_cs);
	if( m_handleMap.size() < MAX_THREAD ) {
		ThreadContext *pContext = new ThreadContext(this, m_nContext);
		m_handleMap[m_nContext] = CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)ThreadProc, pContext,NULL,NULL);

		ATLTRACE(_T("*******Now The Thread size :%d and url: %s***********\n"), m_handleMap.size(), lpUrl);
	} else {
		ATLTRACE("**************The Thread out of size :%d******************\n", m_handleMap.size());
	}
}

inline void http_inet_fetcher::GetProc( int nIndex )
{
	CSingleFetcher *pUnit = NULL;

	while( m_bRuning && (pUnit = GetNextTask()) )
	{
		ResultParam	 param;
		pUnit->DoIt( &param );
		m_dammyWnd.Fire( WPARAM(&param), 0 );		
	}

	DoCloseHandle(nIndex);
}

inline BOOL http_inet_fetcher::IsInList(int nContext)
{
	CAutoLock lock(&m_cs);

	for( int i = m_downloadList.size() - 1; i >= 0; i-- )
	{
		if( CSingleFetcher *pFetcher = m_downloadList[i] ) {
			if( nContext == pFetcher->GetContext() ) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

inline CSingleFetcher* http_inet_fetcher::GetNextTask()
{
	CAutoLock lock(&m_cs);
	for( int i = 0; i < m_downloadList.size(); i++ )
	{
		if( CSingleFetcher * pFetcher = m_downloadList[i] ) {
			if( !pFetcher->IsInTask() ) {
				return pFetcher;
			}
		}
	}
	return NULL;
}

inline void http_inet_fetcher::DoCloseHandle(int nIndex)
{
	CAutoLock lock(&m_cs);
	if( m_handleMap.find(nIndex) != m_handleMap.end() ) {
		CloseHandle(m_handleMap[nIndex]);
		m_handleMap.erase(nIndex);
	}
}