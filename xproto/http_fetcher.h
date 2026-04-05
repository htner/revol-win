#pragma once
#include <atlbase.h>
#include "http/httpcallback.h"
#include "http/neohttp.h"
#include "timer/itimer.h"
#include <map>
#include <list>
#include "helper/utf8to.h"

class http_fetcher;
class HttpRequest;



class http_fetcher_handler : public HttpReqCallBack
{
public:
	http_fetcher_handler(http_fetcher*);
	virtual ~http_fetcher_handler();
	virtual void onResponse();
	void	get(LPCTSTR url, int context, int type);
	void	post(LPCTSTR url, http_post_data& params, int context , int type);
public:
	http_fetcher*	__fetcher;
	HttpRequest*	__httpReq;
	int				__context;
	bool			__dirty;
	int				__type;
};

struct http_fetcher_cb {
		virtual void on_http_data(int context, int type, std::string& data) = 0;
		virtual void on_http_error(int context, int type) = 0;
};

class http_fetcher : public ITimerSenseObj
{
protected:
	http_fetcher(void);
	virtual ~http_fetcher(void);

public:
	virtual void ondata(int type, int context, std::string& data);
	virtual void onerror(int type, int context);

	static http_fetcher*	inst();

	int  get(LPCTSTR url,int type, http_fetcher_cb*);
	int  post(LPCTSTR url, http_post_data& params, int type, http_fetcher_cb*);
	void revoke(int context);
	void clear();

	virtual void TimeArrive();
protected:
	std::list<http_fetcher_handler*>	__handlers;
	std::map<int, http_fetcher_cb*>     __callbacks;
	std::wstring __authkey;
	int __context;
};
