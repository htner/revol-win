// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "sciter-x-host-callback.h"
#include "sciter-x-script.h"
#include "sciter-x-graphin.h"


class SciterBoolean
{
public:
	SciterBoolean(bool* v)
	{
		_v = v;		
		_out = *v;
	}
	~SciterBoolean(){
		*_v = _out;
	}
	bool* _v;
	bool _out;

};

#define CHAIN_STATE_IDEL	0
#define CHAIN_STATE_BUSY	1
#define CHAIN_STATE_FINISH	2


#define CHAIN_TO_SCRITER_EX() \
	SciterBoolean xXx(&_bHasChainToSciter);\
	if( !_bHasChainToSciter ){\
		_bHasChainToSciter = true;\
		BOOL bHandled = FALSE; \
		lResult = ::SciterProcND(hWnd,uMsg, wParam, lParam, &bHandled); \
		if(bHandled) return TRUE; \
	}


#define BEGIN_SCRITER_MSG_MAP(theClass) \
public: \
	BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0) \
{ \
	BOOL bHandled = TRUE; \
	switch(dwMsgMapID) \
	{ \
	case 0:\
	{
		//CHAIN_TO_SCRITER_EX()

#define END_SCRITER_MSG_MAP() \
	break; \
	}\
	default: \
		ATLTRACE(ATL::atlTraceWindowing, 0, _T("Invalid message map ID (%i)\n"), dwMsgMapID); \
		ATLASSERT(FALSE); \
		break; \
	} \
	return FALSE; \
}


#define CHAIN_SCRITER_MSG_MAP(theChainClass) \
{ \
	if(theChainClass::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult)) {\
		return TRUE; \
	}\
}

#define CHAIN_TO_SCITER() \
    { \
        BOOL bHandled = FALSE; \
        lResult = ::SciterProcND(hWnd,uMsg, wParam, lParam, &bHandled); \
        if(bHandled) return TRUE; \
    }

#define CHAIN_TO_SCITER_EX(thisClassName) \
	if( _ChainId == thisClassName::SciterChainID){\
		BOOL bHandled = FALSE; \
		lResult = ::SciterProcND(hWnd,uMsg, wParam, lParam, &bHandled); \
		if(bHandled) {\
			return TRUE;\
		}\
	}

		
#define INIT_SCITER_CHAIN_ID(thisClassName,baseClassName) \
	protected:\
	enum {SciterChainID = baseClassName::SciterChainID + 1};\
	virtual void InitChainId(){\
		_ChainId = thisClassName::SciterChainID;\
	};

class CSciterView : 
    public CWindowImpl<CSciterView>, 
    public sciter::notification_handler<CSciterView>, // callback handler
    public sciter::event_handler // dom events handler

{
protected:
	virtual void InitChainId(){
		_ChainId = CSciterView::SciterChainID;
	};
public:
	enum {SciterChainID = 0};
  int			 _ChainId;
  typedef sciter::notification_handler<CSciterView> super_handler;

  CSciterView(){	  
	  _ChainId = -1;
  }

  //
  HWND Create(HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
	  DWORD dwStyle = 0, DWORD dwExStyle = 0,
	  _U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
  {
	  InitChainId();
	  return CWindowImpl<CSciterView>::Create(hWndParent,rect,szWindowName,dwStyle,dwExStyle,MenuOrID,lpCreateParam);
  }
  // sciter::notification_handler traits
  HWND      get_hwnd() { return m_hWnd; }
  HINSTANCE get_resource_instance() { return _Module.GetResourceInstance(); }

  // sciter::event_handler stuff
  //virtual bool handle_scripting_call(HELEMENT he, SCRIPTING_METHOD_PARAMS& params ){return true;}; 

  HVM VM() { return SciterGetVM(m_hWnd); }

  virtual LRESULT on_load_data(LPSCN_LOAD_DATA pnmld)
  {
	  return LOAD_OK;
  }

  virtual LRESULT on_data_loaded(LPSCN_DATA_LOADED pnmld)  
  { 
    return 0; 
  }

	BEGIN_MSG_MAP(CSciterView)		
		CHAIN_TO_SCITER_EX(CSciterView) // this must be the very first item!

		MESSAGE_HANDLER(WM_CREATE, OnCreate)		
	END_MSG_MAP()


	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		setup_callback();
		sciter::attach_dom_event_handler(m_hWnd, this); //  attach this sciter::event_handler
		return 0;
	}


  // invokes view.something(...) method if that view.something is defined.
  // used for view events generation.
  json::value InvokeViewMethod(LPCWSTR name, unsigned argc = 0, json::value* argv = 0 )
  {
     WCHAR fullname[128];
     unsigned fullname_length = swprintf(fullname,L"view.%s",name);
     json::value m;
     SciterEval(m_hWnd,fullname,fullname_length,&m);
     if( m.is_object_function() )
     {
        return m.call(argc,argv);
     }
     return json::value();
  }

  virtual LRESULT on_callback_host(LPSCN_CALLBACK_HOST pns) 
  { 
#ifndef _DEBUG
	  extern int GetRcEnvConfigInt(LPCTSTR lpSesstion,LPCTSTR lpKey,int nDefValue);
	  static BOOL g_bHostCb = GetRcEnvConfigInt(_T("sciter"),_T("hostcb"),0);
	  if( !g_bHostCb )
		return 0;
#endif // Relase

    static sciter::debug_output_console dbgcon;
    switch(pns->channel)  
    {
      case 0: // 0 - stdin, read from stdin requested, put string into pnmld->r 
        break;  
      case 1: // 1 - stdout, "stdout << something" requested, pnmld->p1 is 
              //     string to output.
        //dbgcon.printf("stdout:%S", pns->p1.to_string().c_str() );
        dbgcon.print("stdout:");
        dbgcon.print(pns->p1.to_string().c_str());
        break;  
      case 2: // 2 - stderr, "stderr << something" requested or error happened, 
              //     pnmld->p1 is string to output.
        //dbgcon.printf("stderr:%S", pns->p1.to_string().c_str() );
        dbgcon.print("stderr:");
        dbgcon.print(pns->p1.to_string().c_str());
        break;
      default:
        // view.callback(channel,p1,p2) call from script

        dbgcon.printf("callback on channel %d, values: %S,%S\n", pns->channel, pns->p1.to_string(CVT_JSON_LITERAL).c_str(), pns->p2.to_string(CVT_JSON_LITERAL).c_str() );
        // implement this if needed
        break;  
    }
    return 0; 
  }
  
  /* test of calling function on DOCUMENT_COMPLETE for the root document.
  virtual bool on_event (HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason ) 
  { 
    if( type == DOCUMENT_COMPLETE )
    {
      sciter::dom::element root = GetRootElement();
      if( target == root )
      {
        tiscript::value v = tiscript::call(VM(), root.get_namespace(), "fn");
      }
    }
    return FALSE; 
  }*/



};
