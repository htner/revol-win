#ifndef  _NU_AUTO_PTR_H
#define _NU_AUTO_PTR_H
#include "nuassert.h"
#include "loki_alloctor.h"
#include "sockcomm.h"

namespace nsox
{

template <class T>
class _NoAddRefReleaseOnCComPtr : public T
{
private:
	virtual void addref()  = 0;
	virtual void release() = 0;
};


template <class T>
class NuAutoPtrBase
{
protected:
	NuAutoPtrBase() throw()
	{
		p = NULL;
	}
	NuAutoPtrBase(int nNull) throw()
	{
		nu_assert(nNull == 0);
		(void)nNull;
		p = NULL;
	}
	NuAutoPtrBase(T* lp) throw()
	{
		p = lp;
		if (p != NULL)
			p->addref();
	}
public:
	typedef T _PtrClass;
	~NuAutoPtrBase() throw()
	{
		if (p)
			p->release();
	}
	operator T*() const throw()
	{
		return p;
	}
	T& operator*() const throw()
	{
		nu_assert(p!=NULL);
		return *p;
	}
	//The assert on operator& usually indicates a bug.  If this is really
	//what is needed, however, take the address of the p member explicitly.
	T** operator&() throw()
	{
		nu_assert(p==NULL);
		return &p;
	}
	//_NoAddRefReleaseOnCComPtr<T>* operator->() const throw()
	//{
	//	return (_NoAddRefReleaseOnCComPtr<T>*)p;
	//}
        T* operator->() const throw()
        {
		return p;
        }

	bool operator!() const throw()
	{
		return (p == NULL);
	}
	bool operator<(T* pT) const throw()
	{
		return p < pT;
	}
	bool operator==(T* pT) const throw()
	{
		return p == pT;
	}

	// Release the interface and set to NULL
	void release() throw()
	{
		T* pTemp = p;
		if (pTemp)
		{
			p = NULL;
			pTemp->release();
		}
	}
	// Attach to an existing interface (does not AddRef)
	void attach(T* p2) throw()
	{
		if (p)
			p->release();
		p = p2;
	}
	// Detach the interface (does not Release)
	T* detach() throw()
	{
		T* pt = p;
		p = NULL;
		return pt;
	}
	bool copyTo(T** ppT) throw()
	{
		nu_assert(ppT != NULL);
		if (ppT == NULL)
			return false;
		*ppT = p;
		if (p)
			p->addRef();
		return true;
	}
	T* p;
};

template <class T>
class nu_auto_ptr: public NuAutoPtrBase<T>
{
typedef NuAutoPtrBase<T> Base;
public:
	nu_auto_ptr() throw()
	{
	}
	nu_auto_ptr(int nNull) throw() :NuAutoPtrBase<T>(nNull)
	{
	}
	nu_auto_ptr(T* lp) throw() :NuAutoPtrBase<T>(lp)
	{
	}
	nu_auto_ptr(const nu_auto_ptr<T>& lp) throw() :NuAutoPtrBase<T>(lp.p)
	{
	}
	nu_auto_ptr& operator=(T* pT)
	{
		if(Base::p) Base::p->release();
		Base::p = pT;
		if(Base::p) Base::p->addref();
		return *this;
	}
	nu_auto_ptr& operator=(const nu_auto_ptr<T>& pT)
	{
			*this = (T*)pT;
			return *this;
	}
};

class nu_single_thread_model
{
public:
	static long increment(long* p){return ++ (*p);}
	static long decrement(long* p){return -- (*p);}
};

#ifdef NSOX_WIN32
class nu_multi_thread_model
{
public:
	static long increment(long* p){return InterlockedIncrement(p);}
	static long decrement(long* p){return InterlockedDecrement(p);}
};
#endif


template <class Base, class ThreadModel = nu_single_thread_model>
class nu_create_object : public Base
						, public Loki::SmallObject<>
{
private:
	typedef Base _BaseClass;
	nu_create_object(void* = NULL) throw():m_dwRef(0L){			
	}
	virtual ~nu_create_object() throw(){ // this constuct will delete IAutoPtr vtable, remember
	}
public:
	virtual void addref(){			
			ThreadModel::increment(&m_dwRef);
	}
	virtual void release(){
		if(ThreadModel::decrement(&m_dwRef) == 0){
				delete this;
		}
	}	
	static Base* create_inst()
	{
			nu_create_object* pObj = new nu_create_object;
			//pObj->addref();
			return pObj;
	}
protected:
	long m_dwRef;
};

}

#endif

