#pragma once

#ifdef _DEBUG
#define REF_STATIC_MODULE
#else
#endif

#ifdef REF_STATIC_MODULE
#include "atlconv.h"
#include "map"
#include "string"

class CRefStatic
{
public:
	CRefStatic(LPCTSTR name,int nThreshold = 0)
	{
		_subClassRef = new RefMap; // don't delete
		_nRef = 0;				
		_nThreshold = nThreshold;
		_tcscpy(_szName,name);

		extern CRefStatic g__RefStaticRoot__;
		_next = NULL;
		
		if(this == &g__RefStaticRoot__)
		{
			g__RefStaticRoot__._next = _root;
			_root = &g__RefStaticRoot__;
		}else
		{
			if(_root == NULL)
			{			
				_root = this;
			}else
			{
				CRefStatic* last = _root;
				for (; last->_next; last = last->_next){};
				last->_next = this;
			}
		}
		
	}
	~CRefStatic()
	{
		USES_CONVERSION;
		if(_nRef > _nThreshold)
		{
			ATLTRACE(_T("memory leak?![obj: %s, count: %d]\n"),_szName,_nRef);
			RefMap::iterator it = _subClassRef->begin();
			RefMap::iterator itEnd = _subClassRef->end();
			for (; it != itEnd; it++)
			{
				if( it->second != 0 )
				{
					ATLTRACE(_T("====>leak: %s, %d \n"),CW2T(it->first.c_str()),it->second);
				}
			}			
			ATLASSERT(FALSE);
		}		

		if(this == _root)
		{
			CRefStatic* p = _root->_next;
			while (p)
			{
				CRefStatic* pp = p->_next;				
				delete p;
				p = pp;
			}
			_root->_next = NULL;
		}		
	}
public:
	void AddRef(LPCSTR className = NULL)
	{
		USES_CONVERSION;
		if(className){
			std::wstring s = CA2W(className);
			AddRef(s.c_str());
		}
	}
	void Release(LPCSTR className = NULL)
	{
		USES_CONVERSION;
		if(className){
			std::wstring s = (CA2W(className));
			Release(s.c_str());			
		}
	}
	void AddRef(LPCWSTR className = NULL)
	{				
		Ref(className,TRUE);
	}
	void Release(LPCWSTR className = NULL)
	{				
		Ref(className,FALSE);
	}
protected:
	void Ref(LPCWSTR cls,BOOL bAdd)
	{
		if(!cls)return;

		if(bAdd)
		{	
			_nRef++;
			(*_subClassRef)[cls] ++;		
		}else
		{
			_nRef--;
			(*_subClassRef)[cls]--;
		}
	}
public:
	int		_nRef;
	int		_nThreshold;	
	TCHAR	_szName[128];		
	CRefStatic*			_next;
	typedef std::map< std::wstring ,int> RefMap;
	RefMap*	_subClassRef;
	static CRefStatic* _root;
};		

	#define REF_STATIC_NAME(name)						g__##name##__
	#define GET_CLASS_TYPE()							typeid(this).name()
	#define DEFINE_REF_STATIC(name)						CRefStatic* REF_STATIC_NAME(name) = new CRefStatic(_T(#name),0);
	#define DEFINE_REF_STATIC_EX(name,threshold)		CRefStatic* REF_STATIC_NAME(name) = new CRefStatic(_T(#name),threshold);
	#define REF_STATIC_ADDREF(name)						REF_STATIC_NAME(name)->AddRef( GET_CLASS_TYPE() );
	#define REF_STATIC_RELEASE(name)					REF_STATIC_NAME(name)->Release( GET_CLASS_TYPE() );
	#define REF_STATIC_ID_ADDREF(name,id)				REF_STATIC_NAME(name)->AddRef( id );
	#define REF_STATIC_ID_RELEASE(name,id)				REF_STATIC_NAME(name)->Release( id );

#else

	#define DEFINE_REF_STATIC(name)						
	#define DEFINE_REF_STATIC_EX(name,threshold)		
	#define REF_STATIC_ADDREF(name)						
	#define REF_STATIC_RELEASE(name)					
	#define REF_STATIC_ID_ADDREF(name,id)
	#define REF_STATIC_ID_RELEASE(name,id)

#endif // REF_STATIC_MODULE
