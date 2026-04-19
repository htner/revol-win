#ifndef IBASE_COMMON_H_
#define IBASE_COMMON_H_
#include "common/packet.h"
#include "common/core/request.h"
#include <string>

namespace core {
	struct IConn;

struct IRequestHandle {
	virtual ~IRequestHandle() {
	};
	//implement answer forward end, redirect to ready
	virtual Request::ResponseType doService(Request &request) = 0;
};


struct IFormHandle {
	virtual ~IFormHandle(){}
	virtual void *handlePacket(sox::Unpack &up) = 0;
	virtual void destroyForm(void *form) = 0;
};

struct IFilter {
	virtual ~IFilter(){}
	virtual void onFilter(Request &req,IConn *conn) = 0;
};

struct FormEntry;
struct IFormTarget{
	//hold place
};

typedef void (IFormTarget::*TargetFunc)();
enum FormProcType{
	fpt_vv,				//void ::(void)
	fpt_vr,				//void ::(resCode)
	fpt_rr,				//responseType ::(resCode)
	fpt_vc,           // void ::(Cmd*);
	fpt_rc,            //  responseType :: (Cmd*);
	fpt_vcr,           // void ::(Cmd*, rescode);
	fpt_rcr,            //  responseType :: (Cmd*, rescode);
	fpt_vcc,			// void ::(Cmd *, Conn* );
	fpt_rcc,            // responseType ::(Cmd *, Conn* );
	fpt_vccr,			// void ::(Cmd *, Conn*, rescode);
	fpt_rccr,            // responseType ::(Cmd *, rescode, Conn* );
	fpt_vtc,			// void ::(Cmd *, thread local db connection *);
	fpt_rtc,           // response ::(Cmd *, thread local db connection *);
	fpt_vcrr,			// void ::(Cmd *, resCode, Request &)
	fpt_rcrr,			// response ::(Cmd *, resCode, Request &)
	fpt_vcra,			// void ::(Cmd *, Request &)
	fpt_rcra,			// response ::(Cmd *, Request &)
	fpt_rccra,			// response ::(Cmd *, Conn *, Request &)
	fpt_vccra,			// response ::(Cmd *, Conn *, Request &)
	fpt_rccrr,         // responseType ::(Cmd *, Conn *, resCode, Request &)
	fpt_vuc,			// void ::(const std::string &, Cmd *)
	fpt_ruc,			// response ::(const std::string &, Cmd *)
	fpt_vu,				// void ::(const std::string &)
	fpt_rucr,			// response ::(const std::string &, Cmd *, Request &)
	fpt_rucrr,			// response ::(const std::string &, Cmd *, uint32_t, Request &)
	fpt_null,			//do nothing
	ftp_last,
};
struct WrapFormHandle{
	IFormHandle *_f;
	WrapFormHandle(IFormHandle *f): _f(f){}
	~WrapFormHandle(){delete _f;}
	inline IFormHandle * get(){ return _f;}
};
struct FormEntry{
	uint32_t uri;
	WrapFormHandle requestForm;
	int type;
	TargetFunc proc;
	IFormTarget *target;
};


#define DECLARE_FORM_MAP static core::FormEntry *getFormEntries(); \
						static core::FormEntry formEntries[];

#define BEGIN_FORM_MAP(theClass) \
	FormEntry* theClass::getFormEntries()\
{ return theClass::formEntries; } \
	FormEntry theClass::formEntries[] = \
{ \


#define END_FORM_MAP() \
{0, WrapFormHandle(NULL), fpt_null, NULL, NULL} \
}; \

struct IContext {
	uint32_t cid;
	uint16_t sid;
	uint32_t connSerial;
	URI_TYPE uri;
	IContext (){cid = 0xffffffff; sid = 0xffff; uri = 0xffffffff; connSerial = 0xffffffff;}
	virtual ~IContext(){}
	virtual void * getHandler() = 0;
};

class SimpleCtx: public core::IContext{
public:
	SimpleCtx(uint32_t c, uint32_t s, uint32_t serial){
		cid = c;
		sid = s;
		connSerial = serial;
	}
	virtual void * getHandler(){
		assert(false);
		return NULL;
	}
};

struct IAppContextEx{
	virtual ~IAppContextEx(){}
	virtual Request::ResponseType contextDispatch(Request &request, IConn *conn, IContext *ctx) = 0;
	virtual Request::ResponseType requestDispatch(Request &request, IConn *conn, bool bCtx = false) = 0;
	virtual void setParent(IAppContextEx *p) = 0;
	//xxx todo be true virtual 
	virtual void addEntry(FormEntry *entries, void *target){}
	//virtual void setDispatcher(protocol::link::IResponseDispatcher * disp) = 0;
};

/*
struct IAppContext {
	//set request handle is a prepare operation; it's a inner interface
	//virtual void addRequestHandle(URI_TYPE, IRequestHandle *, IFormHandle *) = 0;
	//virtual void setResponseHdl(URI_TYPE, IResponseHandle*, IFormHandle *) = 0;
	virtual ~IAppContext(){}
	virtual size_t dispatchData(const char *data, size_t sz, IConn *conn) = 0;

	virtual void contextDispatch(const LFrame &lf, sox::Unpack &up, IConn *conn, IContext *ctx) = 0;
	virtual void requestDispatch(const LFrame &lf, sox::Unpack &up, IConn *conn) = 0;
	virtual void setDispatcher(IResponseDispatcher * disp) = 0;
	//virtual void start() = 0;
};*/

}
#endif /*IBASE_H_*/
