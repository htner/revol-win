#include "SingleFetcher.h"
#include <wininet.h>

#define MAX_TIMEOUT		10*1000

#define HTTP_ERROR_SPECIALCODE	600

#pragma comment(lib, "wininet.lib")

inline CSingleFetcher::CSingleFetcher( const std::wstring &strUrl, int nType, int nContext, http_fetcher_cb* pCallback, 
							  BOOL bGet, const std::string &strPostData, const std::string &strJsonPostData)
{
	m_strUrl = strUrl;
	m_strPostData = strPostData;
	m_nType = nType;
	m_nContext = nContext;
	m_pCallback = pCallback;
	m_bGet = bGet;
	m_strJsonPostData = strJsonPostData;
	m_bInTask = FALSE;
}

inline CSingleFetcher::~CSingleFetcher()
{
	
}

inline void CSingleFetcher::DoIt( ResultParam *pResult )
{
	m_bInTask = TRUE;

	if( m_bGet ) {
		DoGet(pResult);
	} else if ( m_strJsonPostData.empty() ) {
		DoPost(pResult);
	} else {
		DoJsonPost(pResult);
	}
}

inline int CSingleFetcher::GetContext()
{
	return m_nContext;
}

inline BOOL CSingleFetcher::IsInTask()
{
	return m_bInTask;
}

