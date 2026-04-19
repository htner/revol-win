#ifndef TREQUESTHELPER_H_
#define TREQUESTHELPER_H_
#include "common/core/ibase.h"
#include "member_dispatcher.h"
#include "contexthelper.h"

template<class O, class C> core::Request makeRequest_RH(O &obj, C *ptr, core::Request::ResponseType (C::* fun)(core::Request &)){
	return core::Request(obj.uri, obj,  new misc::OwnPtrCtx<core::IRequestHandle>(new misc::RequestDispatcher<C>(ptr, fun)));
}

template<class O, class C> core::Request makeRequestEnd_RH(O &obj, C *ptr, void (C::* fun)(core::Request &)){
	return core::Request(obj.uri, obj,  new misc::OwnPtrCtx<core::IRequestHandle>(new misc::RequestDispatcherEnd<C>(ptr, fun)));
}

#endif /*TREQUESTHELPER_H_*/
