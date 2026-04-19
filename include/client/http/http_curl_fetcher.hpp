
#include "common/nsox/nulog.h"
#include "client/helper/utf8to.h"

#define WM_GETDATA	(WM_USER + 128)
#define MAX_LIMIT	10

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

extern int GetRcEnvConfigInt(LPCTSTR lpSesstion,LPCTSTR lpKey,int nDefValue);
static BOOL g_enableFiddler = GetRcEnvConfigInt(_T("http"),_T("fiddler"), 0);
 
namespace LOCK{

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

}

inline long http_curl_fetcher::writer_buff(void *data, int size, int nmemb, std::string &content)
{	
	long sizes = size * nmemb;
	string temp((char*)data,sizes);
	content += temp; 
	return sizes;
}


inline http_curl_fetcher* http_curl_fetcher:: inst()
{
	static http_curl_fetcher instance;
	return &instance;
}
inline http_curl_fetcher::	http_curl_fetcher()
{
	m_bRuning = FALSE;
	m_hThread = NULL;
	_multi_handle = NULL;
	_hModule = NULL;

	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL,szPath,MAX_PATH);
	if(TCHAR* lpSpliter = _tcsrchr(szPath,'\\')) {
		lpSpliter++;
#ifdef _DEBUG
		_tcscpy(lpSpliter,_T("libcurl_d.dll"));
#else
		_tcscpy(lpSpliter,_T("libcurl.dll"));
#endif
		_hModule = LoadLibrary(szPath);
	}

	if( _hModule )
	{
		ginit_ptr		= (curl_global_init_ptr)GetProcAddress(_hModule,"curl_global_init");
		einit_ptr		=  (curl_easy_init_ptr)GetProcAddress(_hModule,"curl_easy_init");
		setopt_ptr		= (curl_easy_setopt_ptr)GetProcAddress(_hModule,"curl_easy_setopt");
		info_ptr		= (curl_easy_getinfo_ptr)GetProcAddress(_hModule,"curl_easy_getinfo");
		clean_ptr		= (curl_easy_cleanup_ptr)GetProcAddress(_hModule,"curl_easy_cleanup");
		gclean_ptr		= (curl_global_cleanup_ptr)GetProcAddress(_hModule,"curl_global_cleanup");

		multinit_ptr    = (curl_multi_init_ptr)GetProcAddress(_hModule, "curl_multi_init");
		multadd_ptr		= (curl_multi_add_handle_ptr)GetProcAddress(_hModule, "curl_multi_add_handle");
		multremove_ptr	= (curl_multi_remove_handle_ptr)GetProcAddress(_hModule, "curl_multi_remove_handle");
		multtimeout_ptr	= (curl_multi_timeout_ptr)GetProcAddress(_hModule, "curl_multi_timeout"); 
		multfdset_ptr	= (curl_multi_fdset_ptr)GetProcAddress(_hModule, "curl_multi_fdset");
		multperform_ptr = (curl_multi_perform_ptr)GetProcAddress(_hModule, "curl_multi_perform");
		multreadinfo_ptr = (curl_multi_info_read_ptr)GetProcAddress(_hModule, "curl_multi_info_read");
		multcleanup_ptr	= (curl_multi_cleanup_ptr)GetProcAddress(_hModule, "curl_multi_cleanup");

		multcleanup_ptr	= (curl_multi_cleanup_ptr)GetProcAddress(_hModule, "curl_multi_cleanup");
		slist_append_ptr = (curl_slist_append_ptr)GetProcAddress(_hModule, "curl_slist_append");

		_multi_handle	= multinit_ptr();

		m_DammyWnd.Create();
		m_DammyWnd.Watch(this, WM_GETDATA);

		if( !ginit_ptr || ginit_ptr(CURL_GLOBAL_DEFAULT) != CURLE_OK )
		{
			FreeLibrary(_hModule);
			_hModule = NULL;
		}
	}
	InitializeCriticalSection(&_cs);
};

inline bool  http_curl_fetcher::isCurlLoad()
{
	return (_hModule != NULL);
}

