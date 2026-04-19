#pragma once
#include <atlbase.h>
#include "httpcallback.h"
#include "neohttp.h"
#include "timer/itimer.h"
#include "http_fetcher_cb.h"
#include <map>
#include <list>

class http_xml32_fetcher;
class HttpRequest;



class http_fetcher_handler : public HttpReqCallBack
{
public:
	http_fetcher_handler(http_xml32_fetcher*);
	virtual ~http_fetcher_handler();
	virtual void onResponse();
	void	get(LPCTSTR url, int context, int type,BOOL bDisableCache = FALSE);
	void	post(LPCTSTR url, http_post_data& params, int context , int type);
	void	postJson(LPCTSTR url, Json::Value& params, int context, int type);
	bool	abort();
public:
	http_xml32_fetcher*	__fetcher;
	HttpRequest*	__httpReq;
	int				__context;
	bool			__dirty;
	int				__type;
};


class http_xml32_fetcher : public IHttp_fetcher
						 , public ITimerSenseObj
{
public:
	http_xml32_fetcher(void);
	virtual ~http_xml32_fetcher(void);
public:
	virtual int  get(LPCTSTR url,int type, http_fetcher_cb*,bool bIgnoreCache = false);	
	virtual int  post(LPCTSTR url, http_post_data& params, int type, http_fetcher_cb*);
	virtual int  postJson(LPCTSTR url, Json::Value& params, int type, http_fetcher_cb* cb);
	virtual void clear();
	virtual void remove(http_fetcher_cb *);
	virtual void remove(int nContext);

public:
	virtual void ondata(int type, int context, std::string& data);
	virtual void onerror(int type, int context,int status);

	static http_xml32_fetcher*	inst();
	void revoke(int context);	

	void StartTrace();
	void StopTrace();

	virtual void TimeArrive();
protected:
	std::map<int, http_fetcher_cb*>      __callbacks;
	std::map<int, http_fetcher_handler*> __handlers;
	std::wstring __authkey;
	int __context;
	bool	__bTraceing;

};

#define http_xml32_fetcher_get(url,type,cb)					http_xml32_fetcher::inst()->get(url,type,cb)
#define http_xml32_fetcher_get2(url,type,cb,bIgnoreCache)		http_xml32_fetcher::inst()->get(url,type,cb,bIgnoreCache)
#define http_xml32_fetcher_post(url,params,type,cb)					http_xml32_fetcher::inst()->post(url,params,type,cb)

#include "http_xml32_fetcher.hpp"
