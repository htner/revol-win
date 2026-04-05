#include "sessioninfo.h"
#include "protocol/pkg_tag.h"
#include "sessionimp.h"
#include "helper/utf8to.h"
#include "sessionmemberlistimp.h"
#include "OpenPlatformInfo.h"

using namespace xproto;
using namespace protocol::session;

/************************************************************************/
/* SessionInfo Implemention                                             */
/************************************************************************/


SessionInfo::SessionInfo()
{
	isSess = false;
	get_extend_ = false;
}
SessionInfo::~SessionInfo()
{

}

void SessionInfo::setProperty(sox::Properties &p)
{
	sox::properties::replace(__getter, p);
}

void SessionInfo::setSession(SessionImp* sess)
{
	pSession = sess;
}

void SessionInfo::setName(const xstring &n){
	setString(SIT_NAME, n);
}

void SessionInfo::setPub(bool p){
	setBool(SIT_ISPUB, p);
}

void SessionInfo::setLimit(bool l){
	setBool(SIT_ISLIMIT, l);
}

void SessionInfo::setBulletin(const xstring &hello){
	setString(SIT_BULLETIN, hello);
}

void SessionInfo::setMicTime(int t){
	setInt(SIT_MICROTIME,t);
}

void SessionInfo::SetIsp(ISPType ntype)
{
	setInt(SIT_ISPTYPE,ntype);
}

//走 list 通道
void SessionInfo::changeMemberRight(const UID_T &uid, ChannelRoler nr){
	assert(0);
	//pSession->ChangeRoler(uid, nr, getSid());
}

void SessionInfo::update(){
	if(!__setter.props.empty()){
		if (isSess == true) {
			pSession->UpdateSessionInfoRequest(__setter);
		} else {
			pSession->UpdateChannelInfoRequest(innerGetSid(), __setter);
		}
		__setter.props.clear();
	}
}

std::vector<MiniInfo> SessionInfo::getManager() const{
	std::vector<MiniInfo> ret;
	for(MemberMap::const_iterator it = members.begin(); it != members.end(); ++it){
		if(it->second.role >= CMANAGER){
			ret.push_back(it->second);
		}
	}
	return ret;
}
//xxx bug
//std::vector<MiniInfo> SessionInfo::getMember() const{
//	std::vector<MiniInfo> ret;
//	for(std::map<UID_T, MiniInfo>::const_iterator it = members.begin(); it != members.end(); ++it){
//		if(it->second.role == MEMBER){
//			ret.push_back(it->second);
//		}
//	}
//	return ret;
//}

void SessionInfo::onInfoChange(){
	XConnPoint<ISInfoEvent>::forEachWatcher0(&ISInfoEvent::onChange);
}


void SessionInfo::setMemberLimited(uint32_t ml){
	setInt(SIT_MEMBERLIMIT, ml);
}

void SessionInfo::setPass(const std::string &md5Pass){
	//alert
	setBString(SIT_LIMITPASSWD, md5Pass,true);
}

void SessionInfo::setArea(int n){
	setInt(UIT_AREA, n);
}


void SessionInfo::setProvince(int n){
	setInt(UIT_PROVINCE, n);
}


void SessionInfo::setCity(int n){
	setInt(UIT_CITY, n);
}


void SessionInfo::setType(GuildType type){
	setInt(SIT_TYPE, type);
}

void SessionInfo::setTypeString(const xstring &ts){
	setString(SIT_TYPESTRING, ts);
}


uint32_t SessionInfo::innerGetSid() const{
	return getInt(SIT_SID);
}

void SessionInfo::Watch(ISessMgrWatcher *w){
	if(!XConnPoint<ISessMgrWatcher>::Exist(w))
	{
		XConnPoint<ISessMgrWatcher>::Watch(w);
		if (members.empty() && pSession->port_ < 10000) {
			XCast<PSyncManagers> sm;
			sm.cid = this->getSid();
			pSession->SendNprotoRequest(sm);
		} else if (pSession->port_ >= 10000){
			members.clear();
			SessionImp::UserRolers& user_roles = pSession->user_rolers_;
			for(SessionImp::UserRolers::const_iterator it = user_roles.begin(); it != user_roles.end(); ++it) {
				for(int i = 0; i < it->second.size(); ++i) {
					const RolerPair& roler_pair = it->second[i];
					if (roler_pair.roler == protocol::session::CMANAGER && roler_pair.channelId == innerGetSid()) {
						MiniInfo info;
						info.gender = pSession->getSessionList()->getSex(it->first);
						info.role = protocol::session::CMANAGER;
						info.nick = pSession->getSessionList()->getNick(it->first);
						info.uid = it->first;
						members[it->first] = info;
					}
				}
			}
			refreshManager();
		}
	}
}