inline http_curl_fetcher::~http_curl_fetcher()
{
	m_DammyWnd.Destroy();
	m_DammyWnd.Revoke();

	DeleteCriticalSection(&_cs);
	if( _hModule )
	{
		gclean_ptr();
		FreeLibrary(_hModule);
		_hModule = NULL;
	}
}
inline int  http_curl_fetcher::NextCount() {
	static int count = 0;
	return ++count;
}

inline	int http_curl_fetcher::PushJsonTask(LPCTSTR url, int type, http_fetcher_cb* cb, Json::Value &root) {
	int id = NextCount();
	Item* item = new Item;
	item->_url = FilterString(url);
	item->_type = type;
	item->_cb = cb;
	item->_Itemid = id;
	item->_inHandler = FALSE;

	CURLcode code;

	USES_CONVERSION;
	std::string urlA = CT2A((item->_url).c_str());
	if (CURL *curl = einit_ptr()) {
		try {
			root["type"] = "curl";
			Json::FastWriter writer;
			item->_postData = writer.write(root);
		}
		catch (...) {

		}
		if (g_enableFiddler) {
			code = setopt_ptr(curl, CURLOPT_PROXY, "localhost:8888");
		}
		code = setopt_ptr(curl, CURLOPT_DNS_CACHE_TIMEOUT, -1);
		code = setopt_ptr(curl, CURLOPT_URL, urlA.c_str());
		code = setopt_ptr(curl, CURLOPT_POSTFIELDS, item->_postData.c_str());
		code = setopt_ptr(curl, CURLOPT_POSTFIELDSIZE, item->_postData.length());
		code = setopt_ptr(curl, CURLOPT_WRITEFUNCTION, writer_buff);
		code = setopt_ptr(curl, CURLOPT_WRITEDATA, &item->_content);
		code = setopt_ptr(curl, CURLOPT_POST, 1);
		code = setopt_ptr(curl, CURLOPT_VERBOSE, 1L);
		code = setopt_ptr(curl, CURLOPT_FOLLOWLOCATION, 1);
		code = setopt_ptr(curl, CURLOPT_COOKIEFILE, "");
		code = setopt_ptr(curl, CURLOPT_SSL_VERIFYPEER, 0L);

		// Set Content-Type header
		struct curl_slist *headers = NULL;
		headers = slist_append_ptr(headers, "Content-Type: application/json");
		if (isOfficialUrl(helper::app2net(url))) {
			std::string token = GetRcAuthorizationString();
			if (!token.empty()) {
				headers = slist_append_ptr(headers, ("Authorization: Rex " + token).c_str());	
			}
		}
		code = setopt_ptr(curl, CURLOPT_HTTPHEADER, headers);
	}
	{
		LOCK::CAutoLock lock(&_cs);
		m_ItemList.push_back(item);
	}
	return id;
}

