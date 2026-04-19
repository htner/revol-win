#pragma once
#include <map>
#include "ILayout.h"
#include "LayoutEvent.h"
#include "common/xstring.h"
#include "xctrl/xctrl.h"
#include <list>

NAMESPACE_BEGIN(xlayout)
struct ILayoutWindow;
struct ILayoutRequest;

struct ILayoutWindow
	: implement xctrl::IReCalcUI
{
	virtual void        AttachLayout(xlayout::ILayoutObj* layout)				= 0;
	virtual void        DetachLayout()											= 0;
	virtual void        MoveLayoutWnd(LPCRECT lpCect, BOOL bRepaint)			= 0;
	virtual void        ShowLayoutWnd(bool bShow)								= 0;
	virtual HDWP        DeferWindowPos(LPCRECT lpCect, HDWP hDwp)				= 0;
	virtual void		DestroySelf()											= 0;
	virtual void		SetTipHolder(xctrl::IToolTipHolder* pTipHolder)			= 0;
};

struct ILayoutDrawer
{
	virtual void        DrawLayout(xgdi::ICanvas* canvas, RECT rcUpdate)		= 0;
};


struct ILayoutMgrWindow
{
	virtual HWND        GetHWND()	                                    = 0;
	virtual int         GetCtrlIdByName(LPCTSTR)                        = 0;
};

struct ILayoutWndCreator 
{
	virtual ILayoutWindow* Create(ILayoutMgrWindow* , ILayoutObj*)		= 0;
};

class LayoutWndFactory
{
public:
	static LayoutWndFactory* inst()
	{
		static LayoutWndFactory factory;
		return &factory;
	}

	void RegisterCreator(LPCTSTR wndType, ILayoutWndCreator* creator)
	{
		__creators[wndType] = creator;
	}
	ILayoutWindow* Create(LPCTSTR wndType,	ILayoutMgrWindow* pMgrWnd, ILayoutObj* pLayoutObj)
	{
		WndCreatorMap::iterator itr = __creators.find(wndType);
		if(itr != __creators.end()){
			ILayoutWndCreator* creator = itr->second;
			return creator->Create(pMgrWnd, pLayoutObj);
		}
		return NULL;
	}
protected:
	typedef std::map<xstring, ILayoutWndCreator*>  WndCreatorMap;
	WndCreatorMap __creators;
};

