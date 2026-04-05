#include "raiduser.h"
#include "ProtoLink.h"
#include "protocol/pkg_tag.h"
#include "common/iproperty.h"
#include "common/core/request.h"
#include "helper/utf8to.h"

#include "OpenPlatformInfo.h"

using namespace xproto;

CRaidUserManager::CRaidUserManager()
{
		__protoLink = NULL;
}

CRaidUserManager::~CRaidUserManager(void)
{
}
void CRaidUserManager::setProtoLink(CProtoLink* link)
{
		__protoLink = link;
}

IUserInfo*  CRaidUserManager::getUserInfo(const UID_T &uid)
{
	nsox::nu_auto_ptr<CRaidUser>  user = __users[uid];
	if(user == NULL){
		user = nsox::nu_create_object<CRaidUser>::create_inst();	
		user->__uid = uid;
		user->setManager(this);
		__users[uid] = user;
	}
	return user;
}

IUserInfo*  CRaidUserManager::getUserInfo(const ACCOUNT_T &account)
{
	account2uid_map::iterator it = __trans.find(account);
	if (it != __trans.end()) {
			return getUserInfo(it->second);
	}

	nsox::nu_auto_ptr<CRaidUser>  user = __accounts[account];
	if(user == NULL){
		user = nsox::nu_create_object<CRaidUser>::create_inst();	
		user->__uid = 0;
		user->__account = account;
		user->setManager(this);
		__accounts[account] = user;
	}
	return user;
}

void CRaidUserManager::onGetUid(XCast<PRepGetUid>& cmd, nproto::connection* c)
{
	UID_T uid = cmd.uid;
	xstring account = net2app(cmd.account);
	account_usr_map::iterator itr = __accounts.find(account);
	__trans[account] = uid;
	if(itr != __accounts.end()){
		itr->second->onGetUid(cmd, c);
		if(__users.find(uid) == __users.end()) {
			__users[uid] = itr->second;
			__accounts.erase(account);
			
		}
	}
}

void CRaidUserManager::onGetUInfo(XCast<PRepGetUInfo>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		UID_T uid = sox::properties::getint(cmd, UIT_UID);
		xstring account = net2app(sox::properties::get(cmd, UIT_ACCOUNT));
		__trans[account] = uid;
		account_usr_map::iterator itr = __accounts.find(account);
		if(itr != __accounts.end()){

			itr->second->onGetUInfo(cmd, c);

			raid_usr_map::iterator itr_2 = __users.find(uid);
			if(itr_2 == __users.end()){
				__users[uid] = itr->second;
				__accounts.erase(account);
			}else{
				nsox::xlog(NsoxError, "auto prt ?");
				__users[uid] = itr->second;
				__accounts.erase(account);
			}

		}else{
			raid_usr_map::iterator itr = __users.find(uid);
			if(itr != __users.end()){
				itr->second->onGetUInfo(cmd, c);
			}else{
				nsox::xlog(NsoxInfo, "can't locate user info, uid:%d", uid);
			}
		}
	}
}
void CRaidUserManager::onUpdated(XCast<PRepUpdateUInfo>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		raid_usr_map::iterator itr = __users.find(__protoLink->getUid());
		if(itr != __users.end()){
			itr->second->onUpdated(cmd, c);
		}else{
			//nsox::xlog(NsoxInfo, "can't locate user info, uid:%d", uid);
		}
	}
}

void CRaidUserManager::writeRequest(nproto::request* req, int uri)
{
	__protoLink->writeRequest(req, uri);
}

void CRaidUserManager::onGetUserInSess(XCast<PRepGetUserInSessRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		raid_usr_map::iterator itr = __users.find(cmd.uid);
		if(itr != __users.end()){
			itr->second->onGetUserInSess(cmd, c);
		}else{
			//nsox::xlog(NsoxInfo, "can't locate user sess, uid:%d", cmd.uid);
		}
	}
}

