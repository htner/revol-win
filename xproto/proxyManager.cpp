#include "proxyManager.h"
#include "ProtoLink.h"
#include "protocol/pkg_tag.h"
#include "common/iproperty.h"
#include "common/core/request.h"
#include "helper/utf8to.h"

#define LIST_PING_TIME 1* 1000

using namespace xproto;

CProxyManager::CProxyManager()
{
		__listTimer.init(this, &CProxyManager::onListTimeout);
		__protoLink = NULL;
}


CProxyManager::~CProxyManager(void)
{
}
void CProxyManager::setProtoLink(CProtoLink* link)
{
		__protoLink = link;
}

void CProxyManager::onGetProxy(XCast<PClientDcProxyRes>& cmd, nproto::connection* c)
{
	nsox::xlog(NsoxInfo, " proxy back, method:%s res:%d", cmd.method.data(), cmd.resCode);
	if(core::Request::ifSuccess(cmd.resCode)){
		
		raid_proxy_map::iterator itr = __elems.find(cmd.method);
		if(itr != __elems.end()){
			itr->second->onGetProxy(cmd, c);
		}else{
			nsox::xlog(NsoxInfo, "can't proxy method, method:%s", cmd.method.data());
		}
	}
}


void CProxyManager::writeRequest(XCast<PClientDcProxy>& req)
{
	
	if (__lasttime  > timeGetTime() || timeGetTime() - __lasttime > 1000) {
		__lasttime  = timeGetTime();
		__count = 1;
	} else {
		++__count;
	}
	if   (__count > 3|| !__reqlist.empty() ) {
		__listTimer.start(LIST_PING_TIME);
		__reqlist.push_back(req);
	} else {
		__protoLink->writeRequest(&req, PClientDcProxy::uri);
	}
}


IProxyElem* CProxyManager::getProxyElem(const xstring& m)
{
	   std::string method = app2net(m);

	nsox::nu_auto_ptr<CProxyElem>  elem = __elems[method];
	if(elem == NULL){
		elem = nsox::nu_create_object<CProxyElem>::create_inst();	
		elem->setMethod(m);
		elem->setManager(this);
		__elems[method] = elem;
	}
	return elem;
}

void  CProxyManager::onListTimeout()
{
	while(!__reqlist.empty()) {
		if (__lasttime  > timeGetTime() || timeGetTime() - __lasttime > 1000) {
			__lasttime  = timeGetTime();
			__count = 1;
		} else {
			++__count;
		}
		if (__count > 3) {
				break;
		}
		nsox::xlog(NsoxInfo, "onListTimeout pop list request %d", timeGetTime());
		req_list_t::iterator it = __reqlist.begin();
		__protoLink->writeRequest(&(*(it)), PClientDcProxy::uri);
		__reqlist.pop_front();
		
	};
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
CProxyElem::CProxyElem()
{
		//__method = m;
		__proxyMgr = NULL;
}

void  CProxyElem::setMethod(const xstring& m)
 {
	 __method = m;
 }

void CProxyElem::request(uint16_t type,  const xstring& params, const xstring& postdata)
{
	//nsox::xlog(NsoxInfo, "get new request %s %s", __method.data() , req.method.data(););
	XCast<PClientDcProxy> req;
	req.idd = type;
	req.method = app2net(__method);
	req.params =  app2net(params);
	req.postdata =  app2net(postdata);
	__proxyMgr->writeRequest(req);

	nsox::xlog(NsoxInfo, "get new request %s, %d", req.method.data(), timeGetTime());
}


void CProxyElem::onGetProxy(XCast<PClientDcProxyRes>& cmd, nproto::connection* c)
{
	forEachWatcher4(&IProxyEvent::onProxyArrive, net2app(cmd.method), cmd.code, net2app(cmd.retStr), cmd.idd);
}

void CProxyElem::setManager(CProxyManager* mgr)
{
		__proxyMgr = mgr;
}

xstring	CProxyElem::getMethod()		const
{
	return __method;
}
