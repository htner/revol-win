#pragma once
#include <string>

struct http_fetcher_cb;

typedef struct tagResult
{
	http_fetcher_cb*	_pCallBack;
	int					_nType;
	int					_nContext;
	std::string			_data;
	BOOL				_bHttps;
	DWORD				_dwStatusCode;	
}ResultParam;	

class CSingleFetcher
{
public:
	CSingleFetcher( const std::wstring &strUrl, int nType, int nContext, http_fetcher_cb* pCallback,  
		BOOL bGet, const std::string &strPostData = "", const std::string& jsonData = "");
	~CSingleFetcher();

	void DoIt( ResultParam *pResult );
	int	GetContext();
	BOOL IsInTask();
protected:
	void DoGet( ResultParam *pResult );
	void DoPost( ResultParam *pResult );
	void DoJsonPost(ResultParam *pResult);
	BOOL IsHttps(const std::wstring &strUrl);
public:
	int					m_nContext;
	http_fetcher_cb*	m_pCallback;
private:
	std::wstring		m_strUrl;
	std::string			m_strPostData;
	std::string			m_strJsonPostData;
	int					m_nType;
	BOOL				m_bGet;
	BOOL				m_bInTask;
};

#include "SingleFetcher.hpp"