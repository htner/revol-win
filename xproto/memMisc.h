#pragma once

#include "client/helper/allocater.h"

extern void* _temp_alloc_(size_t size);
extern void _temp_free_(void* p);

template<class T>
void _temp_delete_mem_(T* p){
	p->~T();
	_temp_free_(p);
}

#define TEMP_ALLOC_FUNC(count)		_temp_alloc_(count)
#define TEMP_FREE_FUNC(p)			_temp_free_(p)

#define TEMP_NEW_FUNC(type)		new(_temp_alloc_(sizeof(type)))
#define TEMP_DELETE_FUNC(p)		_temp_delete_mem_(p)

#define DEFINE_NEW_DELETE_OPERATOR()	\
	inline void* operator new(size_t size)\
{\
	return TEMP_ALLOC_FUNC(size);;\
}\
	inline void operator delete(void* p)\
{	\
	TEMP_FREE_FUNC(p);\
}

#ifdef _DEBUG
#define OBJECT_LEAK_CHECK(className) \
	class CObjLeakCheck{\
	public:\
		CObjLeakCheck(){\
		markptr();\
	}\
	CObjLeakCheck(CObjLeakCheck&){\
		markptr();\
	}\
	CObjLeakCheck& operator= (CObjLeakCheck&){\
		markptr();\
	}\
	~CObjLeakCheck(){\
		TEMP_FREE_FUNC(_ptr);\
	}\
	void markptr(){\
		_ptr = (char*)TEMP_ALLOC_FUNC( sizeof(#className)+2);\
			strcpy(_ptr,#className);\
		}\
		char* _ptr;\
	};\
	CObjLeakCheck __leaker__;

#else
#define OBJECT_LEAK_CHECK(className) ;
#endif // _DEBUG


#define TempAllocator(type)	allocator_X<type, _temp_alloc_, _temp_free_ >
#define PairAllocator(key,value) allocator_X<std::pair< key, value >, _temp_alloc_, _temp_free_ >

//typedef std::basic_string<char, std::char_traits<char>,TempAllocator(char) >			localStringA;
//typedef std::basic_string<wchar_t, std::char_traits<wchar_t>,TempAllocator(wchar_t) >	localStringW;
typedef std::string localStringA;
typedef std::wstring localStringW;


template <class Base>
class nu_create_object_local : public Base
{
public:	
	nu_create_object_local() :m_dwRef(0L){	
#ifdef _DEBUG		
		_ptr = (char*)TEMP_ALLOC_FUNC(strlen(typeid(Base).name()) + 2);
		strcpy(_ptr,typeid(Base).name());
#endif // _DEBUG
	}
	virtual ~nu_create_object_local(){ // this constuct will delete IAutoPtr vtable, remember
#ifdef _DEBUG
		TEMP_FREE_FUNC(_ptr);
#endif // _DEBUG
	}
public:
	virtual void addref(){			
		m_dwRef++;
	}
	virtual void release(){
		m_dwRef--;
		if( m_dwRef == 0 ){
			TEMP_DELETE_FUNC( this );
		}
	}	
	static Base* create_inst()
	{
		nu_create_object_local* pObj = TEMP_NEW_FUNC(nu_create_object_local) nu_create_object_local;
		//pObj->addref();
		return pObj;
	}
protected:
	long m_dwRef;
#ifdef _DEBUG
	char* _ptr;
#endif // _DEBUG
};