void CRaidUserManager::onGetEffect(XCast<PGetEffectRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		raid_usr_map::iterator itr = __users.find(cmd.uid);
		if(itr != __users.end()){
			itr->second->onGetEffect(cmd, c);
		}else{
			//nsox::xlog(NsoxInfo, "can't locate user sess, uid:%d", cmd.uid);
		}
	}
}


void CRaidUserManager::onGetEffectInfo(XCast<PGetEffectInfoRes>& cmd, nproto::connection* c)
{
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
CRaidUser::CRaidUser()
{
		__userMgr = NULL;
		__utime = 0;
		__effecttime = 0;
}
UID_T		CRaidUser::getUid()	const
{
		return getInt(UIT_UID);
}

xstring	CRaidUser::getAccount() const
{
		return getString(UIT_ACCOUNT);
}
xstring	CRaidUser::getNick()	const
{
		return getString(UIT_NICK);
}
void	CRaidUser::setNick(xstring& s)
{		
		return setString(UIT_NICK,s);
}
void	CRaidUser::setPasswd(xstring& s)
{
		return setString(UIT_PASS, s);
}
SEX CRaidUser::getSex() const
{
		return (SEX)getInt(UIT_GENDER);
}
void	CRaidUser::setSex(SEX sex)
{
		setInt(UIT_GENDER, sex);
}

long	CRaidUser::getBirthday() const
{
		return getInt(UIT_BIRTHDAY);
}
void	CRaidUser::setBirthday(long birty)
{
		setInt(UIT_BIRTHDAY, birty);
}

int		CRaidUser::getArea() const
{
		return getInt(UIT_AREA);
}
void	CRaidUser::setArea(int area)
{
		setInt(UIT_AREA, area);
}

int		CRaidUser::getProvince() const
{
		return getInt(UIT_PROVINCE);
}
void	CRaidUser::setProvince(int prov)
{
		setInt(UIT_PROVINCE, prov);
}
int		CRaidUser::getCity() const
{
		return getInt(UIT_CITY);
}
void	CRaidUser::setCity(int city)
{
		return setInt(UIT_CITY, city);
}

uint32_t CRaidUser::getUserJF() const
{
		return getInt(UIT_USERJF);
}
void	CRaidUser::refresh()
{
		uint32_t ntime = (uint32_t)time(NULL);
		if (ntime - __utime < 10) {
			// 骗客户端说，这是最新的
			XConnPoint<IUserEvent>::forEachWatcher0(&IUserEvent::onRefresh);
			return;
		}
		XCast<PGetUInfoByUid> cmd;
		cmd.uid = __uid;
		cmd.ver = UIT_VERSION;
		__userMgr->writeRequest(&cmd, PGetUInfoByUid::uri);
}
void	CRaidUser::update()
{	
	assert( __userMgr->__protoLink->getUid() == __uid);
	if (__userMgr->__protoLink->getUid() == __uid) {
		if (__setter.props.size()) {
			//updateProperty();
			XCast<PUpdateUInfo> cmd;
			cmd.props = __setter.props;
			__userMgr->writeRequest(&cmd, PUpdateUInfo::uri);
		}
	}
}

void	CRaidUser::updateUserSess(uint32_t pid)
{
	if (getPrivacy() == EPL_ABSOLUTE && pid == 1000000004 && __userMgr->__protoLink->getUid() != __uid) {
		
			//__userSess.clear();
			XConnPoint<IUserEvent>::forEachWatcher1(&IUserEvent::onJoinSessRefresh, pid);
			return;
	}
	if (__userGetSess.find(pid) == __userGetSess.end()) {
				XCast<PRepGetUserInSess> cmd;
				cmd.pid = pid;
				cmd.uid = __uid;
				__userMgr->writeRequest(&cmd, PRepGetUserInSess::uri);
	} else {
			XConnPoint<IUserEvent>::forEachWatcher1(&IUserEvent::onJoinSessRefresh, pid);
	}
}

void	CRaidUser::updateUid()
{

	//主動獲得uid
	if (!__account.empty() && __uid == 0) {
		XCast<PGetUid> cmd;
		cmd.account = app2net(__account);
		__userMgr->writeRequest(&cmd, PGetUid::uri);
	} else {
		XConnPoint<IGetUidEvent>::forEachWatcher0(&IGetUidEvent::onRefresh);
	}
}

xstring  CRaidUser::getSign()
{
		return getString(UIT_SIGN);
}
void	CRaidUser::setSign(xstring& sign)
{
		setString(UIT_SIGN, sign);
}

xstring	CRaidUser::getIntro() const
{
		return getString(UIT_INTRO);
}
void	CRaidUser::setIntro(xstring& s)
{
		return setString(UIT_INTRO, s);
}

void CRaidUser::onGetUInfo(XCast<PRepGetUInfo>& cmd, nproto::connection* c)
{
		__utime = (uint32_t)time(NULL);
		__getter.props.clear();
		sox::properties::replace(__getter, cmd);
		__uid = getUid();
		__account = getAccount();
		XConnPoint<IUserEvent>::forEachWatcher0(&IUserEvent::onRefresh);
}

void CRaidUser::onUpdated(XCast<PRepUpdateUInfo>& cmd, nproto::connection* c)
{
		__utime = (uint32_t)time(NULL);

		__setter.props.clear();

		sox::properties::replace(__getter, cmd);
		XConnPoint<IUserEvent>::forEachWatcher0(&IUserEvent::onRefresh);
}

void CRaidUser::setManager(CRaidUserManager* mgr)
{
		__userMgr = mgr;
}
void  CRaidUser::onGetUid(XCast<PRepGetUid>& cmd, nproto::connection* c)
{
	sox::properties::setint(__getter, UIT_UID, cmd.uid);
	__uid = getUid();
	__account = getAccount();
	XConnPoint<IGetUidEvent>::forEachWatcher0(&IGetUidEvent::onRefresh);
}


uint32_t CRaidUser::getUpdateTime()
{
		return __utime;
}

xstring  CRaidUser::getFbId() const
{
	return getString(UIT_FBID);
}

xstring  CRaidUser::getFbToken() const
{
	return getString(UIT_FBTOKEN);
}


const userJoinSess& CRaidUser::getUserSess() const
{
	return __userSess;
}

void CRaidUser::onGetUserInSess(XCast<PRepGetUserInSessRes>& cmd, nproto::connection* c)
{
	//__userSess.clear();
	std::set<uint32_t> sids;
	if (__userGetSess.find(cmd.pid) == __userGetSess.end()) {
			__userGetSess.insert(cmd.pid);
			for(std::vector<PSUserInSess>::const_iterator it = cmd.userInSess.begin(); it != cmd.userInSess.end(); ++it) {
					CRaidUserSess us;
					us.contribute = it->contribute;
					us.roler = it->roler;
					us.sid = it->sid;
					us.pid = cmd.pid;
					if (sids.find(it->sid) == sids.end()) {
						us.sname = net2app(it->sname);
						__userSess.push_back(us);
						sids.insert(it->sid);
					}
			}
			XConnPoint<IUserEvent>::forEachWatcher1(&IUserEvent::onJoinSessRefresh, cmd.pid);
	}
}


void CRaidUser::onGetEffect(XCast<PGetEffectRes>& cmd, nproto::connection* c)
{
	__effecttime = (uint32_t)time(NULL);
	CRaidUserEffect effect;
	__userEffects.clear();
	for (int i = 0; i < cmd.effects.size(); ++i) {
		effect.eid = cmd.effects[i].eid;
		effect.sid = cmd.effects[i].sid;
		effect.type = cmd.effects[i].type;
		effect.starttime = cmd.effects[i].start_time;
		effect.endtime = cmd.effects[i].end_time;
		__userEffects.push_back(effect);
	}
	XConnPoint<IUserEvent>::forEachWatcher0(&IUserEvent::onEffectRefresh);
	// getOpenPlatformInfo();

}

void CRaidUser::updateEffect()
{
		if ((uint32_t)time(NULL) - __effecttime < 60) {
			return;
		}
		XCast<PGetEffect> cmd;
		cmd.uid = __uid;
		__userMgr->writeRequest(&cmd, PGetEffect::uri);
}

const userEffects& CRaidUser::getEffects() const
{
		return __userEffects;
}


int CRaidUser::getGold() const
{
	return getInt(UIT_GOLD);
}

int CRaidUser::getSilver() const
{
	return getInt(UIT_SILVER);
}

int CRaidUser::getFlowers() const
{
	return getInt(UIT_FLOWER);
}


IUserInfo::PrivacyLevel CRaidUser::getPrivacy() const
{
	return (PrivacyLevel)getInt(UIT_JOIN_SESSION_PRIVACY);
}

void	CRaidUser::setPrivacy(IUserInfo::PrivacyLevel privacy)
{
	setInt(UIT_JOIN_SESSION_PRIVACY, privacy);
	updateProperty();
}

bool  CRaidUser::getVisitHistoryPrivacy() const {
	return getBool(UIT_PUBLIC_VISIT_SESSION);
}

void	CRaidUser::setVisitHistoryPrivacy(bool privacy) {
	setBool(UIT_PUBLIC_VISIT_SESSION, privacy);
	updateProperty();
}

bool  CRaidUser::getCollectSessionPrivacy() const {
	return getBool(UIT_PUBLIC_COLLECT_SESSION);
}

void	CRaidUser::setCollectSessionPrivacy(bool privacy) {
	setBool(UIT_PUBLIC_COLLECT_SESSION, privacy);
	updateProperty();
}


void CRaidUser::updateProperty()
{
	if (__setter.props.find(UIT_JOIN_SESSION_PRIVACY) == __setter.props.end()) {
			__setter.props[UIT_JOIN_SESSION_PRIVACY] = __getter.props[UIT_JOIN_SESSION_PRIVACY];
	}
	if (__setter.props.find(UIT_PUBLIC_VISIT_SESSION) == __setter.props.end()) {
		__setter.props[UIT_PUBLIC_VISIT_SESSION] = __getter.props[UIT_PUBLIC_VISIT_SESSION];
	}
	if (__setter.props.find(UIT_PUBLIC_COLLECT_SESSION) == __setter.props.end()) {
		__setter.props[UIT_PUBLIC_COLLECT_SESSION] = __getter.props[UIT_PUBLIC_COLLECT_SESSION];
	}
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

const OpenPlatformInfoVector& CRaidUser::getOpenPlatformInfo() {
	if (__userPlatformvInfos.empty()) {
		OpenPlatformInfoManager::Instance()->GetUserOpenPlatformInfo(__uid);
	}
	return __userPlatformvInfos;
}

void CRaidUser::setOpenPlatformInfo(OpenPlatformInfo info) {
	OpenPlatformInfoManager::Instance()->SetUserOpenPlatformInfo(
		__uid,
		info.platform_id,
		app2net(info.platform_user_id),
		app2net(info.platform_user_name),
		app2net(info.pf_token),
		app2net(info.pf_email),
		app2net(info.pf_url));
}

void CRaidUser::removeOpenPlatformInfo(OpenPlatformInfo info) {
		OpenPlatformInfoManager::Instance()->RemoveUserOpenPlatformInfo(
		__uid,
		info.platform_id);
}

void CRaidUser::AddPlatformInfo(xproto::OpenPlatformInfoVector info) {
	__userPlatformvInfos = info;
	XConnPoint<IUserEvent>::forEachWatcher0(&IUserEvent::onOpenPlatformRefresh);
}