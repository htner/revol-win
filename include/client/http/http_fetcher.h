#pragma once

#include "http_xml32_fetcher.h"
#include "http_curl_fetcher.h"
#include "http_inet_fetcher.h"

extern int GetRcEnvConfigInt(LPCTSTR lpSesstion,LPCTSTR lpKey,int nDefValue);
//wininet��1�tʹ��wininet����t���z�y��libcurl.dll��Ԓʹ��libcurl�� �]�Єtʹ��msxml��
static BOOL g_bUseWinInet = GetRcEnvConfigInt(_T("http"),_T("wininet"), 1);
static BOOL g_bEnableHttp = GetRcEnvConfigInt(_T("http"),_T("enable"), 1);

class http_fetcher
{
public:
	static http_fetcher* inst()
	{
		static http_fetcher g_inst;
		return &g_inst;
	}
	http_fetcher(){
		_fetcher = NULL;
	}
	~http_fetcher()
	{
		ATLASSERT( _fetcher == NULL );
	}
	void Init()
	{		
		if(_fetcher)
			return;
		if( g_bUseWinInet ) {
			_fetcher = new http_inet_fetcher;
			return;
		}

		if(http_curl_fetcher* p = new http_curl_fetcher)
		{
			if(p->isCurlLoad())
				_fetcher = p;
			else
				delete p;
		}
		if( !_fetcher )
			_fetcher  = new http_xml32_fetcher;

	}
	void UnInit()
	{
		if( _fetcher )
		{
			_fetcher->clear();
			delete _fetcher;
			_fetcher = NULL;
		}
	}
public:
	int  get(LPCTSTR url,int type, http_fetcher_cb* cb,bool bIgnoreCache = false)
	{
		if( !g_bEnableHttp ) {
			static int nCoun = 0;
			nCoun++;
			return nCoun;
		}

		if( _fetcher )
		{
			return _fetcher->get(url,type,cb,bIgnoreCache);
		}
		return -1;
	}
	int  post(LPCTSTR url, http_post_data& params, int type, http_fetcher_cb* cb)
	{
		if( !g_bEnableHttp ) {
			static int nCoun = 0;
			nCoun++;
			return nCoun;
		}

		if( _fetcher )
		{
			return _fetcher->post(url,params,type,cb);
		}
		return -1;
	}
	int  postJson(LPCTSTR url, Json::Value& params, int type, http_fetcher_cb* cb)
	{
		if( !g_bEnableHttp ) {
			static int nCoun = 0;
			nCoun++;
			return nCoun;
		}

		if( _fetcher )
		{
			return _fetcher->postJson(url,params,type,cb);
		}
		return -1;
	}
	void remove(http_fetcher_cb * cb)
	{
		if( _fetcher )
		{
			_fetcher->remove(cb);
		}
	}
	void remove(int nContext)
	{
		if( _fetcher )
		{
			_fetcher->remove(nContext);
		}
	}
protected:
	IHttp_fetcher* _fetcher;
};

#define http_fetcher_get(url,type,cb)					http_fetcher::inst()->get(url,type,cb,false)
#define http_fetcher_get2(url,type,cb,bIgnoreCache)		http_fetcher::inst()->get(url,type,cb,bIgnoreCache)