inline	int http_curl_fetcher::PushTask(LPCTSTR url, int type, http_fetcher_cb* cb, int flag, http_post_data* post)
{
	//static int count = 0;

	int id  = NextCount();
	Item* item = new Item;
	item->_url = FilterString(url);
	item->_type = type;
	item->_cb = cb;
	item->_Itemid = id;
	item->_inHandler = FALSE;
	
	CURLcode code;

	USES_CONVERSION;
	std::string urlA = CT2A((item->_url).c_str());	
	if( CURL *curl = einit_ptr() ) {
		if (flag == http_curl_fetcher::HTTP_GET)
		{
			if( g_enableFiddler ) {
				code = setopt_ptr(curl, CURLOPT_PROXY, "localhost:8888");
			}
			code = setopt_ptr(curl, CURLOPT_DNS_CACHE_TIMEOUT, -1);
			code = setopt_ptr(curl, CURLOPT_VERBOSE, 1L);
			code = setopt_ptr(curl, CURLOPT_URL, urlA.c_str());
			code = setopt_ptr(curl, CURLOPT_FOLLOWLOCATION, 1);
			code = setopt_ptr(curl, CURLOPT_WRITEFUNCTION, writer_buff);
			code = setopt_ptr(curl, CURLOPT_WRITEDATA, &item->_content);
			code = setopt_ptr(curl, CURLOPT_SSL_VERIFYPEER, 0L);

			// Set Content-Type header
			
			if (isOfficialUrl(helper::app2net(url))) {
				std::string token = GetRcAuthorizationString();
				if (!token.empty()) {
					struct curl_slist *headers = NULL;
					headers = slist_append_ptr(headers, ("Authorization: Rex " + token).c_str());
					code = setopt_ptr(curl, CURLOPT_HTTPHEADER, headers);
				}
			}
			
		}
		else if(flag == http_curl_fetcher::HTTP_POST_JSON)
		{   
			http_post_data::value_iterator it=		post->values.begin();
			http_post_data::value_iterator itend =	post->values.end();
			for (; it!=itend; it++)
			{
				item->_postData += it->first + "=" + it->second + "&";
			}

			if( g_enableFiddler ) {
				code = setopt_ptr(curl, CURLOPT_PROXY, "localhost:8888");
			}
			code = setopt_ptr(curl, CURLOPT_DNS_CACHE_TIMEOUT, -1);
			code = setopt_ptr(curl, CURLOPT_URL, urlA.c_str());  
			code = setopt_ptr(curl, CURLOPT_POSTFIELDS, item->_postData.c_str());   
			code = setopt_ptr(curl, CURLOPT_WRITEFUNCTION, writer_buff);   
			code = setopt_ptr(curl, CURLOPT_WRITEDATA, &item->_content);
			code = setopt_ptr(curl, CURLOPT_POST, 1);   
			code = setopt_ptr(curl, CURLOPT_VERBOSE, 1L);      
			code = setopt_ptr(curl, CURLOPT_FOLLOWLOCATION, 1);   
			code = setopt_ptr(curl, CURLOPT_COOKIEFILE, "");   
			code = setopt_ptr(curl, CURLOPT_SSL_VERIFYPEER, 0L);

			struct curl_slist *headers = NULL;
			headers = slist_append_ptr(headers, "Content-Type: application/json");
			if (isOfficialUrl(helper::app2net(url))) {
				std::string token = GetRcAuthorizationString();
				if (!token.empty()) {
					struct curl_slist *headers = NULL;
					headers = slist_append_ptr(headers, ("Authorization: Rex " + token).c_str());
					
				}
			}
			code = setopt_ptr(curl, CURLOPT_HTTPHEADER, headers);
		}

		item->_curl = curl;
		{
			LOCK::CAutoLock lock(&_cs);
			m_ItemList.push_back(item);
		}
	} else {
		ATLASSERT(FALSE);
	}

	return id;
}

inline	void http_curl_fetcher:: StartDownload()
{	
	if( m_hThread )return;
	
	m_bRuning = TRUE;
	m_hThread = CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)ThreadProc,this,NULL,NULL);

}

inline  DWORD http_curl_fetcher:: ThreadProc(LPVOID lpParam)
{
	http_curl_fetcher* lpThis = (http_curl_fetcher*)lpParam;
	return lpThis->GetProc();
}

