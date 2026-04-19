#include <list>
#include <atlstr.h>
#include <map>
#include "helper/INet.h"


#define WM_UPLOAD_FILE	(WM_USER + 123)


inline http_uploader::http_uploader(){
	_curTask = NULL;
	_hThread = NULL;
	_DammyWnd.Watch( this, WM_UPLOAD_FILE);
	InitializeCriticalSection(&_cs);
};
inline http_uploader::~http_uploader(){
	
	_DammyWnd.Revoke();
	DeleteCriticalSection(&_cs);
};

inline int http_uploader::UploadFile(LPCTSTR lpPath,http_upload_cb* cb,int type,LPCTSTR lpUploadUrl,BOOL bDelUploadfile)
{

	URL_COMPONENTS cmp = {0};
	TCHAR szHost[MAX_PATH]= {0};
	TCHAR szUrl[MAX_PATH * 2]= {0};
	TCHAR szUserName[MAX_PATH] = {0};
	TCHAR szPassword[MAX_PATH] = {0};

	cmp.dwStructSize = sizeof(cmp);
	cmp.lpszHostName = szHost;
	cmp.dwHostNameLength = _countof(szHost);
	cmp.lpszUrlPath = szUrl;
	cmp.dwUrlPathLength = _countof(szUrl);
	cmp.dwUserNameLength = _countof(szUserName);
	cmp.lpszUserName = szUserName;
	cmp.dwPasswordLength = _countof(szPassword);
	cmp.lpszPassword = szPassword;
	
	if(InternetCrackUrl(lpUploadUrl,_tcslen(lpUploadUrl),0,&cmp))
	{
		_DammyWnd.Create();
		uploadItem item;
		item._strpath = lpPath;
		item._cb = cb;
		item._type = type;
		item._strUrl = cmp.lpszUrlPath;
		item._strHost = cmp.lpszHostName;
		item._nPort = cmp.nPort;
		item._bDelete = bDelUploadfile;
		PushTask(item);
		StartUpload();
		return item._itemId;		
	}
	return -1;
}


inline void http_uploader::Remove(http_upload_cb* cb,bool bCancelTask)
{
	EnterCriticalSection(&_cs);
	ItemMap::iterator it = _ItemMap.begin();
	for(; it != _ItemMap.end(); )
	{
		if( it->second._cb == cb )
		{
			if( bCancelTask )
			{
				it = _ItemMap.erase( it );
				continue;
			}else
			{
				it->second._cb = NULL;
			}
		}
		it++;
	}

	if( _curTask && cb == _curTask->_cb)
	{
		_curTask->_cb = NULL;
	}
	LeaveCriticalSection(&_cs);
}


inline BOOL http_uploader::UploadFile(uploadItem& item)
{
	
	BOOL bSuc = FALSE;

	INTERNET_BUFFERS BufferIn = {0};
	LPCTSTR acceptTypes[2] = {_T("*/*"), 0};
	DWORD dwFlags = INTERNET_FLAG_RELOAD|INTERNET_FLAG_NO_CACHE_WRITE;

	HANDLE hFile = CreateFile (item._strpath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile )
	{	
		BufferIn.dwStructSize = sizeof( INTERNET_BUFFERS);
		
		
		CInternetSession hSession(_T(""), PRE_CONFIG_INTERNET_ACCESS/*INTERNET_OPEN_TYPE_DIRECT*/, NULL, NULL, 0);
		CInternetConnection hConnect(hSession,item._strHost.c_str(), item._nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0);

		//CHttpFile hQuery(hConnect, _T("HEAD"), item._strUrl.c_str(), _T("HTTP/1.1"), NULL, acceptTypes, dwFlags);
		//if(bSuc = HttpSendRequestEx(hQuery,&BufferIn, NULL, 0,(DWORD_PTR)(CInternetHandle*)&hQuery)){
		//	if(bSuc = HttpEndRequest(hQuery,NULL,NULL,NULL))
		//	{
		//		UploadResult res;
		//		res._itemId = item._itemId;

		//		BYTE szContentLength[64] = {0};
		//		int nLength = sizeof(szContentLength);
		//		try{
		//			hQuery.QueryInfo(HTTP_QUERY_CONTENT_LENGTH,szContentLength,(unsigned long *)&nLength,NULL);	
		//			res._bSuc = true;
		//		}catch(...){
		//			res._bSuc = false;
		//		}
		//		if(szContentLength[0] != '0' || !res._bSuc)
		//		{
		//			if( item._cb)
		//			{
		//				//ÞDUI¾€³Ì
		//				_DammyWnd.Fire((WPARAM)&item,(LPARAM)&res);
		//			}
		//			return TRUE;
		//		}
		//	}			
		//}


		BufferIn.dwBufferTotal = GetFileSize (hFile, NULL);
		CHttpFile hRequest(hConnect, _T("PUT"), item._strUrl.c_str(), _T("HTTP/1.1"), NULL, acceptTypes, dwFlags);
		bSuc = HttpSendRequestEx(hRequest,&BufferIn, NULL, 0,(DWORD_PTR)(CInternetHandle*)&hRequest);
		
		int nProcessLast = -1;
		if( bSuc )
		{
			DWORD dwBytesRead, dwBytesWritten;
			BYTE pBuffer[1024];
			BOOL bRead;

			DWORD sum = 0;

			do
			{
				if( ! (bRead = ReadFile(hFile, pBuffer, sizeof(pBuffer),&dwBytesRead, NULL)) ) {
					bSuc = FALSE;
					break;
				}

				if( ! (bSuc = InternetWriteFile(hRequest, pBuffer, dwBytesRead,&dwBytesWritten)) ) {
					break;
				}
				sum += dwBytesWritten;

			}while(dwBytesRead == sizeof(pBuffer));
			CloseHandle(hFile);

			UploadResult res;
			res._bSuc = false;
			res._itemId = item._itemId;
			if( bSuc )
			{
				bSuc = HttpEndRequest(hRequest,NULL,NULL,NULL);
				res._bSuc = bSuc;
				if( bSuc && item._cb)
				{					
					res._strResult = ReadRespond(&hRequest);										
				}						
			}
			if( item._cb)
			{
				//ÞDUI¾€³Ì
				_DammyWnd.Fire((WPARAM)&item,(LPARAM)&res);
			}
		}		
	}	
	return bSuc;
}

