#import "msxml3.dll"

#include "httprequest.h"
#include <algorithm>
#include <atlstr.h>
#include "json/json.h"
#include "client/helper/utf8to.h"






inline HttpRequest::HttpRequest()
{
		m_pCallBack = NULL;
}
inline int	HttpRequest::getStatus()
{
		if(m_spRequest)
		{
				long status;
				m_spRequest->get_status(&status);
				return status;
		}
		return 0;
}

inline HttpRequest::~HttpRequest(void)
{
}
inline void HttpRequest::setCallBack(HttpReqCallBack* cb)
{
		m_pCallBack = cb;
		if(m_spRequest)
		{
				m_spRequest->put_onreadystatechange((IDispatch*)m_pCallBack);
		}
}


inline void	HttpRequest::get(LPCTSTR lpUrl,BOOL bDisableCache)
{
		ATL::CString url = lpUrl;
		if(url.IsEmpty()){
				return;
		}
#ifdef _DEBUG
		m_url = lpUrl;
#endif // _DEBUG

		BOOL hr = CreateRequest();
		ATLASSERT(hr);
		if(m_spRequest)
		{							
				if(m_pCallBack){
						m_spRequest->put_onreadystatechange((IDispatch*)m_pCallBack);
				}			
				m_spRequest->open(_T("get"), (LPCTSTR)url,
								 CComVariant(TRUE), CComVariant(""), CComVariant(""));

				m_spRequest->setRequestHeader(_T("Content-Type"), 
												_T("application/x-www-form-urlencoded"));

				if (isOfficialUrl(helper::app2net(lpUrl))) {
					std::string token = GetRcAuthorizationString();
					if (!token.empty()) {
						//strHeader += net2app("Authorization: Rex " + token).c_str());
						m_spRequest->setRequestHeader(_T("Authorization"), 
							helper::net2app("Rex " + token).c_str());	
					}
			}
												
				if( bDisableCache )
					m_spRequest->setRequestHeader(_T("Cache-Control"),_T("no-cache")); 
				http_post_data params;
				CComVariant var =  CComVariant(join(params, "&").c_str());
				try{
						m_spRequest->send(var);
				}catch (...) {
				}
				
				ATLTRACE(_T("send packet : %s \r\n"), var.bstrVal);
		}
}
inline void HttpRequest::post(LPCTSTR lpUrl, std::string& data,BOOL bDisableCache)
{
	ATL::CString url = lpUrl;
	if(url.IsEmpty()){
		return;
	}
#ifdef _DEBUG
	m_url = lpUrl;
#endif // _DEBUG

	BOOL hr = CreateRequest();
	ATLASSERT(hr);
	if(m_spRequest)
	{							
		if(m_pCallBack){
			m_spRequest->put_onreadystatechange((IDispatch*)m_pCallBack);
		}			
		m_spRequest->open(_T("post"), (LPCTSTR)url,
			CComVariant(TRUE), CComVariant(""), CComVariant(""));

		m_spRequest->setRequestHeader(_T("Content-Type"), 
			_T("application/x-www-form-urlencoded"));
		if( bDisableCache )
			m_spRequest->setRequestHeader(_T("Cache-Control"),
										_T("no-cache")); 
		CComVariant var =  CComVariant(data.c_str());
		try{
			m_spRequest->send(var);
		}catch (...) {
		}
		ATLTRACE(_T("send packet : %s \r\n"), var.bstrVal);
	}
}
inline void	HttpRequest::post(LPCTSTR lpUrl, http_post_data& params)
{
		ATL::CString url = lpUrl;
		if(url.IsEmpty()){
			return;
		}

		BOOL hr = CreateRequest();
		ATLASSERT(hr);
		if(m_spRequest)
		{							
			if(m_pCallBack){
				m_spRequest->put_onreadystatechange((IDispatch*)m_pCallBack);
			}			
			m_spRequest->open(_T("post"), (LPCTSTR)url,
				CComVariant(TRUE), CComVariant(""), CComVariant(""));

			m_spRequest->setRequestHeader(_T("Content-Type"), 
										_T("application/x-www-form-urlencoded"));

			http_post_data::header_iterator hitr = params.headers.begin();
			for(; hitr != params.headers.end(); ++hitr)
			{
					m_spRequest->setRequestHeader(hitr->first.c_str(),
												  hitr->second.c_str());
			}


			CComVariant var =  CComVariant(join(params, "&").c_str());
			try{
				m_spRequest->send(var);
			}catch (...) {
			}
			ATLTRACE(_T("send packet : %s \r\n"), var.bstrVal);
		}
}


