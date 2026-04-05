#include "sessionmanager.h"
#include "protocol/psession.h"
//#include "common/core/form.h"
//#include "common/core/ilink.h"
#include "common/res_code.h"
#include "common/nsox/nulog.h"

#include "SessionList.h"
#include "helper/utf8to.h"
#include "SessionImp.h"
#include "SessionMemberListImp.h"
#include "MicrophoneListImp.h"

#include "protolink.h"

using namespace protocol::session;
using namespace protocol::link;
using namespace core;
using namespace protocol;
//using namespace misc;
using namespace xproto;
#define LBS_MAX_TIME 2
#define LONG_SEREC_TIME_OUT 60 * 1000   // 因為要等待登陸流程，所以設置更長的時候來重連
#define SELBS_TIME_OUT 30 * 1000
#define SEREC_TIME_OUT 20 * 1000

BEGIN_REQUEST_MAP_EXT(SessionManager)
	//REQUEST_HANDLER(XCast<PCreateChannelRes>,	onCreate)
	REQUEST_HANDLER(XCast<PSessionLbsRes>,		onJoin)
	//REQUEST_HANDLER(XCast<PDismissChannel>,		onDismiss)
	//REQUEST_HANDLER(XCast<PFindResult>,			onFindResult)
	//REQUEST_HANDLER(XCast<POnGetNews>,			onGetNews)
END_REQUEST_MAP()

SessionManager::SessionManager()
{
	pUInfo = NULL;
	initialCloseSession = false;
	newsIndex = -1;
	__protoLink = NULL;
	sessionLogTimer.init(this, &SessionManager::onSessionTimeOut);
	UserInfoChangeTimer.init(this, &SessionManager::UInfoChangeTimeout);
	_sl_status = e_init;
	_relbstime = 0;
}



SessionManager::~SessionManager(void)
{
}

void SessionManager::setSessionList(slist::SessionList* pl)
{
	pSList = pl;
}

void SessionManager::updateSlist(uint32_t sid, const SItem &s)
{
	if (pSList) {
		pSList->updateSinfo(sid, s);
	}
}

void SessionManager::updateMyList(const SID_T& sid, bool badd)
{
	if (pSList) {
		pSList->updateMyList(sid, badd);
	}
}

void SessionManager::setProtoLink(CProtoLink* protoLink)
{
		__protoLink = protoLink;
}

ISession *SessionManager::join(const SID_T &sid, const xstring& chPasswd, ISessionHandler *h){

	sessionLogTimer.stop();
	if(!sessionLogTimer.hasStart()){
				sessionLogTimer.start(SELBS_TIME_OUT);
	}


	nsox::xlog(NsoxInfo, "begin to join session %d .....................................(1)", sid);
	if(sid != 0){
		__session = nsox::nu_create_object<SessionImp>::create_inst(); 
		__session ->SetManager(this);
		xstring token = __protoLink->getLoginSessionToken();
		if (!__protoLink->getPasswd().empty()) {
			__session ->Init(sid,__protoLink->getUid(), __protoLink->getPasswd(), chPasswd, h, __protoLink->getVersion());
		} else if (!token.empty()) {
			__session ->InitByToken(sid,__protoLink->getUid(), token,  chPasswd, h, __protoLink->getVersion());
		} else {
			__session ->Init(sid,__protoLink->getUid(), __protoLink->getPasswd(), chPasswd, h, __protoLink->getVersion());
		}
			
		SID_T asid = pSList->getAliaseId(sid);
		if(!asid){
			__session->SetAliasId(asid);
		}
		_sl_status = e_lbs;
		_relbstime = 0;
		XCast<PSessionLbs> lbs;
		lbs.sid = sid;
		lbs.type = ISP_US;

		sendRequest(lbs);

		h->onStatus(ISessionHandler::JOIN1, __session);
		__session->InitReportTimes();
	}
	
	return __session;
}

ISession* SessionManager::GetJoined() {
	return __session;
}

std::string urlEncode(const std::string& m) {
	std::string c;
	const char* ptr=m.c_str();
	char buf[5];
	for (; *ptr != 0; ++ptr) {
		unsigned char ch = (unsigned char)(*ptr);
		if (isalnum(ch) || ch == '.')
			c.append(1, (char)ch);
		else {
			sprintf(buf, "%%%02x", ch);
			c.append(buf);
		}
	}
	return c;
}

void SessionManager::onRefresh(){
	std::string nn = app2net(pUInfo->getNick());
	uinfo::SEX ng = pUInfo->getSex();
	std::string ns = app2net(pUInfo->getSign());

	if(nn != nick || ng != gender || ns != sign){
		UserInfoChangeTimer.start(4000);
		nick = nn;
		gender = ng;
		sign = ns;
		/*
		if(__session){
				__session ->ChangeNick(nn, ng, ns);
		}
		*/
	}
}