class CLayoutMgrWindow
	: public CWindowImpl<CLayoutMgrWindow>
	, public CMessageFilter
	, public ILayoutMgrWindow
{


#define XLAYOUT_ID(name) \
	GetCtrlIdByName(_T(#name))

#define XLAYOUT_BUTTON(name)\
	GetCtrlByName<xctrl::CButton>(_T(#name))

#define XLAYOUT_EDIT(name)\
	GetCtrlByName<xctrl::CEdit>(_T(#name))

#define XLAYOUT_STATIC(name)\
	GetCtrlByName<xctrl::CStatic>(_T(#name))

#define XLAYOUT_PICTURE(name)\
	GetCtrlByName<xctrl::CPicture>(_T(#name))

#define XLAYOUT_SCROLLBAR(name)\
	GetCtrlByName<xctrl::CScrollBar>(_T(#name))

#define XLAYOUT_SLIDERBAR(name)\
	GetCtrlByName<xctrl::CSliderBar>(_T(#name))

#define XLAYOUT_RICHEDIT(name)\
	GetCtrlByName<xctrl::XCssRichEdit>(_T(#name))

#define XLAYOUT_HYPERLINK(name)\
	GetCtrlByName<xctrl::CHyperLink>(_T(#name))

#define XLAYOUT_BUTTONLINK(name)\
	GetCtrlByName<xctrl::CButtonLink>(_T(#name))

#define XLAYOUT_TABCTRL(name)\
	GetCtrlByName<xctrl::CTabCtrl>(_T(#name))

#define XLAYOUT_TABBROWSER(name)\
	GetCtrlByName<xctrl::CTabBrowser>(_T(#name))

#define XLAYOUT_LISTBOX(name)\
	GetCtrlByName<xctrl::CListBox>(_T(#name))

#define XLAYOUT_LISTVIEW(name)\
	GetCtrlByName<xctrl::CWTLListViewCtrl>(_T(#name))

#define XLAYOUT_COMBOX(name)\
	GetCtrlByName<xctrl::CCombox>(_T(#name))

#define XLAYOUT_TREECTRL(name)\
	GetCtrlByName<xctrl::CWTLTreeCtrl>(_T(#name))

#define XLAYOUT_COMBOX(name)\
	GetCtrlByName<xctrl::CCombox>(_T(#name))

#define XLAYOUT_TOOLBAR(name)\
	GetCtrlByName<xctrl::CRCToolbar>(_T(#name))

#define XLAYOUT_HOTKEYCTRL(name)\
	GetCtrlByName<xctrl::XHotKeyCtrl>(_T(#name))

#define XLAYOUT_GROUP(name)\
	GetCtrlByName<xctrl::CGroupBox>(_T(#name))

#define XLAYOUT_FOLDERVIEW(name)\
	GetCtrlByName<xctrl::CFolderView>(_T(#name))

#define XLAYOUT_CONTROL(TClass,TName)\
	GetCtrlByName<TClass>(_T(#TName))

#define XLAYOUT_ELEMENT(name) m_layout##name

#define XLAYOUT_DEF(tclass, name) \
	xlayout::CWTLayoutWindow<tclass>* XLAYOUT_ELEMENT(name)

#define XLAYOUT_CREATE(tclass, name, text) \
	m_layout##name = new xlayout::CWTLayoutWindow<tclass>();\
	if(m_layout##name->Create(m_hWnd, 0, text, WS_CHILD | WS_VISIBLE, 0, XLAYOUT_ID(name))) { \
	AddLayoutWindow(m_layout##name, _T(#name)); \
	}
#define XLAYOUT_CREATE_EX(tclass, name, text, style) \
	m_layout##name = new xlayout::CWTLayoutWindow<tclass>();\
	if(m_layout##name->Create(m_hWnd, 0, text, WS_CHILD | WS_VISIBLE | style, 0, XLAYOUT_ID(name))) { \
	AddLayoutWindow(m_layout##name, _T(#name)); \
	}

#define XLAYOUT_RECALC_OBJ()\
	if(ReSizeAllObj())ReLayout(true);

public:
	CLayoutMgrWindow()
		: m_fontDefault(NULL), m_nDefID(0)
	{
		m_pLayout = CreateLayout();
		m_pLayout->SetCallBack(CLayoutMgrWindow::LayoutEventProc, (DWORD)this);
		m_hDwp	  = NULL;
	}

	virtual ~CLayoutMgrWindow()
	{
		if(m_pLayout)
		{
			m_pLayout->Uninit();
			m_pLayout->Release();
		}
		LayoutGetter::iterator itr = m_layouts.begin();
		for(; itr != m_layouts.end(); ++ itr)
		{
			if(ILayoutWindow* pWndObj = m_mapObj2Wnd[*itr])
			{
				pWndObj->DestroySelf();
			}
		}
	}
	virtual HWND GetHWND()
	{
		return m_hWnd;
	}

	virtual int GetCtrlIdByName(LPCTSTR lpszName) 
	{
		static int id = 100;

		ctrl_id_map::iterator itr = m_idMap.find(lpszName);
		if(itr == m_idMap.end()){
			m_idMap[lpszName] = ++ id;
			return id;                
		}
		return itr->second;
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(::GetForegroundWindow() == m_hWnd && ::IsWindowVisible(m_hWnd) && pMsg->message == WM_KEYDOWN)
		{
			DealPreMsg(pMsg);
		}
		return FALSE;
	}

	virtual void DealPreMsg(MSG* pMsg)
	{
		ATL::CString strTitle;
		GetWindowText(strTitle);
		//ATLTRACE(_T("Try CWindow(%s)\n"), strTitle.GetString());
		switch(pMsg->wParam)
		{
		case VK_TAB:
			if(::GetParent(pMsg->hwnd) == m_hWnd)
			{
				if(HWND hWnd = ::GetNextDlgTabItem(m_hWnd, ::GetFocus(), ::GetKeyState(VK_SHIFT) & 0x8000))
				{
					::SetFocus(hWnd);
				}
			}
			break;

		case VK_RETURN:
			{
				UINT nID = GetDefID();
				HWND hWndItem = ::GetDlgItem(m_hWnd, nID);
				if(::IsWindowEnabled(hWndItem))
				{
					SendMessage(WM_COMMAND, MAKEWPARAM(nID, BN_CLICKED), (LPARAM)hWndItem);
				}
				break;
			}
		case VK_ESCAPE:
			{
				 DoEscape();
			}
			break;
		default:
			break;
		}
	}

	virtual BOOL DoEscape() { return FALSE;}

	DECLARE_WND_CLASS(NULL)

	BEGIN_MSG_MAP(CLayoutMgrWindow)
		MESSAGE_HANDLER(WM_PAINT,           OnPaint)
		MESSAGE_HANDLER(WM_CREATE,          OnCreate)
		MESSAGE_HANDLER(WM_DESTROY,         OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND,      OnEraseBkgnd)
		MESSAGE_HANDLER(WM_SIZE,            OnSize)

		MESSAGE_HANDLER(WM_LBUTTONDOWN,     OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP,       OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE,       OnMouseMove)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//    LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//    LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//    LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CPaintDC dc(m_hWnd);

		//TODO: Add your drawing code here
#ifdef _DEBUG_DUMP_LAYOUT
		RECT rcClient;
		GetClientRect(&rcClient);
		dc.FillSolidRect(&rcClient, RGB(250, 250, 250));

		class CVisitObject
			: public xlayout::ILayoutVisitor
		{
		public:
			CVisitObject(CPaintDC& dc)
				: m_dcObject(dc)
			{
			}

			virtual void dumpBegin(){}
			virtual void dumpEnd(){}

			virtual void dump(ILayoutObj* pObj, int nDepth)
			{
				WTL::CString str;
				str.Format(_T("%d"), pObj->GetID());
				m_dcObject.SetBkMode(TRANSPARENT);
				switch(pObj->GetType())
				{
				case TYPE_OBJ:
					m_dcObject.DrawText(pObj->GetName(), -1, (LPRECT)pObj->GetBounds(), DT_LEFT | DT_SINGLELINE);
					m_dcObject.DrawText(str, -1, (LPRECT)pObj->GetBounds(), DT_RIGHT | DT_BOTTOM |DT_SINGLELINE);
					m_dcObject.FrameRect(pObj->GetBounds(), (HBRUSH)::GetStockObject(BLACK_BRUSH));
					break;

				case TYPE_SPLITTER_H:
				case TYPE_SPLITTER_V:
				case TYPE_GROUP:
				default:
					break;
				}
			}

		protected:
			CPaintDC& m_dcObject;
		} visitobj(dc);
		m_pLayout->Visit(&visitobj);
#endif

		return 0;
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_LayoutEvent.SetLayout(m_pLayout);
		m_LayoutEvent.SetHwnd(m_hWnd);

		AutoCreateObject();

		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		ReLayout();	
		return 0;
	}

	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		POINT pt = {LOWORD(lParam), HIWORD(lParam)};
		m_LayoutEvent.OnLButtonDown(wParam, pt);

		return 0;
	}

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		POINT pt = {LOWORD(lParam), HIWORD(lParam)};
		m_LayoutEvent.OnLButtonUp(wParam, pt);

		return 0;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		POINT pt = {LOWORD(lParam), HIWORD(lParam)};
		m_LayoutEvent.OnMouseMove(wParam, pt);

		return 0;
	}

	void ReLayout(bool bForce = false)
	{
		RECT rcClient;
		GetClientRect(&rcClient);
		m_hDwp = ::BeginDeferWindowPos(20);

		m_pLayout->ReLayout((bForce ? NULL : &rcClient));

		::EndDeferWindowPos(m_hDwp);

		m_hDwp   = NULL;

	}

	BOOL realResizeObj(xgdi::ICanvas* pCanvas,ILayoutWindow* laywnd,ILayoutObj* pObj)
	{
		BOOL bChange1 = FALSE;
		BOOL bChange2 = FALSE;
		SIZE size = {0};
		if(BOOL bRecal = laywnd->ReCalcUISize(pCanvas,size))
		{				
			wchar_t buf[20];

			if(size.cx)
			{
				if(LPCTSTR lpWidth = pObj->GetProperty(_T("width")))
				{
					int n = _tcstol(lpWidth,NULL,10);
					bChange1 = (n != size.cx);
				}else
				{
					bChange1 = TRUE;
				}
				if( bChange1 )
				{
					_itow(size.cx, buf, 10);			
					bool bSuc = pObj->SetProperty(_T("width"),	buf);
				}
			}

			if(size.cy)
			{
				if(LPCTSTR lpWidth = pObj->GetProperty(_T("height")))
				{
					int n = _tcstol(lpWidth,NULL,10);
					bChange2 = (n != size.cx);
				}else
				{
					bChange2 = TRUE;
				}
				if(bChange2)
				{
					_itow(size.cy, buf, 10);
					pObj->SetProperty(_T("height"),	buf);
				}
			}
		}
		return (bChange1 || bChange2);
	}
	BOOL ReSizeObj(LPCTSTR name)
	{
		BOOL bChange = FALSE;
		if(ILayoutObj* pObj = m_pLayout->GetLayoutObj(name))
		{			
			if( ILayoutWindow* laywnd = m_mapObj2Wnd[pObj] )
			{
				xgdi::ICanvas* pCanvas = (xgdi::ICanvas*)xgdi::CoCreateUIObject(xgdi::IID_UICanvas);
				HDC hDC = GetDC();
				pCanvas->Attach(hDC);
				ReleaseDC(hDC);
				bChange = realResizeObj(pCanvas,laywnd,pObj);
				pCanvas->Detach();
				pCanvas->Release();
			}
		}
		return bChange;

	}
	BOOL ReSizeAllObj()
	{
		BOOL bChange = FALSE;
		HDC hDC = GetDC();
		xgdi::ICanvas* pCanvas = (xgdi::ICanvas*)xgdi::CoCreateUIObject(xgdi::IID_UICanvas);
		pCanvas->Attach(hDC);
		ReleaseDC(hDC);

		LAYOUT_MAP::iterator it =  m_mapObj2Wnd.begin();
		LAYOUT_MAP::iterator itEnd =  m_mapObj2Wnd.end();
		for (; it != itEnd; it++)
		{
			bChange |= realResizeObj(pCanvas,it->second,it->first);
		}

		pCanvas->Detach();
		pCanvas->Release();
		return bChange;
	}

	void AddLayoutWindow(ILayoutWindow* pWindow, LPCTSTR name)
	{
		if( NULL == pWindow ) {
			ATLASSERT(FALSE);
		}
		if(ILayoutObj* pObj = m_pLayout->GetLayoutObj(name))
		{
			if(m_mapObj2Wnd[pObj] == NULL)
			{
				m_mapObj2Wnd[pObj] = pWindow;
				pWindow->AttachLayout(pObj);
			}
		}
	}

	void AddLayoutWindow(ILayoutWindow* pWindow, int nID)
	{
		if( NULL == pWindow ) {
			ATLASSERT(FALSE);
		}
		if(ILayoutObj* pObj = m_pLayout->GetLayoutObj(nID))
		{
			if(m_mapObj2Wnd[pObj] == NULL)
			{
				m_mapObj2Wnd[pObj] = pWindow;
				pWindow->AttachLayout(pObj);
			}
		}   
	}
	template<class TWindow>
	TWindow* GetCtrlByName(LPCTSTR name)
	{
		if (!name)
			return NULL;
		if(ILayoutObj* pObj = m_pLayout->GetLayoutObj(name))
		{
			ILayoutWindow* pWindow = m_mapObj2Wnd[pObj];
			if(pWindow != NULL)
			{
				return dynamic_cast<TWindow*>(pWindow);
			}
		}   
		return NULL;
	}

	virtual BOOL OnLayoutEvent(int nType, ILayoutObj* pObj)
	{
		LAYOUT_MAP::iterator iter = m_mapObj2Wnd.find(pObj);
		if(iter != m_mapObj2Wnd.end())
		{
			if( ILayoutWindow* pWindow = iter->second ) {
				// type = 0  MOVE
				if(nType == 0){
					if(m_hDwp)
					{
						m_hDwp = pWindow->DeferWindowPos(pObj->GetBounds(), m_hDwp);
					}else
					{
						pWindow->MoveLayoutWnd(pObj->GetBounds(), TRUE);
					}

					// type = 1 SHOW
				}else{
					//pWindow->ShowLayoutWnd(pObj->IsHidden());
					pWindow->ShowLayoutWnd(IsObjectVisible(pObj));
				}
				return TRUE;
			}
		}
		return FALSE;
	}

	bool IsObjectVisible(ILayoutObj* pObj)
	{
		if(pObj == NULL) return TRUE;
		if(pObj->IsHidden()) return FALSE;

		return IsObjectVisible(pObj->GetParent());
	}

	virtual void LoadLayout(LPCTSTR filename)
	{
		if(m_pLayout){
			bool bSuc = m_pLayout->Init(filename);
			if (!bSuc)
			{
				xstring strErr = _T("Init layout error :");
				if(filename)
					strErr.append(filename);
				::MessageBox(NULL, strErr.c_str(), _T("error"), MB_OK);
			}
		}
	}

	inline ILayout* GetILayout() const
	{
		return m_pLayout;
	}

	inline UINT GetDefID() const
	{
		return m_nDefID;
	}

	inline void SetDefID(UINT nID)
	{
		m_nDefID = nID;
	}

	virtual xctrl::IToolTipHolder* GetTipHolder()
	{
		return NULL;
	}

	// layout obj auto-creator
	virtual void AutoCreateObject()
	{
		m_layouts.clear();
		m_pLayout->Visit(&m_layouts);

		// Begin Auto Creating Procedure
		LayoutGetter::iterator itr = m_layouts.begin();
		for(; itr != m_layouts.end(); ++itr)
		{
			ILayoutObj* pLayoutObj = (*itr);
			// class-specified only
			if(LPCTSTR wndClass = pLayoutObj->GetProperty(_T("class")))
			{
				if(ILayoutWindow* pLayoutWnd = LayoutWndFactory::inst()->Create(wndClass, this, pLayoutObj))
				{
					pLayoutWnd->SetTipHolder(GetTipHolder());
					AddLayoutWindow(pLayoutWnd, pLayoutObj->GetName());
				}
			}
		}
	}

#define XELEMENT(type,name) \
	Element<type>(_T(#name))

	template <class TWindow>
	TWindow* Element(LPCTSTR name)
	{
		ILayoutObj* pLayObj = m_pLayout->GetLayoutObj(name);
		if(pLayObj){
			ILayoutWindow* pLayWnd= m_mapObj2Wnd[pLayObj];
			return dynamic_cast<TWindow*>(pLayWnd);
		}   
		return NULL;
	}

private:
	static void CALLBACK LayoutEventProc(int nType, ILayoutObj* pObj, DWORD_PTR data)
	{
		CLayoutMgrWindow* pLayoutMgr = (CLayoutMgrWindow*)data;
		pLayoutMgr->OnLayoutEvent(nType, pObj);
	}

protected:
	HFONT                                   m_fontDefault;
	ILayout*                                m_pLayout;

	typedef     std::map<xstring, int>      ctrl_id_map;
	ctrl_id_map                             m_idMap;
	HDWP									m_hDwp;

	UINT                                    m_nDefID;

private:
	class LayoutGetter
		: public ILayoutVisitor
		, public std::list<ILayoutObj*>
	{
	public:
		LayoutGetter(){}

		virtual void dumpBegin(){}
		virtual void dumpEnd(){}
		virtual void dump(xlayout::ILayoutObj* pObj, int nDepth)
		{
			// tag "obj" only
			if(pObj && pObj->GetType() == TYPE_OBJ)
			{
				this->push_back(pObj);
			}
		}

	} m_layouts;

	CLayoutEvent                            m_LayoutEvent;

	typedef std::map<ILayoutObj*, ILayoutWindow*> LAYOUT_MAP;
	LAYOUT_MAP									  m_mapObj2Wnd;
};

/************************************************************************/
/*   CLayoutProperty                                                    */
/************************************************************************/

class CLayoutProperty
	: public IProperty
{
public:
	CLayoutProperty(ILayoutObj* pObj)
	{
		m_pLayoutObj = pObj;
	}
	// From IProperty
	virtual LPCTSTR GetProperty(LPCTSTR lpszProp) const
	{
		return ParseProperty(lpszProp);
	}
	virtual LPCTSTR GetProperty(LPCTSTR lpszProp, LPCTSTR lpszDefault) const
	{
		return ParseDefPro(lpszProp, lpszDefault);
	}
	virtual int GetProperty(LPCTSTR lpszProp, int nDefValue) const
	{
		return ParseDefProInt(lpszProp, nDefValue);
	}
	virtual BOOL SetProperty(LPCTSTR lpszProp)
	{
		return FALSE;
	}
	virtual BOOL SetProperty(int nValue)
	{
		return FALSE;
	}

protected:


	LPCTSTR ParseProperty(LPCTSTR lpProp) const
	{
		if (lpProp)
		{
			if (m_pLayoutObj)
			{
				if (LPCTSTR lpValue = m_pLayoutObj->GetProperty(lpProp) )
				{
					if(lpValue[0] == '#')
					{
						xstring strValue = lpValue;						
						xstring strtemp = lpProp;

						if ( strtemp == _T("font-color") || strtemp == _T("font-color-disabled"))
						{
							CFontColorAdaptor adaptor(strValue.substr(1, strValue.size() - 1).c_str());
							//if (adaptor.IsNeedAdjust())
								return adaptor.NewProperty();
							//return lpValue;
						}
						LPCTSTR lpRes = xskin::QueryProperty(strValue.substr(1, strValue.size() - 1).c_str());
						return lpRes;						
					}
					return lpValue;
				}
				else
					return NULL;
				
			}
		}
		return NULL;
	}

	LPCTSTR ParseDefPro(LPCTSTR lpProp, LPCTSTR lpdef) const
	{
		LPCTSTR lpRet = ParseProperty(lpProp);
		return lpRet ? lpRet : lpdef;
	}

	int ParseDefProInt(LPCTSTR lpProp, int ndef) const
	{
		LPCTSTR lpRet = ParseProperty(lpProp);
		return lpRet ? _ttoi(lpRet) : ndef;
	}

	class CFontColorAdaptor
	{
	public:
#define FONT_ADAPTOR_NODE _T("FontColorAdaptor")
		CFontColorAdaptor()
		{
		}

		CFontColorAdaptor(LPCTSTR lpNode)
		{
			__strNode = lpNode ? lpNode : _T("");
		}

		LPCTSTR NewProperty()
		{
			if (!IsNeedAdjust())
				return NULL;
			LPCTSTR pp = xskin::QueryProperty(__strNode.c_str());;
			xstring strValue = pp;
			int nPosSplitter = strValue.find('#');
			if (nPosSplitter == xstring::npos)
			{
				return pp;
			}


			xstring strTint = strValue.substr(0, nPosSplitter);
			xstring strDark = strValue.substr(nPosSplitter + 1, strDark.size());
			if (xskin::CSkinLoader::inst()->IsDark())
				return xskin::QueryProperty(strDark.c_str());
			else
				return xskin::QueryProperty(strTint.c_str());
		}

		BOOL IsNeedAdjust()
		{
			return TRUE;
			//if (__strNode == FONT_ADAPTOR_NODE)
			//	return TRUE;
			//return FALSE;
		}
	protected:
		xstring __strNode;
	};

protected:
	ILayoutObj* m_pLayoutObj;
	xstring		m_strCurPro;
};

// Windowless
class CLayoutWindowLess
	: implement ILayoutWindow
	, implement ILayoutDrawer
{
public:
	CLayoutWindowLess()
	{
		m_pLayoutObj = NULL;
		m_pToolTipHolder = NULL;
		m_bShow = TRUE;
		m_bAddTooTip = m_bNotifyWnd = FALSE;
	}
	virtual ~CLayoutWindowLess(){
	}
	// From ILayoutWindow
	virtual void AttachLayout(ILayoutObj* layout)
	{
		m_pLayoutObj = layout;
	}

	virtual void DetachLayout()
	{
		m_pLayoutObj = NULL;
	}

	virtual void MoveLayoutWnd(LPCRECT lpRect, BOOL bRepaint)
	{
	}

	virtual HDWP DeferWindowPos(LPCRECT lpRect, HDWP pos)
	{
		return pos;
	}

	virtual void ShowLayoutWnd(bool bShow)
	{
		m_bShow = bShow;
	}

	virtual void DrawLayout(xgdi::ICanvas* canvas, RECT rcUpdate)
	{

	}
	virtual void DestroySelf()
	{
		delete this;
	}

	inline ILayoutObj* GetLayoutObj()
	{
		return m_pLayoutObj;
	}

	LPCRECT GetBounds()
	{
		if(m_pLayoutObj){
				return m_pLayoutObj->GetBounds();
		}else{
				ASSERT(FALSE);
				return &NULL_RECT;
		}		
	}

	virtual void SetTipHolder(xctrl::IToolTipHolder* pTipHolder)
	{
		m_pToolTipHolder = pTipHolder;
	}

protected:
	bool				    m_bShow;
	ILayoutObj*             m_pLayoutObj;
	BOOL			m_bAddTooTip, m_bNotifyWnd;
	xctrl::IToolTipHolder*	m_pToolTipHolder;
};


// Window/Control
template <class TWindow>
class CWTLayoutWindow
	: public TWindow
	, public CLayoutWindowLess
	, public xskin::ISkinEvent
{
public:
	CWTLayoutWindow()
	{
		m_pLayoutObj = NULL;
		xskin::CSkinLoader::inst()->Watch((xskin::ISkinEvent*)this);
	}

	~CWTLayoutWindow()
	{
		xskin::CSkinLoader::inst()->revoke((xskin::ISkinEvent*)this);
	}

	virtual void AttachLayout(ILayoutObj* layout)
	{
		CLayoutWindowLess::AttachLayout(layout);
		PreAddTip(&CLayoutProperty(m_pLayoutObj));
		if(xctrl::ICssParser* pParser = dynamic_cast<xctrl::ICssParser*>(this))
		{
			pParser->ParseCss(&CLayoutProperty(m_pLayoutObj));
		}
	}

	void PreAddTip(IProperty* pProp)
	{
		LPCTSTR take_tooltip= pProp->GetProperty(_T("take-tooltip"));
		if(take_tooltip)
		{
			CTextUtil::TextToInt(take_tooltip, m_bAddTooTip);
		}

		LPCTSTR take_notify_tooltip= pProp->GetProperty(_T("take-notify-tooltip"));
		if(take_notify_tooltip)
		{
			CTextUtil::TextToInt(take_notify_tooltip, m_bNotifyWnd);
			if (m_bNotifyWnd)
				m_bAddTooTip = m_bNotifyWnd;
		}

		LPCTSTR tip = pProp->GetProperty(_T("tstringid"));
		ATL::CString strWndText;
		if(tip)
		{
			int nStringid;
			CTextUtil::TextToInt(tip, nStringid);
			strWndText = LTS_(nStringid);
		}

		if (m_pToolTipHolder && TWindow::IsWindow() && (m_bAddTooTip || m_bNotifyWnd))
		{
			if (strWndText.IsEmpty())
				TWindow::GetWindowText(strWndText);

			if (m_bAddTooTip)
				m_pToolTipHolder->AddTool(TWindow::m_hWnd, strWndText);
			else
				m_pToolTipHolder->RemoveTool(TWindow::m_hWnd);

			if (m_bNotifyWnd)
				m_pToolTipHolder->SetNotify(TWindow::m_hWnd);
			else
				m_pToolTipHolder->RemoveNotify(TWindow::m_hWnd);
		}
	}

	virtual void MoveLayoutWnd(LPCRECT lpRect, BOOL bRepaint)
	{
		CRect rcWindow(lpRect);
		TWindow::MoveWindow(&rcWindow, FALSE);
		TWindow::Invalidate(TRUE);

		ATLTRACE(_T("MoveLayoutWnd: %s\n"), m_pLayoutObj->GetName());
	}

	virtual void ShowLayoutWnd(bool bShow)
	{
		::ShowWindow(TWindow::m_hWnd, bShow ? SW_SHOW : SW_HIDE);

		ATLTRACE(_T("ShowLayoutWnd %s: %s\n"), (bShow ? _T("On") : _T("Off")), m_pLayoutObj->GetName());
	}

	virtual HDWP DeferWindowPos(LPCRECT lpRect, HDWP pos)
	{
		return ::DeferWindowPos(pos,TWindow::m_hWnd,NULL,lpRect->left,lpRect->top,
			lpRect->right - lpRect->left,
			lpRect->bottom - lpRect->top,
			SWP_NOACTIVATE|SWP_NOZORDER);
	}

	virtual void DestroySelf()
	{
		delete this;
	}

	virtual void OnSkinChange()
	{
		AttachLayout(m_pLayoutObj);
	}

	virtual HWND GetUIHandler()
	{
		return TWindow::m_hWnd;
	}

	virtual BOOL ReCalcUISize(xgdi::ICanvas* pCanvas,SIZE& size)
	{
		if(xctrl::IReCalcObj* pParser = dynamic_cast<xctrl::IReCalcObj*>(this))
			return pParser->CalcUISize(pCanvas,size);
		return FALSE;
	}
};

#define SAFE_OBJ_CALL(obj,fun,p) if( (obj) ) {(obj)->fun##p;}

NAMESPACE_END(xlayout)