inline void CSingleFetcher::DoGet( ResultParam *pResult )
{
	if( NULL == pResult ) {
		return;
	}
	pResult->_nType = m_nType;
	pResult->_nContext = m_nContext;
	pResult->_pCallBack = m_pCallback;

	HINTERNET hSession = ::InternetOpen( _T("rc"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	if( hSession ) {
		//�O�ó��r
		DWORD dwTimeOut = MAX_TIMEOUT;
		InternetSetOption(hSession, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeOut, sizeof(dwTimeOut));

		DWORD dwFlag;
		if( IsHttps(m_strUrl) ) {
			dwFlag = INTERNET_FLAG_RELOAD| INTERNET_FLAG_NO_CACHE_WRITE| INTERNET_FLAG_PRAGMA_NOCACHE| INTERNET_FLAG_SECURE;
		} else {
			dwFlag = INTERNET_FLAG_RELOAD| INTERNET_FLAG_NO_CACHE_WRITE| INTERNET_FLAG_PRAGMA_NOCACHE;
		}
		std::wstring strHeader;
		if (isOfficialUrl(helper::app2net(m_strUrl))) {
			std::string token = GetRcAuthorizationString();
			if (!token.empty()) {
				strHeader += helper::net2app("Authorization: Rex " + token + "\r\n").c_str();
			}
		}
		HINTERNET hRequest = ::InternetOpenUrl( hSession, m_strUrl.c_str(), strHeader.c_str(), -1, dwFlag, DWORD_PTR(this) );
		if( hRequest ) {
			

			DWORD dwStatusCode = 0;
			DWORD dwStatusSize = sizeof(DWORD); 
			HRESULT hr = ::HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE| HTTP_QUERY_FLAG_NUMBER, 
				&dwStatusCode, &dwStatusSize, NULL);
			ATLASSERT(hr);
			pResult->_dwStatusCode = dwStatusCode;

			DWORD dwReal = 0;
			BOOL bOK = FALSE;
			std::string strData;
			do 
			{
				char szTmp[1024] = {0};
				bOK = ::InternetReadFile(hRequest, szTmp, 1024, &dwReal);
				if( dwReal ) {
					for( int i= 0; i < dwReal; i++ )
					{
						strData += szTmp[i];
					}
				} 
			} while ( bOK && dwReal );
			pResult->_data = strData;

			::InternetCloseHandle(hRequest);
			::InternetCloseHandle(hSession);
		} else {
			//�Զ��x
			pResult->_dwStatusCode = HTTP_ERROR_SPECIALCODE;
			::InternetCloseHandle( hSession );
		}//hRequest 
	}//hSession
}

inline void CSingleFetcher::DoPost( ResultParam *pResult )
{
	if( NULL == pResult ) {
		return;
	}
	pResult->_nType = m_nType;
	pResult->_nContext = m_nContext;
	pResult->_pCallBack = m_pCallback;

	URL_COMPONENTS urlc;
	ZeroMemory(&urlc, sizeof(URL_COMPONENTS));
	urlc.dwStructSize = sizeof(URL_COMPONENTS);

	TCHAR szHostName[MAX_PATH] = {0};
	TCHAR szUrlPath[MAX_PATH] = {0};
	urlc.lpszHostName = szHostName;
	urlc.dwHostNameLength = MAX_PATH;
	urlc.lpszUrlPath = szUrlPath;
	urlc.dwUrlPathLength = MAX_PATH;
	::InternetCrackUrl( m_strUrl.c_str(), m_strUrl.size(), ICU_ESCAPE, &urlc );

	HINTERNET hSession = ::InternetOpen( _T("rc"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	if( hSession ) {
		//�O�ó��r
		DWORD dwTimeOut = MAX_TIMEOUT;
		InternetSetOption(hSession, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeOut, sizeof(dwTimeOut));

		BOOL bIsHttps = IsHttps(m_strUrl);
		HINTERNET hConnect = ::InternetConnect( hSession, szHostName, bIsHttps ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, 
								INTERNET_SERVICE_HTTP, 0, 0);
		if( hConnect ) {
			DWORD dwFlag;
			if( bIsHttps ) {
				dwFlag = INTERNET_FLAG_KEEP_CONNECTION| INTERNET_FLAG_NO_CACHE_WRITE| INTERNET_FLAG_SECURE;
			} else {
				dwFlag = INTERNET_FLAG_KEEP_CONNECTION| INTERNET_FLAG_NO_CACHE_WRITE;
			}
			HINTERNET hRequest = ::HttpOpenRequest( hConnect, _T("POST"), szUrlPath, NULL, NULL, NULL, dwFlag, DWORD_PTR(this) );
			if( hRequest ) {
				std::wstring strHeader = _T("Content-type: application/x-www-form-urlencoded\r\n");
				if (isOfficialUrl(helper::app2net(m_strUrl))) {
					std::string token = GetRcAuthorizationString();
					if (!token.empty()) {
						strHeader += helper::net2app("Authorization: Rex " + token + "\r\n").c_str();
					}
				}
				::HttpSendRequest( hRequest, strHeader.c_str(), -1, (LPVOID)m_strPostData.c_str(), (m_strPostData.length() + 1)*sizeof(TCHAR) );

				DWORD dwStatusCode ;
				DWORD dwStatusSize = sizeof(DWORD); 
				::HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE| HTTP_QUERY_FLAG_NUMBER, 
					&dwStatusCode, &dwStatusSize, NULL);
				pResult->_dwStatusCode = dwStatusCode;

				::InternetCloseHandle(hSession);
				::InternetCloseHandle(hConnect);
				::InternetCloseHandle(hRequest);
			} else{
				::InternetCloseHandle(hSession);
				::InternetCloseHandle(hConnect);
			}//hRequest
		} else {
			::InternetCloseHandle(hSession);
		}//hConnect
	}//hSession
}


inline void CSingleFetcher::DoJsonPost( ResultParam *pResult )
{
	if( NULL == pResult ) {
		return;
	}
	pResult->_nType = m_nType;
	pResult->_nContext = m_nContext;
	pResult->_pCallBack = m_pCallback;

	URL_COMPONENTS urlc;
	ZeroMemory(&urlc, sizeof(URL_COMPONENTS));
	urlc.dwStructSize = sizeof(URL_COMPONENTS);

	TCHAR szHostName[MAX_PATH] = {0};
	TCHAR szUrlPath[MAX_PATH] = {0};
	urlc.lpszHostName = szHostName;
	urlc.dwHostNameLength = MAX_PATH;
	urlc.lpszUrlPath = szUrlPath;
	urlc.dwUrlPathLength = MAX_PATH;
	::InternetCrackUrl( m_strUrl.c_str(), m_strUrl.size(), ICU_ESCAPE, &urlc );

	HINTERNET hSession = ::InternetOpen( _T("rc"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	if( hSession ) {
		//�O�ó��r
		DWORD dwTimeOut = MAX_TIMEOUT;
		InternetSetOption(hSession, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeOut, sizeof(dwTimeOut));

		BOOL bIsHttps = IsHttps(m_strUrl);
		HINTERNET hConnect = ::InternetConnect( hSession, szHostName, bIsHttps ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, 
								INTERNET_SERVICE_HTTP, 0, 0);
		if( hConnect ) {
			DWORD dwFlag;
			if( bIsHttps ) {
				dwFlag = INTERNET_FLAG_KEEP_CONNECTION| INTERNET_FLAG_NO_CACHE_WRITE| INTERNET_FLAG_SECURE;
			} else {
				dwFlag = INTERNET_FLAG_KEEP_CONNECTION| INTERNET_FLAG_NO_CACHE_WRITE;
			}
			HINTERNET hRequest = ::HttpOpenRequest( hConnect, _T("POST"), szUrlPath, NULL, NULL, NULL, dwFlag, DWORD_PTR(this) );
			if( hRequest ) {
				std::wstring strHeader = _T("Content-type: application/json\r\n");
				if (isOfficialUrl(helper::app2net(m_strUrl))) {
					std::string token = GetRcAuthorizationString();
					if (!token.empty()) {
						strHeader += helper::net2app("Authorization: Rex " + token + "\r\n").c_str();
					}
				}

				::HttpSendRequest(hRequest, strHeader.c_str(), -1, (LPVOID)m_strJsonPostData.c_str(), (m_strJsonPostData.length()));

				DWORD dwStatusCode ;
				DWORD dwStatusSize = sizeof(DWORD); 
				::HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE| HTTP_QUERY_FLAG_NUMBER, 
					&dwStatusCode, &dwStatusSize, NULL);
				pResult->_dwStatusCode = dwStatusCode;

				::InternetCloseHandle(hSession);
				::InternetCloseHandle(hConnect);
				::InternetCloseHandle(hRequest);
			} else{
				::InternetCloseHandle(hSession);
				::InternetCloseHandle(hConnect);
			}//hRequest
		} else {
			::InternetCloseHandle(hSession);
		}//hConnect
	}//hSession
}

inline BOOL CSingleFetcher::IsHttps(const std::wstring &strUrl)
{
	return ( 0 == strUrl.find(_T("https")) );
}