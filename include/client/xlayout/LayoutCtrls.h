#pragma once
#include <map>
#include "ILayout.h"
#include "LayoutEvent.h"
#include <common/xstring.h>
#include "LayoutWindow.h"
#include "xctrl/wtlstatic.h"
#include "xctrl/wtlscrollbar.h"
#include "xctrl/wtlpicture.h"
#include "xctrl/wtlbutton.h"
#include "xctrl/wtltree.h"
#include "xctrl/wtledit.h"
#include "xctrl/XEdit.h"
#include "xctrl/wtlhyperlink.h"
#include "xctrl/wtlslider.h"
#include "xctrl/wtltabbrowser.h"
#include "xctrl/wtltabctrl.h"
#include "xctrl/wtllistbox.h"
#include "xctrl/wtllistview.h"
#include "xctrl/XListView.h"
#include "xctrl/combox.h"
#include "xctrl/RCToolbar.h"
#include "xctrl/wtlgroup.h"
#include "xctrl/wtlHotKeyCtrl.h"
#include "xctrl/wtlFolderView.h"


NAMESPACE_BEGIN(xlayout)

template<class TWindow>
class LayoutWndCreator
    : public ILayoutWndCreator
{
public:
    static LayoutWndCreator<TWindow>* inst()
    {
        static LayoutWndCreator<TWindow> xx;
        return &xx;
    }

    ILayoutWindow* Create(ILayoutMgrWindow* pMgrWnd, ILayoutObj* pLayoutObj)
    {
        CWTLayoutWindow<TWindow> * pLayoutWindow = new CWTLayoutWindow<TWindow>;
		xstring strText;
		if (LPCTSTR lpPro = pLayoutObj->GetProperty(_T("text")))
		{
			strText = lpPro;
		} else if (LPCTSTR lpPro = pLayoutObj->GetProperty(_T("stringid")))
		{
			int nId;
			CTextUtil::TextToInt(lpPro, nId);
			strText = LTS_(nId);
		}
		if (LPCTSTR lpPro = pLayoutObj->GetProperty(_T("suffix")))
		{
			strText += lpPro;
		}
		xctrl::XControlStyle<TWindow> style;
		style.ParseStyle(pLayoutObj->GetProperty(_T("style")));
        pLayoutWindow->Create(pMgrWnd->GetHWND(), NULL, strText.c_str(), WS_CHILD | WS_VISIBLE | style.GetControlStyle(), 0, pMgrWnd->GetCtrlIdByName(pLayoutObj->GetName()));
        return pLayoutWindow;
    }
};


inline static void InitCreators()
{
    LayoutWndFactory::inst()->RegisterCreator(_T("Static"),		LayoutWndCreator<xctrl::CStatic>::inst());
    LayoutWndFactory::inst()->RegisterCreator(_T("Button"),	    LayoutWndCreator<xctrl::CButton>::inst());
    LayoutWndFactory::inst()->RegisterCreator(_T("TBButton"),   LayoutWndCreator<xctrl::CButton>::inst());
    LayoutWndFactory::inst()->RegisterCreator(_T("Picture"),	LayoutWndCreator<xctrl::CPicture>::inst());
    LayoutWndFactory::inst()->RegisterCreator(_T("ScrollBar"),	LayoutWndCreator<xctrl::CScrollBar>::inst());
    LayoutWndFactory::inst()->RegisterCreator(_T("Slider"),	    LayoutWndCreator<xctrl::CSliderBar>::inst());
    LayoutWndFactory::inst()->RegisterCreator(_T("Tree"),	    LayoutWndCreator<xctrl::CWTLTreeCtrl>::inst());
    LayoutWndFactory::inst()->RegisterCreator(_T("Edit"),		LayoutWndCreator<xctrl::CEdit>::inst());
    LayoutWndFactory::inst()->RegisterCreator(_T("RichEdit"),	LayoutWndCreator<xctrl::XCssRichEdit>::inst());
    LayoutWndFactory::inst()->RegisterCreator(_T("HyperLink"),	LayoutWndCreator<xctrl::CHyperLink>::inst());
	LayoutWndFactory::inst()->RegisterCreator(_T("ButtonLink"),	LayoutWndCreator<xctrl::CButtonLink>::inst());
	LayoutWndFactory::inst()->RegisterCreator(_T("TabBrowser"),	LayoutWndCreator<xctrl::CTabBrowser>::inst());
	LayoutWndFactory::inst()->RegisterCreator(_T("TabCtrl"),	LayoutWndCreator<xctrl::CTabCtrl>::inst());
	LayoutWndFactory::inst()->RegisterCreator(_T("ListBox"),	LayoutWndCreator<xctrl::CListBox>::inst());
	LayoutWndFactory::inst()->RegisterCreator(_T("ListView"),	LayoutWndCreator<xctrl::CWTLListViewCtrl>::inst());
	LayoutWndFactory::inst()->RegisterCreator(_T("Combox"),		LayoutWndCreator<xctrl::CCombox>::inst());
	LayoutWndFactory::inst()->RegisterCreator(_T("Toolbar"),	LayoutWndCreator<xctrl::CRCToolbar>::inst());
	LayoutWndFactory::inst()->RegisterCreator(_T("Group"),		LayoutWndCreator<xctrl::CGroupBox>::inst());
	LayoutWndFactory::inst()->RegisterCreator(_T("HotKeyCtrl"),	LayoutWndCreator<xctrl::XHotKeyCtrl>::inst());
	LayoutWndFactory::inst()->RegisterCreator(_T("FolderView"),	LayoutWndCreator<xctrl::CFolderView>::inst());
}

NAMESPACE_END(xlayout)
