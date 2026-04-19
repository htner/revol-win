#pragma once

#include <atlstr.h>
#include "common/xstring.h"
#include <vector>
#include "helper/DammyWnd.h"
#include "helper/utf8to.h"
#include <atlconv.h>
#include "neohttp.h"
#include "http_fetcher_cb.h"
#include <curl/curl.h>
#include <json/json.h>

using namespace std;



class http_curl_fetcher : public IHttp_fetcher
						, public IDammyWndEvent
{
	enum HTTP_ACTION{HTTP_GET=0,HTTP_POST,HTTP_POST_JSON};

	typedef CURLcode (*curl_global_init_ptr)(long flags);
	typedef CURL *(*curl_easy_init_ptr)(void);
	typedef CURLcode (*curl_easy_setopt_ptr)(CURL *curl, CURLoption option, ...);
	typedef CURLcode (*curl_easy_perform_ptr)(CURL *curl);
	typedef CURLcode (*curl_easy_getinfo_ptr)(CURL *curl, CURLINFO info, ...);
	typedef CURLcode (*curl_easy_getinfo_ptr)(CURL *curl, CURLINFO info, ...);
	typedef void (*curl_easy_cleanup_ptr)(CURL *curl);
	typedef void (*curl_global_cleanup_ptr)(void);		

	typedef CURLM *(*curl_multi_init_ptr)(void);
	typedef CURLMcode (*curl_multi_add_handle_ptr)(CURLM *multi_handle, CURL *easy_handle);
	typedef CURLMcode (*curl_multi_remove_handle_ptr)(CURLM *multi_handle, CURL *curl_handle);
	typedef CURLMcode (*curl_multi_timeout_ptr)(CURLM *multi_handle, long *milliseconds);
	typedef CURLMcode (*curl_multi_fdset_ptr)(CURLM *multi_handle, fd_set *read_fd_set, fd_set *write_fd_set, fd_set *exc_fd_set, int *max_fd);
	typedef CURLMcode (*curl_multi_perform_ptr)(CURLM *multi_handle, int *running_handles);
	typedef	CURLMsg *(*curl_multi_info_read_ptr)(CURLM *multi_handle, int *msgs_in_queue);
	typedef CURLMcode (*curl_multi_cleanup_ptr)(CURLM *multi_handle);
	typedef struct curl_slist * (*curl_slist_append_ptr)(struct curl_slist *list, const char *data);
private:
	
	struct Item
	{
		int                 _Itemid;
		xstring             _url;
		std::string			_content;
		std::string			_postData;
		http_fetcher_cb*    _cb;
		CURL*				_curl;
		int                 _type;
		BOOL				_inHandler;
	};
	typedef std::vector<Item*> ItemList;

protected:
	BOOL				   m_bRuning;
	ItemList               m_ItemList;
	HANDLE	               m_hThread;
	CRITICAL_SECTION	   _cs; 
	CDammyWnd              m_DammyWnd;
	HMODULE				  _hModule;
	CURLM*				  _multi_handle;
	curl_global_init_ptr   ginit_ptr ;
	curl_easy_init_ptr	   einit_ptr ;
	curl_easy_setopt_ptr   setopt_ptr ;
	curl_easy_getinfo_ptr  info_ptr;
	curl_easy_cleanup_ptr  clean_ptr;
	curl_global_cleanup_ptr   gclean_ptr;

	curl_multi_init_ptr				multinit_ptr;
	curl_multi_add_handle_ptr		multadd_ptr;
	curl_multi_remove_handle_ptr	multremove_ptr;
	curl_multi_timeout_ptr			multtimeout_ptr;
	curl_multi_fdset_ptr			multfdset_ptr;
	curl_multi_perform_ptr			multperform_ptr;
	curl_multi_info_read_ptr		multreadinfo_ptr;
	curl_multi_cleanup_ptr			multcleanup_ptr;

	curl_slist_append_ptr slist_append_ptr;

public:	
	static http_curl_fetcher*	inst();
	http_curl_fetcher();
	virtual ~http_curl_fetcher();	
public:
	virtual int   get(LPCTSTR url,int type, http_fetcher_cb* cb,bool bIgnoreCache = false);
	virtual int   post(LPCTSTR url, http_post_data& params, int type, http_fetcher_cb* cb);
	virtual int   postJson(LPCTSTR url, Json::Value& params, int type, http_fetcher_cb* cb);
	//virtual int  postJson(LPCTSTR url, Json::Value& params, http_fetcher_cb*) = 0;
	virtual void  clear();	
	virtual void  remove(http_fetcher_cb* cb);
	virtual void  remove(int nContext);
public:
	bool  isCurlLoad();
protected:
	static long writer_buff(void *data, int size, int nmemb, string &content);			
private:
	int    PushTask(LPCTSTR url,int type, http_fetcher_cb* cb,int flag,http_post_data* post = NULL);
	int    PushJsonTask(LPCTSTR url, int type, http_fetcher_cb* cb, Json::Value&);
	void   StartDownload();
	static DWORD ThreadProc(LPVOID lpParam);
	DWORD  GetProc();
	CURLMcode DoPerform(int &nRuning, int &nAppend);
	int PeekTask();
	int AppendTask(int nMax);
	virtual void OnDamyyWndEvent(HWND hwnd,UINT nMsg,WPARAM wParam,LPARAM lParam);
	xstring FilterString(const xstring &strUrl);

	int NextCount();
};

#include "http_curl_fetcher.hpp"


