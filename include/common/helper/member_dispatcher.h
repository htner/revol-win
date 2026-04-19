#pragma once
#include "common/core/ibase.h"


namespace misc {

template<class T> class RequestDispatcher: public core::IRequestHandle{
public:
	typedef core::Request::ResponseType (T::* fun_t)(core::Request&);
	T* ref;
	fun_t mf;
	RequestDispatcher(T *obj, fun_t fun):ref(obj), mf(fun){}
	virtual core::Request::ResponseType doService(core::Request &ctx){
		return (ref->*mf)(ctx);
	}
};

template<class T> class RequestDispatcherEnd: public core::IRequestHandle{
public:
	typedef void (T::* fun_t)(core::Request&);
	T* ref;
	fun_t mf;
	RequestDispatcherEnd(T *obj, fun_t fun):ref(obj), mf(fun){}
	virtual core::Request::ResponseType doService(core::Request &ctx){
		(ref->*mf)(ctx);
		return core::Request::END;
	}
};

}
