#ifndef YY_FORM_DEF
#define YY_FORM_DEF
#include "common/core/ibase.h"
#include "common/helper/formhandlehelper.h"

#define ON_VOID_VOID(hClass, fp)  {hClass::uri, WrapFormHandle(new misc::VoidFormHandle), fpt_vv, \
	(TargetFunc)(static_cast<void (IFormTarget::*)()>(fp)), NULL},

#define ON_VOID_RESULT(hClass, fp)  {hClass::uri, WrapFormHandle(new misc::VoidFormHandle), fpt_vr, \
	(TargetFunc)(static_cast<void (IFormTarget::*)(uint32_t)>(fp)), NULL},

#define ON_RET_RESULT(hClass, fp)  {hClass::uri, WrapFormHandle(new misc::VoidFormHandle), fpt_rr, \
	(TargetFunc)(static_cast<Request::ResponseType (IFormTarget::*)(uint32_t)>(fp)), NULL},

#define ON_VOID_REQUESTALL(hClass, fp)  {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vcra, \
	(TargetFunc)(static_cast<void (IFormTarget::*)(hClass *, Request &)>(fp)), NULL},

#define ON_RET_REQUESTALL(hClass, fp)  {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_rcra, \
	(TargetFunc)(static_cast<Request::ResponseType (IFormTarget::*)(hClass *, Request &)>(fp)), NULL},

#define ON_VOID_RESPONSEALL(hClass, fp)  {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vcrr, \
	(TargetFunc)(static_cast<void (IFormTarget::*)(hClass *, uint32_t, Request &)>(fp)), NULL},

#define ON_RET_RESPONSEALL(hClass, fp)  {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_rcrr, \
	(TargetFunc)(static_cast<Request::ResponseType  (IFormTarget::*)(hClass *, uint32_t, Request &)>(fp)), NULL},

#define ON_VOID_REQUEST(hClass, fp)  {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vc, \
	(TargetFunc)(static_cast<void (IFormTarget::*)(hClass *)>(fp)), NULL},
/*#define ON_RET_REQUEST(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_rc, \
	(TargetFunc)(static_cast<Request::ResponseType (IFormTarget::*)(hClass *)>(fp)), NULL},

#define ON_RET_LINK(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_rcc, \
	(TargetFunc)(static_cast<Request::ResponseType (IFormTarget::*)(hClass *, IConn *)>(fp)), NULL},
*/
#define ON_RET_RESPONSE(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_rcr, \
	(TargetFunc)(static_cast<Request::ResponseType (IFormTarget::*)(hClass *, uint32_t)>(fp)), NULL},

#define ON_VOID_RESPONSE(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vcr, \
	(TargetFunc)(static_cast<void (IFormTarget::*)(hClass *, uint32_t)>(fp)), NULL},

#define ON_VOID_LINK(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vcc, \
	(TargetFunc)(static_cast<void (IFormTarget::*)(hClass *, IConn *)>(fp)), NULL},

#define ON_VOID_LINKRESPONSE(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vccr, \
	(TargetFunc)(static_cast<void (IFormTarget::*)(hClass *, IConn *, uint32_t)>(fp)), NULL},

#define ON_RET_LINKRESPONSE(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_rccr, \
	(TargetFunc)(static_cast<Request::ResponseType (IFormTarget::*)(hClass *, IConn *, uint32_t)>(fp)), NULL},

#define ON_VOID_LINKREQUESTALL(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vccra, \
	(TargetFunc)(static_cast<void (IFormTarget::*)(hClass *, IConn *, Request &)>(fp)), NULL},

#define ON_RET_LINKREQUESTALL(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_rccra, \
	(TargetFunc)(static_cast<Request::ResponseType (IFormTarget::*)(hClass *, IConn *, Request &)>(fp)), NULL},

#define ON_RET_LINKRESPONSEALL(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_rccrr, \
	(TargetFunc)(static_cast<Request::ResponseType (IFormTarget::*)(hClass *, IConn *, uint32_t, Request &)>(fp)), NULL},

#define ON_RET_UREQUESTALL(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_rucr, \
	(TargetFunc)(static_cast<Request::ResponseType  (IFormTarget::*)(uint32_t uid, hClass *, core::Request &)>(fp)), NULL},

#define ON_RET_URESPONSEALL(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_rucrr, \
	(TargetFunc)(static_cast<Request::ResponseType  (IFormTarget::*)(uint32_t uid, hClass *, uint32_t, core::Request &)>(fp)), NULL},

#define ON_VOID_UREQUEST(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vuc, \
	(TargetFunc)(static_cast<void (IFormTarget::*)(uint32_t uid, hClass *)>(fp)), NULL},

#define ON_RET_UREQUEST(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_ruc, \
	(TargetFunc)(static_cast<Request::ResponseType (IFormTarget::*)(uint32_t uid, hClass *)>(fp)), NULL},

#define ON_VOID_UID(hClass, fp) {hClass::uri, WrapFormHandle(new misc::FormHandleT<hClass>()), fpt_vu, \
	(TargetFunc)(static_cast<void (IFormTarget::*)(uint32_t uid)>(fp)), NULL},


namespace core{
union TargetProc{
	TargetFunc mf_oo;
	void (core::IFormTarget::*mf_vv)();
	Request::ResponseType (core::IFormTarget::*mf_rr)(uint32_t);
	void (core::IFormTarget::*mf_vr)(uint32_t);
	Request::ResponseType (core::IFormTarget::*mf_rc)(void *);
	void (IFormTarget::*mf_vc)(void *);
	Request::ResponseType (core::IFormTarget::*mf_rcr)(void *, uint32_t);
	void (IFormTarget::*mf_vcr)(void *, uint32_t);
	Request::ResponseType (core::IFormTarget::*mf_rcc)(void *, core::IConn *);
	void (IFormTarget::*mf_vcc)(void *, core::IConn *);
	Request::ResponseType (IFormTarget::*mf_rccr)(void *, core::IConn *, uint32_t);
	void (IFormTarget::*mf_vccr)(void *, core::IConn *, uint32_t);
	void (IFormTarget::*mf_vcrr)(void *, uint32_t, Request &);
	Request::ResponseType (IFormTarget::*mf_rcrr)(void *, uint32_t, Request &);
	Request::ResponseType (IFormTarget::*mf_rccrr)(void *, IConn *, uint32_t, Request &);
	void (IFormTarget::*mf_vccra)(void *, IConn *, Request &);
	Request::ResponseType (IFormTarget::*mf_rccra)(void *, IConn *, Request &);
	void (IFormTarget::*mf_vcra)(void *, Request &);
	Request::ResponseType (IFormTarget::*mf_rcra)(void *, Request &);
	void (IFormTarget::*mf_vuc)(uint32_t uid, void *);
	void (IFormTarget::*mf_vu)(uint32_t uid);
	Request::ResponseType (IFormTarget::*mf_ruc)(uint32_t uid, void *);
	Request::ResponseType (IFormTarget::*mf_rucr)(uint32_t uid, void *, Request &);
	Request::ResponseType (IFormTarget::*mf_rucrr)(uint32_t uid, void *, uint32_t, Request &);
	//Request::ResponseType (IFormTarget::*)(void *) Request::ResponseType (IFormTarget::*)(void *) Request::ResponseType (IFormTarget::*)(void *) Request::ResponseType (IFormTarget::*)(void *) 
};
}
#endif

