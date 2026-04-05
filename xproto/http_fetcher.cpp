#include "http/httprequest.h"
#include "http_fetcher.h"
#include "timer/ExtraTimer.h"

http_fetcher_handler::http_fetcher_handler(http_fetcher* f)
{
	__fetcher = f;
	__httpReq = NULL;
	__context = 0;
	__dirty	  = false;
}
http_fetcher_handler::~http_fetcher_handler()
{
	if(__httpReq){
			delete __httpReq;
	}	
}
void http_fetcher_handler::get(LPCTSTR url, int context, int type)
{
	__httpReq = new HttpRequest;
	__httpReq->setCallBack(this);

	__context = context;
	__type	  = type;
	try{
			__httpReq->get(url);	
	}catch(...){
			__fetcher->onerror(__context, __type);
			__dirty = true;
	}
}

void	http_fetcher_handler::post(LPCTSTR url,http_post_data& params, int context, int type)
{
	__httpReq = new HttpRequest;
	__httpReq->setCallBack(this);

	__context = context;
	__type	  = type;
	try{
		__httpReq->post(url, params);	
	}catch(...){
		__fetcher->onerror(__context, __type);
		__dirty = true;
	}
}
void http_fetcher_handler::onResponse()
{
	if(__httpReq->getState() == 4){
			int status = __httpReq->getStatus();
			if(status == 200){
					ATLTRACE(__httpReq->getReponseText().c_str());
					std::string data;
					__httpReq->getReponseStream(data);
					__fetcher->ondata(__context, __type, data);
			}else{
					__fetcher->onerror(__context, __type);
			}
			__dirty = true;
	}
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
http_fetcher::http_fetcher(void)
{
		__context = 1000;
}
http_fetcher::~http_fetcher(void)
{
		clear();
}
http_fetcher* http_fetcher::inst()
{
		static http_fetcher instance;
		return &instance;
}
void http_fetcher::onerror(int context,  int type)
{
		std::map<int, http_fetcher_cb*>::iterator itr = __callbacks.find(context);
		if(itr != __callbacks.end()){
			itr->second->on_http_error(context, type);
		}
}
void http_fetcher::ondata(int context, int type, std::string& data)
{
		std::map<int, http_fetcher_cb*>::iterator itr = __callbacks.find(context);
		if(itr != __callbacks.end()){
			itr->second->on_http_data(context, type,data);
		}
}
int http_fetcher::get(LPCTSTR url, int type, http_fetcher_cb* cb)
{
		http_fetcher_handler* h = new http_fetcher_handler(this);
		
		__callbacks[++__context] = cb;
		__handlers.push_back(h);

		h->get(url,  __context, type);
		return __context;
}

int http_fetcher::post(LPCTSTR url, http_post_data& params,int type,  http_fetcher_cb* cb)
{
	http_fetcher_handler* h = new http_fetcher_handler(this);
	
	__callbacks[++__context] = cb;
	__handlers.push_back(h);

	h->post(url,  params, __context, type);
	return __context;
}

void http_fetcher::TimeArrive()
{
		std::list<http_fetcher_handler*>::iterator itr = __handlers.begin();
		while(itr != __handlers.end()){
				std::list<http_fetcher_handler*>::iterator tmp = itr ++;
				if((*tmp)->__dirty){
						delete (*tmp);
						__handlers.erase(tmp);
				}
		}
}

void http_fetcher::clear()
{
	std::list<http_fetcher_handler*>::iterator itr = __handlers.begin();
	for(; itr != __handlers.end(); ++itr){		
		if((*itr)->__dirty){
			delete (*itr);
		}
	}
	__handlers.clear();
}