void SessionManager::UInfoChangeTimeout() {
	UserInfoChangeTimer.stop();
	std::string nn = app2net(pUInfo->getNick());
	uinfo::SEX ng = pUInfo->getSex();
	std::string ns = app2net(pUInfo->getSign());

	if(__session){
		__session ->ChangeNick(nn, ng, ns);
	}
}

void SessionManager::changeSign(xstring& strSign)
{
	//UserInfoChangeTimer.start(4000);
}


void SessionManager::eraseSession(){
	newsIndex = -1;
	__session = NULL;
	setInitialClose(false);
}

void SessionManager::setInitialClose(bool b){
	initialCloseSession = b;
}

bool SessionManager::reconnect(bool bForce){
	if(__session && (_sl_status != e_succ || bForce)){
		sessionLogTimer.stop();
		if(!sessionLogTimer.hasStart()){
			if (bForce == false) {
				sessionLogTimer.start(SEREC_TIME_OUT);
			} else {
				sessionLogTimer.start(LONG_SEREC_TIME_OUT);
			}	
		}
		_sl_status = e_relbs;
		_relbstime++;
		XCast<PSessionLbs> lbs;
		lbs.sid = __session ->GetSessionId();
		lbs.type = ISP_US;
		nsox::xlog(NsoxInfo, "before send session lbs");
		//pSessCtx->registerSession(sess);
		sendRequest(lbs);
		__session->InitReportTimes();
		return true;
	}
	return false;
}

bool SessionManager::leaveGuild(uint32_t sid, ILeaveWatcher *w){
	assert(false);
	return false;
}

/*
std::pair<xstring, xstring> SessionManager::getNextNews(){
	if(newsIndex == -1){
		XCast<PGetNews> gn;
		sendRequest(gn);
	}else{
		if(news.size() > 0){
			newsIndex = (++newsIndex) % news.size();
			return news[newsIndex];
		}
	}
	return make_pair(xstring(), xstring());

}
*/

void SessionManager::initialEraseSession(ISession *s){
	initialCloseSession = true;
	eraseSession();
}

void SessionManager::setUInfo(xproto::IUserInfo *info) {

	if(pUInfo){
		((IConnPoint<IUserEvent>*)pUInfo)->Revoke(this);
	}
	pUInfo = info;
	if(pUInfo){
		((IConnPoint<IUserEvent>*)pUInfo)->Watch(this);
	}

}

void SessionManager::shutdown(){
	assert(false);
}

void SessionManager::onJoin(XCast<PSessionLbsRes>& cmd ,nproto::connection* c)
{
	//times_[1] = ::timeGetTime();
	if (__session == NULL) {
		return;
	}
	__session->SetTime(1);
	sessionLogTimer.stop();
	_sl_status = e_succ;
	_relbstime = 0;
	if(!core::Request::ifSuccess(cmd.resCode)){
		nsox::xlog(NsoxInfo, "session manager joined faild, errno :%d", cmd.resCode);
		if(__session){
			if (cmd.resCode == RES_ENONEXIST) {
				__session->NotifySessionStatus(ISessionHandler::NOTEXIST);
			}  else if (cmd.resCode == RES_ERETRY){
				__session->NotifySessionStatus(ISessionHandler::ERR_RETYR);
			}else {
				__session->NotifySessionStatus(ISessionHandler::INVALID);
			}
			initialEraseSession(__session);
		}

	}else{
			nsox::xlog(NsoxInfo, "session manager joined ......................................(2)");
			uint32_t sid = cmd.sid;
			uint32_t asid = cmd.asid;

			if(__session){
				__session->SetSessionId(sid);
				__session->SetAliasId(asid);

				if(cmd.ports.empty()){
						nsox::xlog(NsoxError, "server error ports is empty!");
						__session->NotifySessionStatus(ISessionHandler::NETBROKEN);
						return;
				}
				__session->NotifySessionStatus(ISessionHandler::JOIN2);

				if (cmd.ports.size() && cmd.ports[1] > 10000) {
					__session->SetToken(__protoLink->getLoginSessionToken());
				}
				if(__session->Start(cmd.ip, cmd.ports))
				{
					//TODO
					//	__session->NotifySessionStatus();
				}
			}
	}
}

void SessionManager::onSessionTimeOut()
{
		sessionLogTimer.stop();
		if (_relbstime < LBS_MAX_TIME) { // 重連時，lbs最多走LBS_MAX_TIME次
			reconnect();
		} else {                         //多次失敗后，將退出
			nsox::xlog(NsoxInfo, "session manager joined faild, timeout.");
			if(__session){
					__session->NotifySessionLbsError(ISessionHandler::NETBROKEN);
					initialEraseSession(__session);
			}
		}
	
}