void SessionInfo::Watch(IBanWatcher* w)
{

    if(!XConnPoint<IBanWatcher>::Exist(w))
    {
        XConnPoint<IBanWatcher>::Watch(w);
        w->onRefresh();

        XCast<PGetBans> gb;
        pSession->SendNprotoRequest(gb);
    }
}


void SessionInfo::onGetManagers(POnSyncManagers *sm){
	members.clear();
	for(std::vector<PRolerMini>::iterator it = sm->managers.begin(); it != sm->managers.end(); ++it){
		MiniInfo &mm = members[it->uid];
		mm.uid = it->uid;
		if(it->role > mm.role){
			mm.nick = net2app(it->nick);
			mm.gender = it->gender;
			mm.role = it->role;
		}
	}

	refreshManager();
}

void SessionInfo::refreshManager(POnMemberUpdated *upd){

	switch(upd->op){
			case POnMemberUpdated::ADD:
				if(innerGetSid() == upd->channelId)
					onRolerAdd(upd->uid, upd->channelId, upd->roler, net2app(upd->nick), upd->sex);
				break;
			case POnMemberUpdated::REMOVE:
				if(members.find(upd->uid) != members.end())
					onRolerRemove(upd->uid, upd->channelId);
				break;
			case POnMemberUpdated::CHANGE:
				if(members.find(upd->uid) != members.end() || innerGetSid() == upd->channelId)
					onRolerChange(upd->uid, upd->channelId, upd->roler, net2app(upd->nick), upd->sex);
				break;
	}
	refreshManager();
}

void SessionInfo::refreshManager(){

	XConnPoint<ISessMgrWatcher>::forEachWatcher0(&ISessMgrWatcher::onRefresh);
}

void SessionInfo::onRolerAdd(const UID_T &uid, 
							 uint32_t channelId, 
							 ChannelRoler r,
							 const xstring &nick, 
							 protocol::uinfo::SEX gender)
{
	MiniInfo &mm = members[uid];

	mm.uid = uid;
	if((channelId == innerGetSid() || channelId == pSession->GetSessionId()) && r > mm.role){
		mm.nick = nick;
		mm.gender = gender;
		mm.role = r;
	}
}

void SessionInfo::onRolerRemove(const UID_T &uid, uint32_t channel)
{
	if(channel == innerGetSid()){
		//xxx todo
		members.erase(uid);
	}
}

void SessionInfo::onRolerChange(const UID_T &uid,
								uint32_t channel, 
								ChannelRoler r, 
								const xstring &nick,
								protocol::uinfo::SEX gender)
{
	if(r < CMANAGER){
		members.erase(uid);
	}else if(r >= CMANAGER && channel == innerGetSid()){
		onRolerAdd(uid, channel, r, nick, gender);
	}
}

void SessionInfo::updateManagerErr(uint32_t resCode)
{
	XConnPoint<ISessMgrWatcher>::forEachWatcher1(&ISessMgrWatcher::onError, resCode);
}

void SessionInfo::unbanID(const UID_T &uid) {
	//std::vector<uint32_t> uids;
	//uids.push_back(uid);
	//batchUnbanID(uids);
	//return;
	XCast<PUpdateBanID> bu;
	bu.uid = uid;
	bu.isBan = false;
	
	pSession->SendNprotoRequest(bu);
}

void SessionInfo::unbanIP(uint32_t ip) {
	XCast<PUpdateBanIp> bu;
	bu.ip = ip;
	bu.isBan = false;
	pSession->SendNprotoRequest(bu);
}

std::vector<BanUserMiniInfo> SessionInfo::getBanIDs() const{
	std::vector<BanUserMiniInfo> rids;
	for(BanIdVecotr::const_iterator it = banIds.begin(); it != banIds.end(); ++it){
		BanUserMiniInfo mi;
		mi.account = net2app(it->account);
		mi.uid = it->uid;
		mi.nick = net2app(it->nick);
		mi.reason = net2app(it->reason);
		mi.gender = it->gender;
		rids.push_back(mi);
	}

	return rids;
}

std::vector<BanIpMiniInfo> SessionInfo::getBanIPs() const{
	std::vector<BanIpMiniInfo> rips;
	for(BanIpMinVector::const_iterator it = banIps.begin(); it != banIps.end(); ++it){
		BanIpMiniInfo mi;
		mi.ip = it->ip;
		mi.uid = it->uid;
		mi.nick = net2app(it->nick);
		mi.gender = it->gender;
		rips.push_back(mi);
	}

	return rips;
}