inline	DWORD http_curl_fetcher:: GetProc()
{	

	int nAppend = 0;
	while( m_bRuning ) 
	{	
		Sleep(10);

		int nInitTotal = AppendTask( MAX_LIMIT - nAppend );
		if( 0 == nInitTotal && 0 == nAppend ) {
			continue;
		}

		int nRuning = 0;
		CURLMcode code = DoPerform(nRuning, nAppend);

		while( nRuning > 0 ) 
		{
			struct timeval timeout;

			fd_set fdread;
			fd_set fdwrite;
			fd_set fdexcep;
			int maxfd = -1;

			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdexcep);

			{
				LOCK::CAutoLock lock(&_cs);
				/* set a suitable timeout to play around with */
				long curl_timeo = -1;			
				timeout.tv_sec = 1;
				timeout.tv_usec = 0;
				multtimeout_ptr(_multi_handle, &curl_timeo);
				if(curl_timeo >= 0) {
					timeout.tv_sec = curl_timeo / 1000;
					if(timeout.tv_sec > 1)
						timeout.tv_sec = 1;
					else
						timeout.tv_usec = (curl_timeo % 1000) * 1000;
				}

				/* get file descriptors from the transfers */
				multfdset_ptr(_multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
			}

			//߀�]���������O��
			if( -1 == maxfd ) {
				//�ęn���h100ms����
				Sleep(100);
				code = DoPerform(nRuning, nAppend);
				continue;
			}

			int rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

			if( rc != -1 ) {
				code = DoPerform(nRuning, nAppend);
			}
		}

		PeekTask();
	}

	CloseHandle(m_hThread);
	m_hThread = NULL;

	return 0;
}

inline CURLMcode http_curl_fetcher::DoPerform(int &nRuning, int &nAppend)
{
	nAppend = 0;
	int nPreRuning = nRuning;
	CURLMcode code = CURLM_CALL_MULTI_PERFORM;

	{
		LOCK::CAutoLock lock(&_cs);
		try
		{
			do 
			{
				code = multperform_ptr(_multi_handle, &nRuning);
			} while ( CURLM_CALL_MULTI_PERFORM == code );
		}
		catch (...)
		{
			return code;
		}

	}

	if( nRuning > MAX_LIMIT ) {
		ATLASSERT(FALSE);
	}

	if( nRuning >= 0 && nPreRuning > nRuning ) {
		//֪ͨ�ѽ�̎����
		int nPeek = PeekTask();
	}

	//���������?
	if( nRuning < MAX_LIMIT ) {
		nAppend = AppendTask( MAX_LIMIT - nRuning );
	} else {
		ATLTRACE("************Extra Append Task: Oh No Full********************\n");
	}

	return code;
}

inline int http_curl_fetcher::PeekTask()
{
	CURLMsg *msg = NULL;
	int msgs_left;
	int nDone = 0;
	while ( (msg = multreadinfo_ptr(_multi_handle, &msgs_left)) ) {
		if (msg->msg == CURLMSG_DONE) {
			m_DammyWnd.Fire((WPARAM)0,(LPARAM)msg->easy_handle);
			nDone++;
		}
	}
	return nDone;
}

inline int http_curl_fetcher::AppendTask(int nMax)
{
	LOCK::CAutoLock lock(&_cs);

	ItemList::iterator itr = m_ItemList.begin();
	ItemList::iterator itrEnd = m_ItemList.end();
	int nReal = 0;
	for( ; itr != itrEnd && nReal < nMax; ++itr )
	{
		Item *item = *itr;
		if( !item->_inHandler ) {
			multadd_ptr(_multi_handle, item->_curl);
			item->_inHandler = TRUE;
			nReal++;
		}
	}
	return nReal;
}

inline void http_curl_fetcher::OnDamyyWndEvent(HWND hwnd,UINT nMsg,WPARAM wParam,LPARAM lParam)
{
	ATLASSERT(nMsg == WM_GETDATA);

	if( CURL* pCurl = (CURL*)lParam ) {
		ItemList::iterator itr = m_ItemList.begin();
		ItemList::iterator itrEnd = m_ItemList.end();
		for( ; itr != itrEnd; ++itr )
		{
			Item *pItem = *itr;

			if( pItem->_curl == pCurl ) {
				m_ItemList.erase(itr);

				//�@ȡ��B
				int status = 0;
				info_ptr(pCurl, CURLINFO_RESPONSE_CODE , &status);

				double dTotalLenth, dDownLoadLength;
				dTotalLenth = dDownLoadLength = 0;
				info_ptr(pCurl,CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dTotalLenth);
				info_ptr(pCurl,CURLINFO_SIZE_DOWNLOAD, &dDownLoadLength);
				long lTotal = (long)dTotalLenth;
				long lDownLoad = (long)dDownLoadLength;
				//������d�ĺ͌��H��С����Ȅt�J�����dʧ��
				if( lTotal != -1 && lDownLoad != -1 && lTotal != lDownLoad ) {
					status = -1;
					nsox::xlog(NsoxError, "Download failed:url:%s--TotalSize:%ld; DownLoadSize:%ld", 
						helper::app2net(pItem->_url).c_str(), lTotal, lDownLoad);
					ATLASSERT(FALSE);
				}
				if( status == 200 ) {
					if(pItem->_cb)
					{
						pItem->_cb->on_http_data(pItem->_Itemid,pItem->_type, pItem->_content);
					}		
				} else {
					if(pItem->_cb)
					{
						pItem->_cb->on_http_error(pItem->_Itemid,pItem->_type, pItem->_content, status);
					}			
				}

				//ጷž��?
				multremove_ptr(_multi_handle, pCurl);
				clean_ptr(pCurl);

				delete pItem;
				break;
			}
		}
	}
}

