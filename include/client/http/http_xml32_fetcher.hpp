#include "httprequest.h"
#include "http_xml32_fetcher.h"
#include "timer/ExtraTimer.h"

inline http_fetcher_handler::http_fetcher_handler(http_xml32_fetcher* f)
{
	__fetcher = f;
	__httpReq = NULL;
	__context = 0;
	__dirty	  = false;
}
inline http_fetcher_handler::~http_fetcher_handler()
{
	if(__httpReq){
			delete __httpReq;
	}	
}
inline bool	http_fetcher_handler::abort()
{
	if(__httpReq){
			__httpReq->abort();
	}
	return true;
}
inline void http_fetcher_handler::get(LPCTSTR url, int context, int type,BOOL bDisableCache)
{
	__httpReq = new HttpRequest;
	__httpReq->setCallBack(this);

	__context = context;
	__type	  = type;
	try{
			__httpReq->get(url,bDisableCache);	
	}catch(...){
			__fetcher->onerror(__context, __type,-1);
			__dirty = true;
	}
}

inline void	http_fetcher_handler::post(LPCTSTR url,http_post_data& params, int context, int type)
{
	__httpReq = new HttpRequest;
	__httpReq->setCallBack(this);

	__context = context;
	__type	  = type;
	try{
		__httpReq->post(url, params);	
	}catch(...){
		__fetcher->onerror(__context, __type,-1);
		__dirty = true;
	}
}

inline void	http_fetcher_handler::postJson(LPCTSTR url, Json::Value& params, int context, int type)
{
	__httpReq = new HttpRequest;
	__httpReq->setCallBack(this);

	__context = context;
	__type	  = type;
	try{
		__httpReq->post(url, params);	
	}catch(...){
		__fetcher->onerror(__context, __type,-1);
		__dirty = true;
	}
}