inline void	HttpRequest::post(LPCTSTR lpUrl, Json::Value& params)
{
		ATL::CString url = lpUrl;
		if(url.IsEmpty()){
			return;
		}

		BOOL hr = CreateRequest();
		ATLASSERT(hr);
		if(m_spRequest)
		{							
			if(m_pCallBack){
				m_spRequest->put_onreadystatechange((IDispatch*)m_pCallBack);
			}			
			m_spRequest->open(_T("post"), (LPCTSTR)url,
				CComVariant(TRUE), CComVariant(""), CComVariant(""));

			m_spRequest->setRequestHeader(_T("Content-Type"), 
										_T("application/json"));
			if (isOfficialUrl(helper::app2net(lpUrl))) {
					std::string token = GetRcAuthorizationString();
					if (!token.empty()) {
						//strHeader += net2app("Authorization: Rex " + token).c_str());
						m_spRequest->setRequestHeader(_T("Authorization"), 
							helper::net2app("Rex " + token).c_str());	
					}
			}
			
			try{
				Json::FastWriter writer;
				params["tool"] = "xml2";
				std::string data = writer.write(params);
				//CComVariant var = CComVariant(data.c_str());

				SAFEARRAY* psa = SafeArrayCreateVector(VT_UI1, 0, data.size());
				if (psa) {
					void* pData;
					SafeArrayAccessData(psa, &pData);
					memcpy(pData, data.c_str(), data.size());
					SafeArrayUnaccessData(psa);
				}

				VARIANT varData;
				VariantInit(&varData);
				varData.vt = VT_ARRAY | VT_UI1;
				varData.parray = psa;

				ATL::CString  strLength;
				strLength.Format(_T("%lu"), data.size());
				m_spRequest->setRequestHeader(_T("Content-Length"), strLength.AllocSysString());

				m_spRequest->send(varData);
			}catch (...) {
			}
			//ATLTRACE(_T("send packet : %s \r\n"), var.bstrVal);
		}
}


inline BOOL HttpRequest::CreateRequest()
{
		if(m_spRequest != NULL)
		{
				m_spRequest->abort();
				return TRUE;
		}

		HRESULT hr = m_spRequest.CoCreateInstance(_T("Msxml2.XMLHTTP.3.0"));
		if(SUCCEEDED(hr) && m_spRequest != NULL)
		{
				return TRUE;
		}
		ATLASSERT(FALSE);
		return FALSE;
}

inline std::string		HttpRequest::join(http_post_data& params, const char* tag)
{
				
		std::string dest; BOOL bFirst = TRUE;

		http_post_data::value_iterator vitr = params.values.begin();
		for(; vitr != params.values.end(); ++vitr)
		{	
				if(!bFirst) dest += tag;
				else bFirst = FALSE;

				if(vitr ->first.empty()){
					
						dest = vitr ->second;
				}else{
						dest += vitr ->first + "=" + vitr ->second;
				}
		}
		return dest;
}

inline int	 HttpRequest::getState()
{
	if(m_spRequest) 
	{
			long state;
			m_spRequest->get_readyState(&state);
			return state;
	}
	ATLASSERT(FALSE);
	return 0;
}
inline std::wstring	HttpRequest::getReponseText()
{
	if(m_spRequest)
	{
			CComBSTR bstr;
			m_spRequest->get_responseText(&bstr);
            if(bstr == NULL) return _T("");

			return std::wstring(bstr);
	}
	return _T("");
}
inline void HttpRequest::getReponseStream(std::string& data)
{
	CComPtr<IStream> spStream;
	char* buffer = new char[1024];
	try{
			if(m_spRequest)
			{
				CComVariant varValue;
				HRESULT hr = m_spRequest->get_responseStream(&varValue);	
				if(SUCCEEDED(hr))
				{
					spStream = (IStream*)varValue.punkVal;
					if(spStream){

						ULONG size = 0;
						do{						
							HRESULT hr = spStream->Read(buffer, 1024, &size);
							if(FAILED(hr)){
										break;
							}
							data.append(buffer, size);
						}while(size);
					}
				}
				
			}
	}catch (...) {
	}
	delete[] buffer;	
}
inline void HttpRequest::abort()
{
	if(m_spRequest) 
			m_spRequest->abort();
}