template<class T1,class T2>
void copyVector(const T1& from,T2& to)
{	
	to.clear();
	to.reserve( from.size() );
	T1::const_iterator it1 = from.begin();
	T1::const_iterator it2 = from.end();
	for(; it1 != it2; it1++){
		to.push_back( *it1 );
	}
}

void SessionInfo::updateBans(std::vector<PBanUserMini> &ids, std::vector<PBanIpMini> &ips){
	//banIds = ids;
	//banIps = ips;
	copyVector(ids,banIds);
	copyVector(ips,banIps);

	XConnPoint<IBanWatcher>::forEachWatcher0(&IBanWatcher::onRefresh);
}


void SessionInfo::banUser(const SID_T &uid, bool isBan, const ACCOUNT_T& account){
	if(!isBan){
		if (uid == 0) {
			banIds.clear();
		} else {
			for(BanIdVecotr::iterator it = banIds.begin(); it != banIds.end(); ++it){
				if(it->uid == uid){
					banIds.erase(it);
					break;
				}
			}
		}
	}else{
		bool bFound = false;				
		for(BanIdVecotr::iterator it = banIds.begin(); it != banIds.end(); it++)
		{
			if(it->uid == uid)
			{
				bFound = true;
				it->nick = app2net(pSession->getSessionList()->getNick(uid));
				it->gender = pSession->getSessionList()->getSex(uid);				
				break;
			}
		}		
		if( !bFound )
		{
			PBanUserMini mini;
			mini.uid = uid;
			mini.nick = app2net(pSession->getSessionList()->getNick(uid));
			mini.gender = pSession->getSessionList()->getSex(uid);
			mini.account = app2net(account);
			banIds.push_back(mini);
		}
	}

	XConnPoint<IBanWatcher>::forEachWatcher0(&IBanWatcher::onRefresh);
}

void SessionInfo::banIp(uint32_t ip, bool bBan){
	if(!bBan){
		if (ip == 0) {
			banIps.clear();
		} else {
			for(BanIpMinVector::iterator it = banIps.begin(); it != banIps.end(); ++it){
				if(it->ip == ip){
					banIps.erase(it);
					break;
				}
			}
		}
	}else{
		PBanIpMini banip;
		banip.ip = ip;
		banIps.push_back(banip);
	}
	XConnPoint<IBanWatcher>::forEachWatcher0(&IBanWatcher::onRefresh);
}


void SessionInfo::batchBanUser(const std::vector<HULI> &uids, bool isBan)
{
	if(!isBan){
		uint32_t uid = 0;
		for(int i = 0; i <  uids.size(); ++i) {
				uid = uids[i];
				for(BanIdVecotr::iterator it = banIds.begin(); it != banIds.end(); ++it){
					if(it->uid == uid){
						banIds.erase(it);
						break;
					}
				}
		}
	}else{
		assert(0);
	}
	XConnPoint<IBanWatcher>::forEachWatcher0(&IBanWatcher::onRefresh);
}

void SessionInfo::batchBanIp(const std::vector<uint32_t> &ips, bool isBan)
{
	if(!isBan){
		uint32_t ip = 0;
		for(int i = 0; i <  ips.size(); ++i) {
				ip = ips[i];
				for(BanIpMinVector::iterator it = banIps.begin(); it != banIps.end(); ++it){
					if(it->ip == ip){
						banIps.erase(it);
						break;
					}
				}
		}
	}else{
		assert(0);
	}
	XConnPoint<IBanWatcher>::forEachWatcher0(&IBanWatcher::onRefresh);
}

SID_T SessionInfo::getAliaseId() const{
	return SessionInfoBase::getAliaseId();

	/*SID_T ss = getSid();
	if(pSession->getRealSid() == getSid()){
		return pSession->getAliasSidi();
	}else{
		if(ss == 0){
			return pSession->getAliasSidi();
		}else{
			return ss;
		}
	}
	*/
}

void SessionInfo::setIntroduce(const xstring& intro){
	setString(SIT_INTRODUCE, intro);
}

void SessionInfo::setVoiceQC(VoiceQC qc){
	setInt(SIT_VOICEQC, qc);
}

void SessionInfo::setStyle(ChannelStyle style){
	setInt(SIT_STYLE, style);
}

void SessionInfo::setOperator(const UID_T &uid){
	setInt(SIT_INFO_OPERATOR, uid);
}



