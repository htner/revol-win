#include "protolink.h"
#include "login.h"
#include "proto/proto_obj_name.h"
#include "raiduser.h"
#include "sessionlist.h"
#include "sessionmanager.h"
#include "helper/utf8to.h"
#include "RaidProto.h"
#include "proxyManager.h"

using namespace xproto;
using namespace protocol::slist;
using namespace protocol::session;

BEGIN_REQUEST_MAP_EXT(CProtoLink)	
	CHAIN_REQUEST_MAP_PTR_MEMBER(__login)
	CHAIN_REQUEST_MAP_PTR_MEMBER(__userMgr)
	CHAIN_REQUEST_MAP_PTR_MEMBER(__sessionList)	
	CHAIN_REQUEST_MAP_PTR_MEMBER(__sessManager)
	CHAIN_REQUEST_MAP_PTR_MEMBER(__proxyMgr)
END_REQUEST_MAP()



CProtoLink::CProtoLink(void)
{

	__proxyMgr = nsox::nu_create_object<CProxyManager>::create_inst();
	__proxyMgr->setProtoLink(this);

	__login		= new Login(this);
	__userMgr	= nsox::nu_create_object<CRaidUserManager>::create_inst();
	__userMgr->setProtoLink(this);

	__sessionList = nsox::nu_create_object<SessionList>::create_inst();
	__sessionList->setProtoLink(this);

	__sessManager = nsox::nu_create_object<SessionManager>::create_inst();
	__sessManager->setProtoLink(this);
	__sessManager->setSessionList(__sessionList);

	__imBase = nsox::nu_create_object<CImBase>::create_inst();
	__imBase->setProtoLink(this);
	
}

CProtoLink::~CProtoLink(void)
{
	__sessionList->unbindLike();

	delete __login;
	
	__sessManager->setUInfo(NULL);
}

IProtoObject* CProtoLink::getUserMgr()
{
	return __userMgr;
}

void CProtoLink::onclose(int err, nproto::connection* c)
{
	__login->onclose(err,c);
}

void CProtoLink::oncreate(nproto::connection* c)
{
	__login->oncreate(c);
}

int	CProtoLink::login(const ACCOUNT_T& account, const xstring& pass, BOOL encrypt)
{
	__login->doLogin(account, pass, encrypt);
	return 0;
}

int	CProtoLink::login(const UID_T &uid, const xstring& pass, BOOL encrypt)
{
	__login->doLogin(uid, pass, encrypt);
	return 0;
}

int	CProtoLink::loginByToken(const UID_T &uid, const xstring& token, const xstring&  kind)
{
	__login->doLoginByToken(uid, token, kind);
	return 0;
}

int CProtoLink::loginByToken(const ACCOUNT_T& account, const xstring& token,const xstring& kind) {
	__login->doLoginByToken(account, token, kind);
	return 0;
}


void CProtoLink::logout()
{
	__login->doLogout();
	//__sessionList->unbindLike();
}
UID_T	CProtoLink::getUid()
{
	return __login->getUid();
}

xstring	CProtoLink::getPasswd()
{
    return __login->getPasswd();
}
xstring	CProtoLink::getEncryptPasswd()
{
	return getPasswd();
}
LinkStatus CProtoLink::getStatus()
{
	return __login->getStatus();
}

IUserInfo*  CProtoLink::getUinfo()
{
	assert(false);
	return NULL;
}

void CProtoLink::cancelLogin()
{
		return __login->cancel();
}

LPCSTR CProtoLink::getObjectName()
{
	return PROTO_LINK_OBJ;
}

void CProtoLink::writeRequest(nproto::request* req, int uri)
{
	__login->writeRequest(req, uri);
}

void CProtoLink::onLoginStatus(LinkStatus st)
{
	switch(st) {
	case LOGIN_SUCCESS:
	case RELOGIN_SUCCESS:
		{
			reConnSession();
			break;
		}
	default:
		break;
	}
	 XConnPoint<ILinkEvent>::forEachWatcher2(&(ILinkEvent::onLinkStatus), st, 0);
}

void CProtoLink::proxyNotice(int type, int resources, const xstring& info)
{
	 XConnPoint<IProxyNoticeEvent>::forEachWatcher3(&(IProxyNoticeEvent::onProxyNotice), type, resources, info);
}

void CProtoLink::onImResponse(protocol::im::PZImResponse &imRes) {
	__imBase->onImResponse(imRes);
}

IProtoObject* CProtoLink::getSessList()
{
	
    return __sessionList;
}

IProtoObject* CProtoLink::getImBase()
{
    return __imBase;
}

void CProtoLink::reConnSession()
{
	__sessManager->setOwner(getUid(), app2net(getPasswd()));
	IUserInfo *usrInfo = __userMgr->getUserInfo(getUid());
	usrInfo->refresh();

	__sessionList->sync();
	__sessManager->setUInfo(usrInfo);
	__sessManager->reconnect(); 
}
	

xstring CProtoLink::getSessKey()
{
	return __login->getSessKey();
}


void CProtoLink::onUpdateSessKey(const xstring& key) 
{
	 XConnPoint<ILinkEvent>::forEachWatcher1(&(ILinkEvent::onUpdateSessKey), key);
}

ACCOUNT_T CProtoLink::getAccount()
{
	return __login->getAccount();
}

IProtoObject*  CProtoLink::getProxyMgr()
{
	return __proxyMgr;
}

void CProtoLink::setMachineCode(const xstring& code)
{
		__login->setMachineCode(code);
}

xstring CProtoLink::geytMachineCode() const
{
		return  __login->getMachineCode();
}

 void CProtoLink::setClientVersion(uint32_t version)
{
		return __login->setClientVersion(version);
}

uint32_t CProtoLink::getClientVersion() const
{
		return __login->getClientVersion();
}

uint32_t	CProtoLink::getVersion()
{
		return getClientVersion();
}

void CProtoLink::setLbsConfig(ILbsConfig* config)
{
		__login->setLbsConfig(config);
}

xstring CProtoLink::getLoginSessionToken() {
		return __login->getLoginSessionToken();
}

void CProtoLink::SetAddrConfig(IAddrConfig* config) {
	__login->SetAddrConfig(config);
}

xproto::IAddrConfig* CProtoLink::GetAddrConfig() {
	return __login->GetAddrConfig();
}

Login*  CProtoLink::GetLogin() {
	return __login;
}