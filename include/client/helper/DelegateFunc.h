#pragma once

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#define DELEGATE_HANDEL_CREATE(Iname,Cname,Ptype,Vlist)	\
class Iname\
{\
public:\
	virtual void Call##Ptype = 0;\
	virtual int Compare(const Iname*) = 0;\
};\
template<typename T>\
class Cname : public Iname\
{\
	typedef void (T::*classfn)##Ptype;\
public:	\
	Cname(T* pThis,classfn pfn):m_pClass(pThis),m_pfn(pfn){};\
	virtual void Call##Ptype\
	{\
		(m_pClass->*m_pfn)##Vlist;\
	}\
	virtual int Compare(const Iname* i){\
		Cname<T>* xx = (Cname<T>*)i;\
		return (xx->m_pfn == m_pfn && xx->m_pClass == m_pClass) ;\
	}\
protected:\
	classfn m_pfn;\
	T*		m_pClass;\
};



#define DEFINE_DELEGATE__HANDLE(fnName,Hname,Plist,member)	\
	public:\
	template<typename T>\
	void fnName(T* pClass, void (T::*classfn)##Plist,BOOL bConnect = TRUE){\
		if( bConnect ){\
			member( new Hname##<T>(pClass,classfn) );\
		}else{\
			if( true ){\
				member( NULL );\
			}\
		}\
	}
//
//
//template<typename T>
//class CHanldeHelper
//{
//public:
//	CHanldeHelper(){
//		_lpHandle = NULL;
//	};
//	~CHanldeHelper(){
//		Free();
//	};
//	void operator()(T* lp)
//	{
//		Free();
//		_lpHandle = lp;
//	};
//	T& GetInstance(){
//		return *_lpHandle;
//	};
//	T* GetPtr(){
//		return _lpHandle;
//	}
//protected:
//	void Free(){
//		if( _lpHandle )
//		{
//			delete _lpHandle;
//			_lpHandle = NULL;
//		}
//	}
//protected:
//	T*	_lpHandle;
//};

/*

HANDEL_CREATE(OnErrorMsg,CErrorMsgHandle,(int nError,LPCTSTR lpMsg),(nError,lpMsg) )

class xxx
{
public:
	xxx(){};

	DEFINE_HANDLE(OnErrorMsgHandle,CErrorMsgHandle,(int,LPCTSTR),m_OnErrorMsgHanlde);
protected:
	CHanldeHelper<OnErrorMsg>					m_OnErrorMsgHanlde
}


*/