void  SessionInfo::getKickOffs(const SID_T &sid, std::vector<SKickData>& kicks,  std::map<UID_T, MiniInfo>& kinfos) const
{
	kickoff_map_t::const_iterator itks = kickOffs.find(sid);
	if (itks!= kickOffs.end()) {
		//kicks = itks->second;
		copyVector(itks->second,kicks);
		for(std::vector<SKickData>::const_iterator it = kicks.begin(); it != kicks.end(); ++it){
			KickOffMap::const_iterator itm = kickOffInfos.find(it->uid);
			if (itm != kickOffInfos.end()) {
				MiniInfo mi;
				mi.uid = itm->second.uid;
				mi.nick = net2app(itm->second.nick);
				mi.gender =itm->second.gender;
				kinfos.insert(std::make_pair(it->uid, mi));				
			}
		}
	}
}

void  SessionInfo::updateKickOffs(const SID_T& sid, std::vector<PListMini> &kickoffinfo, PUpdateKickOffRes::kuser_map_t& kuser) 
{

	for(int i = 0; i < kickoffinfo.size(); ++i)
	{
		kickOffInfos[kickoffinfo[i].uid] = kickoffinfo[i];
	}

	kickOffs.erase(sid);
	for (PUpdateKickOffRes::kuser_map_t::const_iterator it =  kuser.begin();
		   it  != kuser.end(); it++) {
					SKickData data;
					data.uid = it->first;
					const kick_vec_t& vec  =  it->second.vec;
					for(int i = 0; i < vec.size(); ++i ) {
						data.tick =vec[i].tick;
						data.sid = vec[i].sid;
						data.aid = vec[i].admin;
						data.op_time = vec[i].op_time;
						data.reason = net2app(vec[i].reason);
						kickOffs[sid].push_back(data);
					}
	}
	XConnPoint<IKickOffWatcher>::forEachWatcher0(&IKickOffWatcher::onRefresh);
}

void SessionInfo::Watch(IKickOffWatcher* w)
{
    if(!XConnPoint<IKickOffWatcher>::Exist(w))
    {
        XConnPoint<IKickOffWatcher>::Watch(w);
        w->onRefresh();

		XCast<PUpdateKickOff> req;
		req.cid = 0;
        pSession->SendNprotoRequest(req);
    }
}

void SessionInfo::removeKickOff(const UID_T &uid, const SID_T &sid)
{
	if (kickOffs.find(sid) != kickOffs.end()) {
		for(KickDataVector::iterator it = kickOffs[sid].begin();  it != kickOffs[sid].end();  it++){
			if(it->uid == uid){
				kickOffs[sid].erase(it); 
				break;
			}
		}
	}

	if (kickOffs.find(0) != kickOffs.end()) {
		for(KickDataVector::iterator it = kickOffs[0].begin();  it != kickOffs[0].end();  it++){
			if(it->uid == uid){
				kickOffs[0].erase(it); 
				break;
			}
		}
	}
	XConnPoint<IKickOffWatcher>::forEachWatcher0(&IKickOffWatcher::onRefresh);
}

void SessionInfo::addKickOff(const UID_T &uid, const SID_T &sid)
{
		kickoff_map_t::iterator itks = kickOffs.find(sid);
		if (itks!= kickOffs.end()) {
			kickOffs[sid];
			itks = kickOffs.find(sid);
		}
		if (itks!= kickOffs.end()) {		
			SKickData data;
			data.tick = 0;
			data.sid = sid;
			data.uid = uid;
			kickOffs[sid].push_back(data);
		}
		PListMini mini;
		mini.uid = uid;
		mini.nick = app2net(pSession->getSessionList()->getNick(uid));
		mini.gender = pSession->getSessionList()->getSex(uid);	
		kickOffInfos[uid] = mini;
		XConnPoint<IKickOffWatcher>::forEachWatcher0(&IKickOffWatcher::onRefresh);
}

void SessionInfo::setTxtInterval(int n)
{
	setInt(SIT_VTXTINTERVAL, n);
}

void SessionInfo::setTxtInitWait(int n)
{
	setInt(SIT_VTXTINITALWAIT, n);
}

void SessionInfo::setTxtMaxLength(int len)
{
	if (getTxtMaxLength() != len) {
			setInt(SIT_VTXTLENGTH, len);
			setOperator(pSession->GetUid());
	}
}


void SessionInfo::setTxtLimitFormat(TxtFormatLimit tfl)
{
	setInt(SIT_VTXTFORMAT, tfl);
}


void SessionInfo::setMicLimitQueue(bool b)
{
	setBool(SIT_VMICLIMITQUEUE, b);
}
void SessionInfo::setMicLimitFree(bool b)
{
	setBool(SIT_VMICLIMITFREE, b);
	
}

void SessionInfo::setApplyInfo(const xstring &info)
{
	setString(SIT_APPLYINFO, info);
}