inline void http_fetcher_handler::onResponse()
{
	if(__httpReq->getState() == 4){
			int status = __httpReq->getStatus();
			if(status == 200){
					//ATLTRACE(__httpReq->getReponseText().c_str());
					std::string data;
					__httpReq->getReponseStream(data);
					__fetcher->ondata(__context, __type, data);
			}else{
					__fetcher->onerror(__context, __type,status);
			}
			__dirty = true;
	}
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
inline http_xml32_fetcher::http_xml32_fetcher(void)
{
		__bTraceing = false;
		__context = 1000;
}
inline http_xml32_fetcher::~http_xml32_fetcher(void)
{
	ATLASSERT(__handlers.empty() &&  __callbacks.empty());
	clear();
}
inline http_xml32_fetcher* http_xml32_fetcher::inst()
{
		static http_xml32_fetcher instance;
		return &instance;
}

inline void http_xml32_fetcher::StartTrace()
{	
	if( !__bTraceing )
	{
		__bTraceing = true;
		CTimer::Instance()->RegisterListener(this, 1000 * 10);
	}
}

inline void http_xml32_fetcher::StopTrace()
{
	if( __bTraceing )
	{
		__bTraceing = false;
		CTimer::Instance()->UnRegisterListener(this);
	}
}

inline void http_xml32_fetcher::onerror(int context,  int type,int status)
{
	std::map<int, http_fetcher_cb*>::iterator itr = __callbacks.find(context);
	if(itr != __callbacks.end() ){
		if(itr->second){
			std::string data;
			itr->second->on_http_error(context, type, data, status);
		}
	}
}

inline void http_xml32_fetcher::ondata(int context, int type, std::string& data)
{
	std::map<int, http_fetcher_cb*>::iterator itr = __callbacks.find(context);
	if(itr != __callbacks.end() ){
		if( itr->second )
		{
			itr->second->on_http_data(context, type,data);
		}
	}

}


inline int http_xml32_fetcher::get(LPCTSTR url, int type, http_fetcher_cb* cb,bool bIgnoreCache)
{
		http_fetcher_handler* h = new http_fetcher_handler(this);

		__context++;
		__callbacks[__context] = cb;
		__handlers[__context] = h;

		std::wstring strNew = url;
		if( bIgnoreCache )
		{			
			strNew += _T("&AAnothingTickBB=");
			DWORD dw = GetTickCount();
			wchar_t buf[20];
			std::wstring x =  std::wstring(_itow(dw, buf, 10));
			strNew += x;
		}
		url = strNew.c_str();

		h->get(url,  __context, type,bIgnoreCache);
		StartTrace();
		return __context;
}


inline int http_xml32_fetcher::post(LPCTSTR url, http_post_data& params,int type,  http_fetcher_cb* cb)
{
	http_fetcher_handler* h = new http_fetcher_handler(this);

	__context++;
	__callbacks[__context] = cb;
	__handlers[__context] = h;
	h->post(url,  params, __context, type);
	StartTrace();
	return __context;
}

 inline int http_xml32_fetcher::postJson(LPCTSTR url, Json::Value& params, int type, http_fetcher_cb* cb) {
	http_fetcher_handler* h = new http_fetcher_handler(this);

	__context++;
	__callbacks[__context] = cb;
	__handlers[__context] = h;
	h->postJson(url,  params, __context, type);
	StartTrace();
	return __context;
 }

inline void http_xml32_fetcher::TimeArrive()
{
		//std::list<http_fetcher_handler*>::iterator itr = __handlers.begin();
		//while(itr != __handlers.end()){
		//		std::list<http_fetcher_handler*>::iterator tmp = itr ++;
		//		if((*tmp)->__dirty){
		//				delete (*tmp);
		//				__handlers.erase(tmp);
		//		}
		//}

	ATLTRACE("=======> http_fetcher handlers size : %d <===========\n", __handlers.size());

	std::map<int, http_fetcher_handler*>::iterator itr = __handlers.begin();
	while(itr != __handlers.end())
	{
			std::map<int, http_fetcher_handler*>::iterator tmp = itr ++;

			if( tmp->second->__dirty){

				std::map<int,http_fetcher_cb*>::iterator itrHandler2 = __callbacks.find(tmp->first);
				if(itrHandler2 != __callbacks.end()) {
					__callbacks.erase(itrHandler2);
				}

				delete tmp->second;
				__handlers.erase(tmp);
			}
	}

	if( __handlers.size() <= 0 )
		StopTrace();
}

inline void http_xml32_fetcher::clear()
{
	//std::list<http_fetcher_handler*>::iterator itr = __handlers.begin();
	//for(; itr != __handlers.end(); ++itr){			
	//	if((*itr) &&(*itr)->abort()){
	//		delete (*itr);
	//	}
	//}
	std::map<int, http_fetcher_handler*>::iterator itr = __handlers.begin();
	for(; itr != __handlers.end(); ++itr){	
		http_fetcher_handler *pHandler = itr->second;
		if(pHandler &&pHandler->abort()){
			delete pHandler;
		}
	}
	__handlers.clear();
	__callbacks.clear();
	StopTrace();
}




inline void http_xml32_fetcher::remove(int nContext)
{
	std::map<int,http_fetcher_handler*>::iterator itrHandler1 = __handlers.find(nContext);
	if(itrHandler1 != __handlers.end()) {
		itrHandler1->second->abort();
		delete itrHandler1->second;
		__handlers.erase(itrHandler1);
	}

	std::map<int,http_fetcher_cb*>::iterator itrHandler2 = __callbacks.find(nContext);
	if(itrHandler2 != __callbacks.end()) {
		__callbacks.erase(itrHandler2);
	}

	if( __handlers.size() <=0 && __callbacks.size()<=0 )
		StopTrace();
}


inline void http_xml32_fetcher::remove(http_fetcher_cb *phttp)
{
	int context;
	std::map<int, http_fetcher_cb*>::iterator itrCb = __callbacks.begin();
	for(; itrCb != __callbacks.end(); )
	{
		if(itrCb->second == phttp) {
			context = itrCb->first;
			itrCb = __callbacks.erase(itrCb);
			//clear handlers;
			std::map<int,http_fetcher_handler*>::iterator itrHandler = __handlers.find(context);
			if(itrHandler != __handlers.end()) {
				itrHandler->second->abort();
				delete itrHandler->second;
				__handlers.erase(itrHandler);
			}

		} else {
			++itrCb;
		}
	}
	if( __handlers.size() <=0 && __callbacks.size()<=0 )
		StopTrace();
}