inline std::string http_uploader::ReadRespond(CHttpFile* hRequest)
{
	std::string strResult;
	CHAR buf[4096]={0}; 
	CHttpFile::CInfo info;
	DWORD dwRead = 0;		
	try{
		//BOOL bRead = InternetReadFile(hRequest, buf, sizeof(buf), &dwRead);
		dwRead = hRequest->Read(buf,sizeof(buf),info);
		while(dwRead > 0)
		{
			strResult.append( buf,dwRead);
			dwRead = hRequest->Read(buf,sizeof(buf),info);
		}			
		
	}catch(...){}
	return strResult;
}
inline void http_uploader::StartUpload()
{	
	DWORD dwWait = WaitForSingleObject( _hThread, 1);

	if( dwWait == WAIT_TIMEOUT )
		return;
	_hThread = CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)UploadThreadProc,this,NULL,NULL);
}
inline DWORD	http_uploader::UploadThreadProc(LPVOID lpParam)
{
	http_uploader* lpThis = (http_uploader*)lpParam;
	return lpThis->UploadProc();
}

inline DWORD http_uploader::UploadProc()
{
	uploadItem task;		
	while( PopTask(task) )
	{
		_curTask  = &task;
		UploadFile(task);
		_curTask = NULL;
	}
	CloseHandle(_hThread);
	_hThread = NULL;
	return 0;
}

inline int http_uploader::PushTask(uploadItem& task)
{
	static int g_taskId = 0;
	EnterCriticalSection(&_cs);
	task._itemId = g_taskId++;
	_ItemMap[task._itemId] = task;		
	LeaveCriticalSection(&_cs);
	return task._itemId;
}

inline BOOL http_uploader::PopTask(uploadItem& task)
{
	BOOL bSuc = FALSE;

	EnterCriticalSection(&_cs);
	ItemMap::iterator it = _ItemMap.begin();
	if( it != _ItemMap.end() )
	{
		task = it->second;
		_ItemMap.erase( it );
		bSuc  = TRUE;
	}
	LeaveCriticalSection(&_cs);

	return bSuc;
}

inline void http_uploader::OnDamyyWndEvent(HWND hwnd,UINT nMsg,WPARAM wParam,LPARAM lParam)
{
	ATLASSERT(nMsg == WM_UPLOAD_FILE);
	UploadResult* lpRes = (UploadResult*)lParam;
	EnterCriticalSection(&_cs);
	//ItemMap::iterator it = _ItemMap.begin();
	//ItemMap::iterator itEnd = _ItemMap.end();
	//for( ; it != itEnd; it++)
	{
		uploadItem& item = *(uploadItem*)wParam;//it->second;
		//if( item._itemId == lpRes->_itemId )
		{
			if(http_upload_cb* cb = item._cb)
			{				
				if(lpRes->_bSuc)	
					cb->on_upload_data(item._itemId,item._type,lpRes->_strResult);
				else
					cb->on_upload_error(item._itemId,item._type);
				if(item._bDelete)
				{
					DeleteFile(item._strpath.c_str());
				}
			}
			//break;
		}
	}	
	if( _ItemMap.empty() )
		_DammyWnd.Destroy();
	LeaveCriticalSection(&_cs);
}