void SessionInfo::setApplyUjifen(int jifen)
{
	setInt(SIT_APPLYUJIFEN, jifen);
}

void SessionInfo::setApplyUsjifen(int jifen)
{
	setInt(SIT_APPLYUSJIFEN, jifen);
}

void SessionInfo::setSearchByIDLimit(bool b)
{
	setBool(SIT_SEARCHLIMIT, b);
}

void SessionInfo::setMemberListLimit(bool b)
{
	setBool(SIT_VMLISTLIMIT,b);
}


void SessionInfo::setJoinJifenLimit(int n)
{
	setInt(SIT_VJOINJIFEN, n);
}

void SessionInfo::setChannelDisable(bool b)
{
	setBool(SIT_DISABLE, b);
}

void SessionInfo::setAutoApply(int n)
{
	setInt(SIT_APPLYAUTO, n);
}

void SessionInfo::setChangeQuality(bool b)
{
	//setBool(SIT_CHANGEQUALITY, b);
}

void SessionInfo::setOrder(const xstring& order)
{
	setString(SIT_ORDER, order);
}

void  SessionInfo::setHall(int hall)
{
	setInt(SIT_HALL, hall);
}

void  SessionInfo::setReception(int reception)
{
	setInt(SIT_RECEPTION, reception);
}

void SessionInfo::setLables(const xstring& lables)
{
	setString(SIT_LABLES, lables);
}


void SessionInfo::setSlogan(const xstring& slogan)
{
	setString(SIT_SLOGAN, slogan);
}

void SessionInfo::setFlashUrl(const xstring& url)
{
	setString(SIT_FLASHURL, url);
}

void SessionInfo::setAccessProperty(CAccessProperty ap)
{
	switch(ap) {
	case e_public:				//公開群
		{
			setInt(SIT_ISPUB, e_public);
			setBool(SIT_VMLISTLIMIT,true);
			setBool(SIT_ISLIMIT, false);
			setBool(SIT_SEARCHLIMIT, false);
			break;
		}
	case e_protected:			//半公開群
		{
			setInt(SIT_ISPUB, e_protected);
			setBool(SIT_ISLIMIT, true);
			setBool(SIT_SEARCHLIMIT, false);
			setBool(SIT_VMLISTLIMIT,true);
			break;
		}
	case e_private:				//私有群
		{
			setInt(SIT_ISPUB, e_default);
			setBool(SIT_ISLIMIT, false);
			setBool(SIT_VMLISTLIMIT,true);
			setBool(SIT_SEARCHLIMIT, true);
			break;
		}
	default:
		{
			setInt(SIT_ISPUB, e_public);
			setBool(SIT_VMLISTLIMIT,true);
			setBool(SIT_ISLIMIT, false);
			setBool(SIT_SEARCHLIMIT, false);
			break;
		}
	}
}

void SessionInfo::setIsSess(bool b)
{
	isSess = b;
}

bool SessionInfo::getIsSess() const 
{
	return isSess;
}


void SessionInfo::batchUnbanID(const std::vector<UID_T> &uids) {
	XCast<PBatchUpdateBanID> bu;
	bu.uids = uids;
	bu.isBan = false;
	
	pSession->SendNprotoRequest(bu);
}

void SessionInfo::batchUnbanIP(const std::vector<uint32_t> &ips) {
	XCast<PBatchUpdateBanIp> bu;
	bu.ips = ips;
	bu.isBan = false;
	pSession->SendNprotoRequest(bu);
}


void SessionInfo::setNavigation(const xstring& na) 					//導航
{
	if (na.size() > 1000) {
		assert(0);
		return;
	}
	setString(SIT_NAVIGATION, na);
}

void SessionInfo::setShowFlower(bool b)
{
	setBool(SIT_SHOWFLOWER, !b);
	setBool(SIT_NAVIGATION, !b);
}

void SessionInfo::setOpenPlatformInfo(xproto::OpenPlatformInfo info) { 
		OpenPlatformInfoManager::Instance()->SetSessionOpenPlatformInfo(
		getSid(),
		info.platform_id,
		app2net(info.platform_user_id),
		app2net(info.platform_user_name),
		app2net(info.pf_token),
		app2net(info.pf_email),
		app2net(info.pf_url));
}

void SessionInfo::removeOpenPlatformInfo(xproto::OpenPlatformInfo info) {
		OpenPlatformInfoManager::Instance()->RemoveSessionOpenPlatformInfo(
		getSid(),
		info.platform_id);
}

bool SessionInfo::GetExtend() {
	return get_extend_;
}

void SessionInfo::SetExtend(bool b) {
	get_extend_ = b;
}