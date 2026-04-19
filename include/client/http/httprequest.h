#pragma once
#include <string>
#include <list>
#include "atlbase.h"
#include "neohttp.h"

namespace MSXML2{
	struct IXMLHTTPRequest;
}

class HttpReqCallBack;




class HttpRequest
{
public:
	HttpRequest();
	~HttpRequest(void);
public:
	void			setCallBack(HttpReqCallBack* cb);
	void			get(LPCTSTR lpUrl,BOOL bDisableCache = FALSE);
	void			post(LPCTSTR lpUrl, http_post_data& params);
	void			post(LPCTSTR lpUrl, std::string& data,BOOL bDisableCache = FALSE);
	void			post(LPCTSTR lpUrl, Json::Value& params);


	int				getState();
	int				getStatus();
	std::wstring	getReponseText();
	void			getReponseStream(std::string& );
	void			abort();
protected:
	BOOL			CreateRequest();
	std::string	join(http_post_data& params, const char* tag);
protected:
	CComPtr<MSXML2::IXMLHTTPRequest> m_spRequest;
	HttpReqCallBack*	m_pCallBack;
#ifdef _DEBUG
#ifdef UNICODE
		std::wstring		m_url;
#else
		std::string			m_url;
#endif

#endif // _DEBUG
}; 

#include "httprequest.hpp"