inline xstring http_curl_fetcher::FilterString(const xstring &strUrl)
{
	xstring url;
	for( int i = 0; i < strUrl.length(); i++ )
	{
		//���@�������ַ���url��fiddlerץȡ����
		if( 10 == strUrl[i] ) {
			url += L'';
		} else {
			url += strUrl[i];
		}
	}
	return url;
}

inline int http_curl_fetcher:: get(LPCTSTR url,int type, http_fetcher_cb* cb,bool bIgnoreCache)
{
	if (_hModule)
	{
		int id = PushTask(url,type,cb,HTTP_GET);
		StartDownload();
		return id;
	}
	return -1;

}

inline int http_curl_fetcher:: post(LPCTSTR url, http_post_data& params, int type, http_fetcher_cb* cb)
{
	if (_hModule)
	{
		int id = PushTask(url,type,cb,HTTP_POST,&params);
		StartDownload();
		return id;	
	}
	return -1;
}

inline int http_curl_fetcher::postJson(LPCTSTR url, Json::Value& params, int type, http_fetcher_cb* cb) {
	if (_hModule)
	{
		int id = PushJsonTask(url, type, cb, params);
		StartDownload();
		return id;
	}
	return -1;
}

inline void http_curl_fetcher:: clear()
{
	//�˳�����
	m_bRuning = FALSE; 
	if( m_hThread ) {
		int i = 0;
		while(i < 20 && m_hThread)
		{
			Sleep(10);
			i++;
		}
		TerminateThread ( m_hThread, 0 );
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	ItemList::iterator itr = m_ItemList.begin();
	for( ; itr != m_ItemList.end();  )
	{	
		if( Item *pItem = *itr ) {
			multremove_ptr(_multi_handle, pItem->_curl);
			clean_ptr(pItem->_curl);
			delete pItem;
		}

		itr = m_ItemList.erase(itr);
	}

	multcleanup_ptr(_multi_handle);

}

inline void http_curl_fetcher:: remove(http_fetcher_cb* cb)
{
	LOCK::CAutoLock lock(&_cs);

	ItemList::iterator itr = m_ItemList.begin();

	for (; itr != m_ItemList.end(); )
	{
		Item *pItem = *itr;
		if( pItem->_cb == cb )
		{
			multremove_ptr(_multi_handle, pItem->_curl);
			clean_ptr(pItem->_curl);
			itr = m_ItemList.erase(itr);			
			delete pItem;
		}else
			itr++;

	}

}

inline void http_curl_fetcher:: remove(int nContext)
{
	LOCK::CAutoLock lock(&_cs);
	ItemList::iterator itr = m_ItemList.begin();
	ItemList::iterator itrEnd = m_ItemList.end();
	for (; itr != itrEnd; itr++)
	{
		Item *pItem = *itr;
		if( pItem->_Itemid == nContext ) {
			multremove_ptr(_multi_handle, pItem->_curl);
			clean_ptr(pItem->_curl);
			m_ItemList.erase(itr);	
			delete pItem;
			break;
		}
	}

}




