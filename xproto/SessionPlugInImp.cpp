#include "SessionPlugInImp.h"
#include "protocol/pkg_tag.h"
#include "common/core/request.h"
#include "helper/utf8to.h"
#include "sessionimp.h"
#define BROAD_PI 0

using namespace protocol::plugin;
using namespace  protocol::session;
using namespace xproto;

SessionPlugInImp::SessionPlugInImp(void)
{
}

SessionPlugInImp::~SessionPlugInImp(void)
{
}

void SessionPlugInImp::send(const std::string& data)
{
	XCast<PFeatureRequest> cmd;
	cmd.data = data;

	sox::PackBuffer pb;
	sox::Pack pk(pb);
	cmd.URI = cmd.uri;
	cmd.pack(pk);
	pk.replace_uint32(0, pk.size());

	__pluginMgr->send(pb.data(), pb.size(), __type);
}

void SessionPlugInImp::onFeatureResPonse(XCast<PFeatureResponse>& cmd, nproto::connection* c)
{
	forEachWatcher2(&ISessionPlugEvent::onData, cmd.type, cmd.data);
}

void SessionPlugInImp::onFeatureBroadCast(XCast<PFeatureBroadCast>& cmd, nproto::connection* c)
{
	//這個是零號專用接口
	forEachWatcher2(&ISessionPlugEvent::onData, cmd.type, cmd.desc);
}

void SessionPlugInImp::setManager(CPlugInManager* mgr)
{
		__pluginMgr = mgr;
}

void SessionPlugInImp::setType(uint32_t type)
{
	__type = type;
}

void SessionPlugInImp::onPlugChange(uint32_t type,  bool open)
{
	forEachWatcher2(&ISessionPlugEvent::onPlugChange, type, open);
}

CPlugInManager::CPlugInManager()
{
}

CPlugInManager::~CPlugInManager(void)
{
}

void CPlugInManager::onPlugData(XCast<protocol::session::PPlugToClient>& ptClient, nproto::connection* c)
{
	int		plen = nproto::request::peek_len(ptClient.cmd.c_str());
	uint32_t uri = nproto::request::peek_uri(ptClient.cmd.c_str());
	int svid = (uri & 0xff);
	if (svid == protocol::PLUGIN_SVID) //只轉這個類型的消息
	{  //如果再有新的指令，就用宏來實現
		if (uri == XCast<PFeatureResponse>::uri) { 
			XCast<PFeatureResponse> pro; 
			sox::Unpack upk(ptClient.cmd.c_str(), plen);	
			try{
	        		pro.unpack(upk);
			}catch(sox::UnpackError& err){ 
					nsox::xlog(NsoxDebug, "unpack request error , uri:%d", uri);\
			}
			pro.type = ptClient.PIType; //不允許插件自己定義
			onFeatureResPonse(pro, c);
		} else if (uri == XCast<PFeatureBroadCast>::uri) {
			XCast<PFeatureBroadCast> pro; 
			sox::Unpack upk(ptClient.cmd.c_str(), plen);	
			try{
	        		pro.unpack(upk);
			}catch(sox::UnpackError& err){ 
					nsox::xlog(NsoxDebug, "unpack request error , uri:%d", uri);\
			}
			pro.type = ptClient.PIType; //不允許插件自己定義
			onFeatureBroadCast(pro, c);
		}
     }
}

void CPlugInManager::onFeatureResPonse(XCast<PFeatureResponse>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)) {
		plugin_map::iterator itr = __plugins.find(cmd.type);
		if(itr != __plugins.end()){
			itr->second->onFeatureResPonse(cmd, c);
		}
	}
}

void CPlugInManager::onFeatureBroadCast(XCast<PFeatureBroadCast>& cmd, nproto::connection* c)
{
	if (core::Request::ifSuccess(cmd.resCode)) {
		plugin_map::iterator itr = __plugins.find(BROAD_PI);
		if(itr != __plugins.end()){
			itr->second->onFeatureBroadCast(cmd, c);
		}
	}
}

ISessionPlugBase* CPlugInManager::getPlugIn(uint32_t type)
{
	nsox::nu_auto_ptr<SessionPlugInImp>  pi = __plugins[type];
	if(pi == NULL){
		pi = nsox::nu_create_object<SessionPlugInImp>::create_inst();	
		pi->setType(type);
		pi->setManager(this);
		__plugins[type] = pi;
	}
	return pi;
}

void CPlugInManager::send(const char* data, uint32_t size,  uint32_t type)
{
	XCast<PForwardToPlug> forward;
	forward.cmd.assign(data, size);
	forward.PIType = type;
	pSession->SendNprotoRequest(forward);
}


void CPlugInManager::setSession(SessionImp* s)
{
	pSession = s;
}

bool  CPlugInManager::isPlugInOpen(uint32_t type) const
{
	return __opens.find(type) != __opens.end();
};


void CPlugInManager::openChange(const xstring& str)
{
		int num;
		plugin_set new_pls;
		plugin_set old_pls;
		uint32_t pi = 0;
		size_t size = str.size();
		for (int i = 0; i < size; ++i)
		{
			num = str[i] - (int)('0');
			if (num >= 0 && num < 10)
			{
				pi = pi * 10 + num;
				if (i == size - 1) {
					new_pls.insert(pi);
				}
			} else {
				new_pls.insert(pi);
				pi = 0;
			}
		}
		plugin_set::iterator it = new_pls.begin();
		while(it != new_pls.end()) {
			if (__opens.find(*it) == __opens.end()) {
				__opens.insert(*it);
				plugin_map::iterator itr = __plugins.find(0);
				if(itr != __plugins.end()){
					itr->second->onPlugChange(*it, true);
				}
			}
			it++;
		}

		it = __opens.begin();
		while(it != __opens.end()) {
			if (new_pls.find(*it) == new_pls.end()) {
				old_pls.insert(*it);
			}
			it++;
		}

		it = old_pls.begin();
		while(it != old_pls.end()) {
			__opens.erase(*it);
			it++;
		}
		plugin_map::iterator itr = __plugins.find(0);
		if(itr != __plugins.end()){
			it = old_pls.begin();
			while(it != old_pls.end()) {
					itr->second->onPlugChange(*it, false);
					it++;
			}
		}
}