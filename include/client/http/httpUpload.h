#pragma once

#include <map>
#include "common/xstring.h"
#include "helper/DammyWnd.h"


struct http_upload_cb {
	virtual void on_upload_data(int context, int type, std::string& data) = 0;
	virtual void on_upload_error(int context, int type) = 0;
};

class CHttpFile;
class http_uploader : public IDammyWndEvent
{
	struct  uploadItem
	{
		xstring _strpath;
		xstring _strUrl;
		xstring _strHost;
		int		_nPort;
		int		_type;
		int		_itemId;
		bool	_bDelete;
		http_upload_cb* _cb;
	};
	struct UploadResult
	{
		bool		_bSuc;								
		int			_itemId;
		std::string _strResult;
	};

	http_uploader();
public:
	~http_uploader();
	static http_uploader* inst()
	{
		static http_uploader ins;
		return &ins;
	}
public:	
	int	 UploadFile(LPCTSTR lpPath,http_upload_cb* cb,int type,LPCTSTR strUploadUrl,BOOL bDelUploadfile = FALSE);
	void Remove(http_upload_cb* cb,bool bCancelTask = false );
public:	
	virtual void OnDamyyWndEvent(HWND hwnd,UINT nMsg,WPARAM wParam,LPARAM lParam);
	//ÞDUI¾€³Ì
	//void Onhttp_uploaderHandle(LPCTSTR lptask,LPCTSTR lpStatus,LPCTSTR lpdata);
protected:
	BOOL UploadFile(uploadItem& item);
	std::string ReadRespond(CHttpFile* hRequest);
	void StartUpload();
	DWORD UploadProc();
	int PushTask(uploadItem& task);
	BOOL PopTask(uploadItem& task);
	static DWORD	UploadThreadProc(LPVOID lpParam);
protected:
	typedef std::map<int,uploadItem>	ItemMap;
	HANDLE	_hThread;
	ItemMap	_ItemMap;
	CDammyWnd _DammyWnd;
	uploadItem* _curTask;
	CRITICAL_SECTION	_cs;
};

#include "httpUpload.hpp"