#ifndef __ISKINMGR_H__
#define __ISKINMGR_H__

#include <string>

class ISkinMgr
{
public:
	virtual void Init(const std::wstring& version, HWND hMainFrame, HWND hSkinBtn, const std::wstring& curThemeName=_T("")) = 0;
	virtual void ShowSkinWnd() = 0;
	virtual void DestroySkinWnd() = 0;
};

class CSkinner
{
protected:
	CSkinner()
	{
		m_hSkinDllInst = NULL;
		m_pSkinMgr = NULL;

		//Init();
	}
	~CSkinner()
	{
		if(m_hSkinDllInst)
		{
			if(m_pSkinMgr)
				m_pSkinMgr->DestroySkinWnd();
			FreeLibrary(m_hSkinDllInst);//FreeLibraryAndExitThread
		}
		m_hSkinDllInst = NULL;
		m_pSkinMgr = NULL;
	}
protected:
	HINSTANCE m_hSkinDllInst;
	ISkinMgr* m_pSkinMgr;
public:
	void Init(const std::wstring& version, HWND hMainFrame, HWND hSkinBtn)
	{
		if(m_hSkinDllInst)
			return;

		//»ñÈ¡APPÂ·¾¶
		std::wstring strAppPath;
		TCHAR tmp[1024];
		tmp[0] = 0;
		if(GetModuleFileName(NULL, tmp, sizeof(tmp)) > 0) {
			TCHAR* dst = NULL;
			TCHAR* p = tmp;
			while (*p) {
				if (*p == _T('\\'))
					dst = p;
				p = CharNext(p);
			}
			if(dst) *dst = 0;
		}
		strAppPath = tmp;
#ifdef _DEBUG
		std::wstring strSkinDll = strAppPath + _T("\\skin_d.dll");
#else
		std::wstring strSkinDll = strAppPath + _T("\\skin.dll");
#endif // _DEBUG
		
		if ( m_hSkinDllInst = LoadLibrary(strSkinDll.c_str()) )
		{
			typedef ISkinMgr* (WINAPI *MYFUNC)();
			MYFUNC funCoCreateSkinMgr = NULL; 
			funCoCreateSkinMgr = (MYFUNC)GetProcAddress (m_hSkinDllInst, "CoCreateSkinMgr");
			if(funCoCreateSkinMgr) 
			{
				m_pSkinMgr = funCoCreateSkinMgr();
				if (m_pSkinMgr)
					m_pSkinMgr->Init(version, hMainFrame, hSkinBtn);
			}
		}
	}
	void Free()
	{
		if(m_hSkinDllInst)
		{
			if(m_pSkinMgr)
			{
				m_pSkinMgr->DestroySkinWnd();
			}
			FreeLibrary(m_hSkinDllInst);
		}
		m_hSkinDllInst = NULL;
		m_pSkinMgr = NULL;
	}

	void ShowSkinWnd(const std::wstring& version, HWND hMainFrame, HWND hSkinBtn, const std::wstring& curThemeName=_T(""))
	{
		if (!m_pSkinMgr)		
		{
			Init(version, hMainFrame, hSkinBtn);
		}
		if(m_pSkinMgr)
		{
			m_pSkinMgr->Init(version,hMainFrame, hSkinBtn, curThemeName);
			m_pSkinMgr->ShowSkinWnd();
		}
	}
public:
	static CSkinner* Inst()
	{
		static CSkinner sk;
		return & sk;
	}
public:

};
#define Skinner  (*(CSkinner::Inst()))



#endif