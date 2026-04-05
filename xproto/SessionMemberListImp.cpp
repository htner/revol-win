#include "sessionmemberlistimp.h"
#include <iterator>
#include "helper/utf8to.h"
#include "protocol/psmemberlist.h"

#include "protocol/pkg_tag.h"
#include "SessionImp.h"
#include "MicrophoneListImp.h"
#include "common/core/form.h"
#include "time.h"

#include "protocol/psession.h"

using namespace protocol::session;
using namespace protocol;
using namespace core;
using namespace xproto;
#define EFFECT_TIME_TICK 30
#define EFFECT_TIME (EFFECT_TIME_TICK * 1000)

#define MEDAL_VIP_FIRST	(100000 - 1)
#define MEDAL_VIP_1			(MEDAL_VIP_FIRST+vip_1)
#define MEDAL_VIP_2			(MEDAL_VIP_FIRST+vip_2)
#define MEDAL_VIP_3			(MEDAL_VIP_FIRST+vip_3)
#define MEDAL_VIP_4			(MEDAL_VIP_FIRST+vip_4)
#define MEDAL_VIP_5			(MEDAL_VIP_FIRST+vip_5)


BEGIN_REQUEST_MAP_EXT(SessionMemberListImp)
	REQUEST_HANDLER(XCast<PPushSessData>, onMemberChanges)
	REQUEST_HANDLER(XCast<PUpdateClientInfos>, onUpdateClientInfos)
	REQUEST_HANDLER(XCast<POnSubChannelRemove>, onChannelRemove)
	REQUEST_HANDLER(XCast<POnSubChannelAdd>, onChannelAdd)
	REQUEST_HANDLER(XCast<PEnableAllText>, onEnableAllText)
	REQUEST_HANDLER(XCast<PEnableAllVoice>,onEnableAllVoice)
	REQUEST_HANDLER(XCast<POnTuoren>, onTuoren)
	REQUEST_HANDLER(XCast<PUpdateMemberCard>, onUpdateMemberCard)  // add ivan 2011.7.8
	//REQUEST_HANDLER(XCast<PChangeFolder>, onChangeFolder)  // add ivan 2011.7.8
	REQUEST_HANDLER(XCast<PSyncEffects>, onSyncEffect)
	REQUEST_HANDLER(XCast<PAddEffects>, onAddEffect)
	REQUEST_HANDLER(XCast<PRemoveEffect>, onRemoveEffect)
	REQUEST_HANDLER(XCast<PGetUserSessionCreditRes>, OnGetUserSessionCreditRes)

//	REQUEST_HANDLER(XCast<PGetSessionMemberCreditRes>, OnGetSessionMemberCreditRes)

	//REQUEST_HANDLER(XCast<PSessionChange>, onMemberChanges)
	//REQUEST_HANDLER(XCast<POnSubChannelRemove>, onChannelRemove)
	//REQUEST_HANDLER(XCast<POnSubChannelAdd>, onChannelAdd)

	//REQUEST_HANDLER(XCast<PSeqOnSubChannelAdd>, OnSeqSubChannelAdd)
	//REQUEST_HANDLER(XCast<PSeqPushSessData>, OnSeqMemberChanges)
END_REQUEST_MAP()


SessionMemberListImp::SessionMemberListImp(){
	bSelfAdd = false;
	bGetMember = false;
	bSyncMemberTime = 10;
	selfMuted = false;
	__effectTimer.init(this, &SessionMemberListImp::onEffectTimeout);
	__effectTimer.start(EFFECT_TIME);;
	medal_compare_ = NULL;
	uinfo_timer_.init(this, &SessionMemberListImp::OnGetUserInfo);
	uinfo_timer_.start(1000);
	member_uinfo_timer_.init(this,  &SessionMemberListImp::OnGetMemberUserInfo);
	last_session_jifen_ = 0;
}

SessionMemberListImp::~SessionMemberListImp(void)
{
}



//Inherit from IListBase<ITreeNode>
bool SessionMemberListImp::isFolder(const ITreeNode &hid) const
{
	return channels.find(hid.value) != channels.end();
}

bool SessionMemberListImp::isExisit(const ITreeNode& hid, const ITreeNode& pid) const
{
	if (hid.type == ITreeNode::E_USER) {
		return people.find(hid.value) != people.end();
	} else if (hid.type == ITreeNode::E_CHANNEL) {
		channels.find(hid.value) != channels.end();
	}
	return false;
}


void SessionMemberListImp::getParent(const ITreeNode &hid, std::vector<ITreeNode >& parents) const
{
	ITreeNode parent;
	if (hid.type == ITreeNode::E_USER) {
		people_doc_t::const_iterator it = people.find(hid.value);
		if(it != people.end()){
			parent.type = ITreeNode::E_CHANNEL;
			parent.value = it->second.pid;
			parents.push_back(parent);
			return ;
		}
	} else if (hid.type == ITreeNode::E_CHANNEL) {
		channal_doc_t::const_iterator it2 = channels.find(hid.value);
		if(it2 != channels.end() && it2->second.getPid() != SID_NULL){
			parent.type = ITreeNode::E_CHANNEL;
			parent.value = it2->second.getPid();
			parents.push_back(parent);
		}
	}
}

LPCWSTR SessionMemberListImp::getNick(const ITreeNode &hid) const
{

	if (hid.type == ITreeNode::E_USER) {
		people_doc_t::const_iterator it = people.find(hid.value);
		if(it != people.end()){
			return it->second.nick.c_str();
		}

		// do ont use this for getNick
		member_doc_t::const_iterator it3 = members.find(hid.value);
		if(it3 != members.end()){
			return it3->second.nick.c_str();
		}

		it = get_people_from_servers_.find(hid.value);
		if(it != get_people_from_servers_.end()){
			return it->second.nick.c_str();
		}

	} else if (hid.type == ITreeNode::E_CHANNEL) {
		channal_doc_t::const_iterator it2 = channels.find(hid.value);

		if(it2 != channels.end()){
			return it2->second.getNick();
		}
	}
	//assert(0); //fix you send user-id as ITreeNode
	return _T("");
}

// 獲取當前的短號
ITreeNode SessionMemberListImp::getAliaseId(const ITreeNode &hid) const
{
	ITreeNode parent;
	parent.type = ITreeNode::E_CHANNEL;
	if(hid.value == pSession->GetSessionId()){
		parent.value = pSession->GetAliasId();
	}else{
		parent.value = hid.value;
	}
	return parent;
}

//
IRoomBase::PlainTree_t SessionMemberListImp::getTreeData() const
{
	ITreeNode node;
	ITreeNode father;
	IRoomBase::PlainTree_t ret;

	node.type = ITreeNode::E_USER;
	father.type = ITreeNode::E_CHANNEL;
	for(people_doc_t::const_iterator it = people.begin(); it != people.end(); ++it){
			node.value = it->first;
			father.value = it->second.pid;
			ret.push_back(std::make_pair(node, father));
	}

	father.type = ITreeNode::E_CHANNEL;
	node.type = ITreeNode::E_CHANNEL;
	for(channal_doc_t::const_iterator it = channels.begin(); it != channels.end(); ++it){
			node.value = it->first;
			father.value = it->second.getPid();
			ret.push_back(std::make_pair(node, father));
	}
	return ret;
}
			
void SessionMemberListImp::fillTreeData(IListFiller* pFill,int flag) const
{
	ITreeNode node;
	ITreeNode father;
	if( flag & ISessionMemberList::DT_CHANNEL )
	{
		node.type = ITreeNode::E_CHANNEL;
		father.type = ITreeNode::E_CHANNEL;
		for(channal_doc_t::const_iterator it = channels.begin(); it != channels.end(); ++it){
			node.value = it->first;
			father.value = it->second.getPid();
			pFill->fillNode(node, father);
		}
	}

	if( flag & ISessionMemberList::DT_USER )
	{
		node.type = ITreeNode::E_USER;
		father.type = ITreeNode::E_CHANNEL;
		for(people_doc_t::const_iterator it = people.begin(); it != people.end(); ++it){
			node.value = it->first;
			father.value = it->second.pid;
			pFill->fillNode(node, father);
		}
	}
}

void SessionMemberListImp::onclose(int err, nproto::connection* c)
{
}

void SessionMemberListImp::oncreate(nproto::connection* c)
{
}

void SessionMemberListImp::leaveGuild()
{
}

bool SessionMemberListImp::isLimit()
{
	return pSession->IsLimit();
}

//channel Interface
uint32_t SessionMemberListImp::getOrder(const SID_T &subId) const
{
	ChannelOrlerMap::const_iterator it2 = channelOrders.find(subId);
	if(it2 != channelOrders.end()){
		return it2->second;
	}
	return subId;
}

void SessionMemberListImp::enableAllText(const SID_T &channelId, bool bEnable)
{
	XCast<PEnableAllText> et;
	et.enable = bEnable;
	et.sid = channelId;
	pSession->SendNprotoRequest(et);
}


bool SessionMemberListImp::hasPasswd(const SID_T &subId) const
{
	if (subId == pSession->getSid()) {
		return false;
	}

	channal_doc_t::const_iterator it2 = channels.find(subId);
	if(it2 != channels.end()){
		return it2->second.hasPasswd();
	}
	return false;
}

uinfo::SEX SessionMemberListImp::getSex(const UID_T& uid) const 
{
	people_doc_t::const_iterator it = people.find(uid);
	if(it != people.end()){
		return it->second.sex;
	}

	member_doc_t::const_iterator it2 = members.find(uid);
	if(it2 != members.end()){
		return it2->second.gender;
	}

	return uinfo::female;
}

//user Interface
bool SessionMemberListImp::isSameParent(const UID_T &u1,const UID_T &u2) const
{
	people_doc_t::const_iterator it1 = people.find(u1);
	people_doc_t::const_iterator it2 = people.find(u2);
	if (it1 != people.end() && it2 !=people.end() )
	{
		return it1->second.pid == it2->second.pid;
	}
	return false;
}



void SessionMemberListImp::changeFolder(const SID_T &from, const SID_T &to, const std::string &passwd)
{
		pSession->ResetVoiceManager();
		pSession->UpdateLastChannelPasswd(passwd);
		if(pSession->getCur() != to){
				XCast<PChangeFolder> cf;
				cf.from = from;
				cf.to = to;
				cf.passwd = passwd;
				pSession->SendNprotoRequest(cf);

				pSession->GetChannelExtendInfo(to);

		}
}

void SessionMemberListImp::changeTo(const SID_T &to, const std::string &passwd)
{
	changeFolder(pSession->getCur(), to, passwd);
}

ChannelRoler SessionMemberListImp::getRoler(const UID_T &hid, const SID_T &channelId) const
{
		return pSession->GetRoler(hid, channelId);
}

void SessionMemberListImp::changeMemberRight(const UID_T  &uid, ChannelRoler role, const SID_T &channelId)
{
		XCast<PUpdateChannelMember> rolerEdit;
		rolerEdit.channelId = channelId;
		rolerEdit.uid = uid;
		rolerEdit.roler = role;

		ChannelRoler er = getRoler(uid, channelId);

		if(role == NORMAL){
			rolerEdit.op = PUpdateChannelMember::REMOVE;
		}else if(er != NORMAL){
			rolerEdit.op = PUpdateChannelMember::CHANGE;
		}else if(er == NORMAL){
			rolerEdit.op = PUpdateChannelMember::ADD;
		}
		pSession->SendNprotoRequest(rolerEdit);
}

void SessionMemberListImp::changeMemberRight(const ACCOUNT_T &account, ChannelRoler role, const SID_T &channelId)
{
		XCast<PUpdateChannelMemberByAccount> rolerEdit;
		rolerEdit.channelId = channelId;
		rolerEdit.account = app2net(account);
		rolerEdit.roler = role;
		rolerEdit.op = PUpdateChannelMember::NULOP;
		pSession->SendNprotoRequest(rolerEdit);
}

void SessionMemberListImp::kickOff(const SID_T & sid, const UID_T &uid, const xstring& reason, uint32_t minutes)
{
		XCast<PSessKickOff> ko;
		ko.sid = sid;
		ko.reason = app2net(reason);
		if (ko.reason.size() > MAX_REASON_SIZE)
			ko.reason = ko.reason.substr(0, MAX_REASON_SIZE);
		ko.secs = minutes * 60;
		ko.uid = uid;

		pSession->SendNprotoRequest(ko);
}

void SessionMemberListImp::tuoren(const UID_T &who, const SID_T &from, const SID_T &to)
{
		XCast<PTuoRen> tr;
		tr.uid = who;
		tr.from = from;
		tr.to = to;

		pSession->SendNprotoRequest(tr);
}

void SessionMemberListImp::enableText(const UID_T &uid, bool bEnable) 
{
		XCast<PDisableText> pd;
		pd.uid= uid;
		pd.disable=!bEnable;
		pd.sid = innerGetUserPid(uid);

		pSession->SendNprotoRequest(pd);
}

bool SessionMemberListImp::isUserTextable(const UID_T &uid) const
{
		people_doc_t::const_iterator it=people.find(uid);
		if(it==people.end()) return false;


		SID_T subid = it->second.pid;
		is_map_t::const_iterator it2 = disabletext.find(it->second.pid);
		if(it2==disabletext.end()) return true;

		std::set<UID_T>::const_iterator it3 = it2->second.find(uid);
		return (!(it3!=it2->second.end()));
}


void SessionMemberListImp::enableVoice(const UID_T &uid, bool bEnable)
{
		XCast<PDisableVoice> pd;
		pd.uid=uid;
		pd.disable=!bEnable;
		pd.sid = innerGetUserPid(uid);

		pSession->SendNprotoRequest(pd);
}

bool SessionMemberListImp::isUserVoiceable(const UID_T &uid) const
{
		people_doc_t::const_iterator it=people.find(uid);
		if(it==people.end()) return false;


		SID_T subid = it->second.pid;
		is_map_t::const_iterator it2 = disablevoice.find(it->second.pid);
		if(it2==disablevoice.end()) return true;

		std::set<UID_T>::const_iterator it3 = it2->second.find(uid);
		return (!(it3!=it2->second.end())) && isChVoiceable(pSession->getCur());
}

bool SessionMemberListImp::canVoiceOpt(const UID_T &who,const UID_T &target) const
{
		people_doc_t::const_iterator it1, it2;
	
		it1 = people.find(who);
		if(it1 == people.end()) return false;

		it1 = people.find(target);
		if(it1 == people.end()) return false;

		return it1->second.pid == it2->second.pid;
}

void SessionMemberListImp::pushToMicrophoneList(const UID_T& uid)
{
		pSession->getMicrophoneList()->tuorenQueue(uid);
}

void SessionMemberListImp::directKickoff(const HULI & sid, const HULI &uid, const xstring& reason, uint32_t minutes)
{
		XCast<PDirectKickOff> ko;
		ko.sid = pSession->GetSessionId();
		ko.reason = app2net(reason);
		ko.secs = minutes * 60;
		ko.uid = uid;


		if (ko.reason.size() > MAX_REASON_SIZE)
			ko.reason = ko.reason.substr(0, MAX_REASON_SIZE);

		pSession->SendNprotoRequest(ko);
}

IUserGuild* SessionMemberListImp::getUserGuilds(const UID_T &uid)
{
		assert(false);
		return NULL;
}

LPCWSTR SessionMemberListImp::getSign(const UID_T &uid) const
{
		people_doc_t::const_iterator it = people.find(uid);
		if(it != people.end()){
			return it->second.sign.c_str();
		}

		member_doc_t::const_iterator it2 = members.find(uid);
		if(it2 != members.end()){
			return it2->second.nick.c_str();
		}

		return _T("");
}

LPCWSTR SessionMemberListImp::getStatus(const UID_T &uid) const
{
		people_doc_t::const_iterator it = people.find(uid);
		if(it != people.end()){
			return it->second.status.c_str();
		}else{
			return _T("");
		}
}

bool SessionMemberListImp::isUserTextableInCh(const UID_T &uid) const
{
		people_doc_t::const_iterator it1 = people.find(uid);
		if(it1 == people.end()) return false;

		return isChTextable(it1->second.pid) || pSession->GetRoler(uid, pSession->GetSessionId()) >= CMANAGER;	
}

uint32_t SessionMemberListImp::getUserJF(const UID_T &uid) const
{
		people_doc_t::const_iterator it1 = people.find(uid);
		if(it1 != people.end()) {
			return it1->second.user_jifen;
		}

		member_doc_t::const_iterator it2 = members.find(uid);
		if(it2 != members.end()){
			return it2->second.user_jifen;
		}
		return 0;
}

int	SessionMemberListImp::getUserSessContribute(const UID_T &uid) {
	std::map<uint32_t, uint32_t>::const_iterator it_jifen = user_session_jifen_.find(uid); 		
	if (it_jifen != user_session_jifen_.end()) {
		return it_jifen->second;
	}
	getUserSessJF(uid);
	return -1;
}

uint32_t SessionMemberListImp::getUserSessJF(const UID_T &uid)
{

	std::map<uint32_t, uint32_t>::const_iterator it_jifen = user_session_jifen_.find(uid); 		
	if (it_jifen != user_session_jifen_.end()) {
		return it_jifen->second;
	}

	if (bSyncMemberTime == 0) {
		member_doc_t::const_iterator it2 = members.find(uid);
		if(it2 != members.end()){
			return it2->second.usess_jifen;
		}
	}

	if (usjifen_requested_.find(uid) == usjifen_requested_.end()) {
		int max = 50;
		if (pSession->GetRoler(pSession->GetUid(), 0) >= protocol::session::MEMBER) {
			max = 100;
		}
		if (usjifen_requests_.size() < max || uid == pSession->GetUid()) {
			usjifen_requests_.insert(uid);
			usjifen_requested_.insert(uid);
			if (!uinfo_timer_.hasStart()) {
				uinfo_timer_.start(10);
			}
		}
	}

	people_doc_t::const_iterator it1 = people.find(uid);
	if(it1 != people.end()) {
		return it1->second.usess_jifen;
	}		

	member_doc_t::const_iterator it2 = members.find(uid);
	if(it2 != members.end()){
		return it2->second.usess_jifen;
	}
	return 0;
}


void SessionMemberListImp::banID(const UID_T &uid) 
{
		XCast<PUpdateBanID> bu;
		bu.isBan = true;
		bu.uid = uid;
		pSession->SendNprotoRequest(bu);
}

void SessionMemberListImp::banIP(const UID_T &uid)
{
		XCast<PUpdateBanIp> bu;
		bu.isBan = true;
		bu.uid =  uid;
		pSession->SendNprotoRequest(bu);
}

void SessionMemberListImp::banAccount(const ACCOUNT_T &account)
{
		XCast<PUpdateBanAccount> bu;
		bu.isBan = true;
		bu.account =  app2net(account);
		pSession->SendNprotoRequest(bu);
}

void SessionMemberListImp::setAllKeyActive(const SID_T &channelId)
{
		XCast<PSetRoomKeyActive> sk;
		sk.sid = channelId ;
		
		pSession->SendNprotoRequest(sk);
}

void SessionMemberListImp::ignoreText(const UID_T &uid, bool ignore)
{
		pSession->IgnoreText(uid, ignore);
		notifyUserChange(uid);
}

void SessionMemberListImp::ignoreVoice(const UID_T &uid, bool ignore)
{
		pSession->IgnoreVoice(uid, ignore);
		notifyUserChange(uid);
}

bool SessionMemberListImp::isIgnoreText(const UID_T &uid) const
{
		return pSession->IsIgnoreText(uid);
}

bool SessionMemberListImp::isIgnoreVoice(const UID_T &uid) const
{
		return pSession->IsIgnoreVoice(uid);
}

//member Interface
void SessionMemberListImp::getMember(std::vector<UID_T>& ret) const 
{
		if (ret.capacity()  < members.size()) {
			ret.reserve(members.size());
		}
		for(member_doc_t::const_iterator it = members.begin(); it != members.end(); ++it){
				ret.push_back(it->first);
		}
}

ChannelRoler SessionMemberListImp::getMemberRoler(const UID_T &uid) const
{
	return pSession->GetMaxRoler(uid);

}

MemberStatus SessionMemberListImp::getMemberStatus(const UID_T &uid) const 
{
		MemberStatus mStatus = ms_offline;
		if (isOnline(uid) == true) {
			mStatus = ms_InSession;
		}
		return mStatus;
}

int SessionMemberListImp::getMemberCount() const
{
	return (members.size() == 0) ?  memberCount : members.size();
}


///////////////////////
//
//
//
//////////////////////

void	SessionMemberListImp::setSession(SessionImp* s)
{
		pSession = s;
}

void	SessionMemberListImp::onSync(std::vector<Partner> &partners, 
			std::vector<SubChannel> &subs, 
			const protocol::session::is_map_t &refbanvoice,
			const protocol::session::is_map_t &refbantext, 
			const std::vector<SID_T> &txtDisabledCh,
			bool bReConnect)
{
		bSelfAdd = !bReConnect;
		people.clear();
		channels.clear();
		disableTextChs.clear();

		for(std::vector<Partner>::iterator it = partners.begin(); it != partners.end(); ++it){
			//uinfo_requests_map_[it->uid] = 0;
			PeopleItem &s = people[it->uid];
			s.nick = filterDisplayString(net2app(it->nick).c_str());
			s.sex = it->gender;
			s.pid = it->pid;
			s.user_jifen = it->user_jifen;
			s.usess_jifen = it->usess_jifen;
			s.sign = net2app(it->sign).c_str();
			s.setClientInfo(net2app(it->client_info).c_str());
		}

		

		for(std::vector<uint32_t>::const_iterator it = txtDisabledCh.begin(); it != txtDisabledCh.end(); ++it){
			disableTextChs.insert(*it);
		}

		for(std::vector<SubChannel>::const_iterator it = subs.begin(); it != subs.end(); ++it) {
			ChannelItem &item = channels[it->sid];
			item.setPid(it->pid);
			item.setNick(net2app(it->nick).c_str());
			item.setPasswd(it->hasPasswd);
		}

		disablevoice=refbanvoice;
		disabletext=refbantext;

		// XConnPoint<IRoomEvent>::forEachWatcher0(&IRoomEvent::onRefresh);

		if(people.find(pSession->GetUid()) != people.end() && bSelfAdd){
			pSession->CurrectChannelChanged(0, people[pSession->GetUid()].pid);
		}
		if (isSelfMuted() == true) {
			this->mutedSelf(selfMuted);
			selfMuted = false;
		}
		//uinfo_timer_.start(100);
}

void SessionMemberListImp::OnInitUserChannel(const std::vector<std::pair<uint32_t, uint32_t> > &sub_channel_infos) {
	for(std::vector<std::pair<uint32_t, uint32_t> >::const_iterator it = sub_channel_infos.begin(); it != sub_channel_infos.end(); ++it){
		PeopleItem &s = people[it->first];
		s.pid = it->second;
		s.sex = protocol::uinfo::female;
		s.user_jifen = 0;
		s.usess_jifen = 0;
		uinfo_requests_map_[it->first] = 0;
	}
	uinfo_timer_.start(10);

	if(people.find(pSession->GetUid()) != people.end()){
		pSession->CurrectChannelChanged(0, people[pSession->GetUid()].pid);
	}
}

void SessionMemberListImp::UpdateDisableVoice(const protocol::session::is_map_t &refbanvoice) {
	disablevoice=refbanvoice;
}

void SessionMemberListImp::UpdateDisableText(const protocol::session::is_map_t &refbantext) {
	disabletext=refbantext;
}

void SessionMemberListImp::UpdateChannelTreeInfo(const std::vector<SubChannel>& sub_channel_infos) {
	for(std::vector<SubChannel>::const_iterator it = sub_channel_infos.begin(); it != sub_channel_infos.end(); ++it){
			ChannelItem &item = channels[it->sid];
			item.setPid(it->pid);
			item.setNick(net2app(it->nick).c_str());
			item.setPasswd(it->hasPasswd);
	}
}

void SessionMemberListImp::UpdateOrder(const std::vector<uint32_t>& orders)
{
		channelOrders.clear();
		maxOrder = orders.size();
		for(int i = 0; i < maxOrder; ++i){
			channelOrders[orders[i]] = i + 100;
		}
		maxOrder += 100;
		channelOrders[pSession->getHall()] = 1;
		XConnPoint<IRoomEvent>::forEachWatcher0(&IRoomEvent::onChangeOrder);
}

//channel
void	SessionMemberListImp::setTextAble(const UID_T &uid,bool bAble)
{
	people_doc_t::const_iterator it = people.find(uid);		
	if (it != people.end()) {
		if(bAble) {
			disabletext[it->second.pid].erase(uid);
		}  else {
			disabletext[it->second.pid].insert(uid);
		}
		notifyUserChange(uid);
	}
}

void	SessionMemberListImp::setVoiceAble(const UID_T &uid,bool bAble)
{
	people_doc_t::const_iterator it = people.find(uid);		
	if (it != people.end()) {
		if(bAble) {
			disablevoice[it->second.pid].erase(uid);
		} else {
			disablevoice[it->second.pid].insert(uid);
		}

		notifyUserChange(uid);
	}
}

bool	SessionMemberListImp::isChTextable(const SID_T &ch) const
{
		return disableTextChs.find(ch) == disableTextChs.end();
}

bool	SessionMemberListImp::isChVoiceable(const SID_T &ch) const
{
		return disableVoiceChs.find(ch) == disableVoiceChs.end();
}

SID_T	SessionMemberListImp::innerGetChannelPid(const SID_T &chid) const
{
		channal_doc_t::const_iterator it2 = channels.find(chid);

		if(it2 != channels.end()){
			return it2->second.getPid();
		}else{
			return 0;
		}
}

void	SessionMemberListImp::notifyChannelChange(const SID_T &chId)
{
		
		channal_doc_t::const_iterator it = channels.find(chId);
		if(it != channels.end()){
			ITreeNode node;
			node.type = ITreeNode::E_CHANNEL;
			node.value =  chId;

			ITreeNode p;
			p.type = ITreeNode::E_CHANNEL;
			p.value =  it->second.getPid();
			XConnPoint<IRoomEvent>::forEachWatcher2(&IRoomEvent::onChange, 
				ITreeNode(node), ITreeNode(p));
		}
}


//user
void	SessionMemberListImp::infoAffectList(int id, const sox::Properties &prop)
{
		if(prop.props.find(SIT_NAME) != prop.props.end()){
			localStringW newNick = net2app(sox::properties::get(prop, SIT_NAME)).c_str();
			channal_doc_t::iterator it = channels.find(id);
			if(it != channels.end()){
				if(it->second.getNick() != newNick){
					it->second.setNick(newNick.c_str());

					XConnPoint<IRoomEvent>::forEachWatcher2(&IRoomEvent::onChange, 
						ITreeNode(id), ITreeNode(it->second.getPid()));
				}
			}
		}

		if(prop.props.find(SIT_BHASPASSWD) != prop.props.end()){
			bool nb = sox::properties::getint(prop,  SIT_BHASPASSWD) != 0;
			channal_doc_t::iterator it = channels.find(id);
			if(it != channels.end()){
				if(it->second.hasPasswd() != nb){
					it->second.setPasswd(nb);


					XConnPoint<IRoomEvent>::forEachWatcher2(&IRoomEvent::onChange, 
						ITreeNode(id), ITreeNode(it->second.getPid()));
				}
			}
		}

		if(id == pSession->GetSessionId()){
			XConnPoint<IRoomEvent>::forEachWatcher2(&IRoomEvent::onChange, 
				ITreeNode(id), ITreeNode(SID_NULL));
		}
}

bool	SessionMemberListImp::isOnline(const UID_T &uid) const
{
		return people.find(uid) != people.end();
}


void	SessionMemberListImp::setKeyActive(const UID_T &uid)
{
		XCast<PSetKeyActive> ps;
		ps.target = uid;
		ps.sid = innerGetUserPid(uid);
		pSession->SendNprotoRequest(ps);
}

void	SessionMemberListImp::onPInfoChange(PPInfoChanged *pp, uint32_t resCode)
{
		if(Request::ifSuccess(resCode)){
			if(pp){
				people_doc_t::iterator it = people.find(pp->uid);
				if (it != people.end()) {
					PeopleItem &item = it->second;
					item.nick = filterDisplayString(net2app(pp->nick).c_str());
					item.sex = pp->gender;
					item.sign = net2app(pp->sign).c_str();

					syncMemberInfo(pp->uid,  NUL_ROLE);

					ITreeNode node, p;
					node.type = node.E_CHANNEL;
					node.value = pp->uid;
					p.type = node.E_CHANNEL;
					p.value = item.pid;
					XConnPoint<IRoomEvent>::forEachWatcher2(&IRoomEvent::onChange, ITreeNode(pp->uid, ITreeNode::E_USER), ITreeNode(item.pid));
					pSession->getMicrophoneList()->onChange(pp->uid, item.pid);
				}
			}
		}
}

void	SessionMemberListImp::onStatusChange(const UID_T& uid,const xstring& status, uint32_t resCode)
{
		if(Request::ifSuccess(resCode)){
			people_doc_t::iterator it = people.find(uid);
			if(it != people.end()){
				PeopleItem &item = it->second;
				if( item.status !=status.c_str() )
				{
					item.status = status.c_str();
					//XConnPoint<IMemberListEvent<> >::forEachWatcher1(&IMemberListEvent<>::onMemberChange, uid);
					XConnPoint<IRoomEvent>::forEachWatcher2(&IRoomEvent::onChange, 
						ITreeNode(uid, ITreeNode::E_USER), ITreeNode(item.pid));
					pSession->getMicrophoneList()->onChange(uid, item.pid);
				}
			}
	    }
}

SID_T	SessionMemberListImp::innerGetUserPid(const UID_T &uid) const
{
		people_doc_t::const_iterator it = people.find(uid);
		if(it != people.end()){
			return it->second.pid;
		}
		return 0;
		
}
 
void	SessionMemberListImp::notifyUserChange(const UID_T &uid)
{
		people_doc_t::iterator it = people.find(uid);
		if(it != people.end()){
			XConnPoint<IRoomEvent>::forEachWatcher2(&IRoomEvent::onChange, 
				ITreeNode(uid, ITreeNode::E_USER), ITreeNode(it->second.pid));
		}	
}


// member
void SessionMemberListImp::onSyncMembers(const std::vector<PMemberInfo>& memberlist)
{
		bGetMember = true;
		members.clear();
		nsox::xlog(NsoxDebug, "synx members , size:%d", memberlist.size());
		for(int count = 0; count < memberlist.size(); ++count)
		{
			if (memberlist[count].uid) {
				MemberItem item;
				item.nick = filterDisplayString(net2app(memberlist[count].nick).c_str());
				item.user_jifen = memberlist[count].user_jifen ;
				item.usess_jifen =  memberlist[count].usess_jifen;
				item.gender = (protocol::uinfo::SEX)memberlist[count].gender;
				item.sign = net2app(memberlist[count].sign).c_str();
				item.roler = (ChannelRoler)memberlist[count].roler;
				item.addtime = memberlist[count].addtime;
				members[memberlist[count].uid] = item;

			}
		}
		XConnPoint<IMemberListEvent >::forEachWatcher0(&IMemberListEvent::onMemberRefresh);
}

void SessionMemberListImp::addMember(const UID_T &uid, const xstring &nick, const xstring &sign,
					protocol::uinfo::SEX gender, uint32_t user_jifen, uint32_t usjifen, ChannelRoler roler)
{
			if (roler == KEFU || roler == SA) {
					return;
			}

			if (bGetMember == false) {
				memberCount++;
				return;
			}
			member_doc_t::const_iterator it = members.find(uid);
			if ( it == members.end())
			{
					members[uid];  // create member for sync
					members[uid].user_jifen = 0;
					members[uid].usess_jifen = 0;

					//usjifen_requests_.insert(uid);
					people_doc_t::const_iterator it1 = people.find(uid);
					if (it1 != people.end())
					{
						//if (pSession->GetServerVserion() != 200) {
							syncMemberInfo(uid, roler, false);
						//}
					} else {
							members[uid].nick = filterDisplayString(nick.c_str());
							members[uid].user_jifen = user_jifen ;
							members[uid].usess_jifen= usjifen;
							members[uid].gender = gender;
							members[uid].sign =sign.c_str();
							members[uid].roler = roler;
							//members[uid].addtime = pSession->getTimeStamp();
							members[uid].addtime = time(NULL);
					}
					members[uid].addtime = time(NULL);
					
					XConnPoint<IMemberListEvent >::forEachWatcher1(&IMemberListEvent::onMemberAdd, uid);
			} else {
				if (pSession->GetServerVserion() != 200) {
					syncMemberInfo(uid , roler);
				}
			}
}


void SessionMemberListImp::removeMember(const UID_T &uid)
{
			if (bGetMember == false) {
				memberCount--;  
				return;
			}
			member_doc_t::iterator it = members.find(uid);
			if ( it != members.end())
			{
				members.erase(it);
				XConnPoint<IMemberListEvent >::forEachWatcher1(&IMemberListEvent::onMemberRemove, uid);
			}
}

void SessionMemberListImp::syncMemberInfo(const UID_T &uid, ChannelRoler roler, bool notice)
{
		member_doc_t::iterator it = members.find(uid);
		if ( it != members.end())
		{
			if (NUL_ROLE != roler ) {
					it->second.roler = roler;
			}

			people_doc_t::const_iterator it1 = people.find(uid);
			if (it1 != people.end( )) {
				it->second.nick = filterDisplayString(it1->second.nick.c_str());

				if (it1->second.user_jifen > it->second.user_jifen) {
					it->second.user_jifen = it1->second.user_jifen ;
				}
				
				if (it1->second.usess_jifen > it->second.usess_jifen) {
					it->second.usess_jifen = it1->second.usess_jifen;
				}

				it->second.gender = it1->second.sex;
				it->second.sign = it1->second.sign;
			}
			if (notice == true) {
				XConnPoint<IMemberListEvent >::forEachWatcher1(&IMemberListEvent::onMemberChange, uid);
			}
		} 
}

void SessionMemberListImp::removeAllMember()
{
		members.clear();
		XConnPoint<IMemberListEvent >::forEachWatcher0(&IMemberListEvent::onMemberRefresh);
}



//packets
void SessionMemberListImp::onMemberAdd(POnMemberAdd* cmd, nproto::connection* c)
{
		people_doc_t::iterator it = people.find(cmd->uid);
		if(it == people.end()){
			remvoeUserEffect(cmd->uid);
			fast_sync_uinfo_.insert(cmd->uid);
			uinfo_requests_map_[cmd->uid] = 0;
			//int rand_t = rand() % 150 + 10;
			uinfo_timer_.start(10);
			PeopleItem &item = people[cmd->uid];
			item.nick = filterDisplayString(net2app(cmd->nick).c_str());
			item.sex = cmd->sex;

			pSession->AddRoler(cmd->uid,cmd->rolers);

			item.pid = cmd->pid;
			assert(item.pid);
			item.user_jifen = cmd->user_jifen;
			item.usess_jifen = cmd->usess_jifen;
			item.sign = net2app(cmd->sign).c_str();

			
			if (pSession->GetServerVserion() != 200) {
				syncMemberInfo(cmd->uid,  NUL_ROLE);
			}

			XConnPoint<IRoomEvent>::forEachWatcher2(&IRoomEvent::onAdd, 
				 ITreeNode(cmd->uid, ITreeNode::E_USER), ITreeNode(item.pid));

			if (cmd->uid == pSession->GetUid() ) {
				pSession->ResetVoiceManager();
				if (bSelfAdd == true ) {
					pSession->CurrectChannelChanged(0, item.pid);
				} else {
					bSelfAdd = true;
				}
			}

		} else if (it->second.pid != cmd->pid) {
					uint32_t from = it->second.pid;
					it->second.pid = cmd->pid;
					
					XConnPoint<IRoomEvent>::forEachWatcher3(&IRoomEvent::onMoved, 
						ITreeNode (it->first, ITreeNode::E_USER), ITreeNode(from), ITreeNode(cmd->pid));

					if(it->first == pSession->GetUid()){
								pSession->CurrectChannelChanged(from, cmd->pid);
					}

		}
}

void SessionMemberListImp::onChannelAdd(XCast<POnSubChannelAdd>& cmd, nproto::connection* c)
{
		pSession->NotifyResult(CREATE_SUBCHANNEL,cmd.resCode);

		if(core::Request::ifSuccess(cmd.resCode)){	
			pSession->UpdateChannelInfo(cmd.sid, cmd.info);

			ChannelItem item;
			item.setNick(net2app(sox::properties::get(cmd.info, SIT_NAME)).c_str());
			item.setPid(sox::properties::getint(cmd.info, SIT_PID));
			item.setPasswd(sox::properties::getint(cmd.info, SIT_BHASPASSWD) != 0);

			HULI ss = cmd.sid;
			channels[ss] = item;
		
			XConnPoint<IRoomEvent>::forEachWatcher2(&IRoomEvent::onAdd, 
						ITreeNode(ss), ITreeNode(item.getPid()));

			updateChannelOrder(ss, cmd.preChannel);  //最後才update order,通知客戶端重新排序
		}
}

void SessionMemberListImp::onChannelRemove(XCast<POnSubChannelRemove>& cmd, nproto::connection* c)
{
		if(!core::Request::ifSuccess(cmd.resCode)){
			pSession->NotifyResult(DISSMISS_SUBCHANNEL,cmd.resCode);
		}else{
			//do nothing
			HULI ss = cmd.sid;
			channelOrders.erase(ss);
		}
}

void SessionMemberListImp::onTuoren(XCast<POnTuoren>& cmd, nproto::connection* c)
{
		if(core::Request::ifSuccess(cmd.resCode)){
			if(pSession->GetUid() == cmd.uid){
				pSession->NotifyTuoren(cmd.admin, cmd.from, cmd.to);
			}
		}else{
			pSession->NotifyResult(CHANGE_FOLDER,cmd.resCode);
		}
}

void SessionMemberListImp::onMemberChanges(XCast<PPushSessData>& cmd, nproto::connection* c)
{
	RealMemberChanges(&cmd, c);
}

void SessionMemberListImp::onEnableAllText(XCast<PEnableAllText>& cmd, nproto::connection* c)
{
		if(core::Request::ifSuccess(cmd.resCode))
		{
				if(cmd.enable){
					disableTextChs.erase(cmd.sid);
				}else{
					disableTextChs.insert(cmd.sid);
				}
				pSession->NotifyEnableAllText(&cmd);
				notifyChannelChange(cmd.sid);
		}
}

void SessionMemberListImp::onEnableAllVoice(XCast<PEnableAllVoice>& cmd, nproto::connection* c)
{
		if(!core::Request::ifSuccess(cmd.resCode)){
			if(cmd.enable){
				disableVoiceChs.erase(cmd.sid);
			}else{
				disableVoiceChs.insert(cmd.sid);
			}
		}
}


SID_T SessionMemberListImp::getUserPid()
{
		return 0;
}

void SessionMemberListImp::updateChannelOrder(uint32_t sid, uint32_t pre)
{
	if (pre == 0) {
			channelOrders[sid] = maxOrder++;
	} else {
		ChannelOrlerMap::const_iterator it = channelOrders.find(pre);
		if (it == channelOrders.end()){
				
		} else {
				uint32_t pre_order = it->second;
				ChannelOrlerMap::iterator it = channelOrders.begin();
				for(; it != channelOrders.end(); it++){
					if (it->second >  pre_order) {
						it->second += 1;
						if (it->second > maxOrder) {
							maxOrder = it->second;
						}
					}
				
				}
				channelOrders[sid] = pre_order + 1;
		}
		XConnPoint<IRoomEvent>::forEachWatcher0(&IRoomEvent::onChangeOrder);
	}

}

void SessionMemberListImp::updateMemberList() const
{
	/*if (__bGetMemberList == false && (__sessInfo->isMemberListLimit() == false || getRoler(__uid, 0) >= MEMBER)) {
		XCast<PGetMemberList> cmd;
		SendNprotoRequest(cmd);
	}
	*/
	if (bGetMember == false) {
		pSession->UpdateMemberList();
	} else {
		// XConnPoint<IMemberListEvent >::forEachWatcher0(&IMemberListEvent::onMemberRefresh);
	}
	//pSession->updateMemberList();
}

void SessionMemberListImp::RefreshSessionTree() {
	XConnPoint<IRoomEvent>::forEachWatcher0(&IRoomEvent::onRefresh);
}

void SessionMemberListImp::endSync(bool bReConnect)
{
	 people_doc_t::const_iterator it = people.find(pSession->GetUid());
	 if (it != people.end()) {
			if(!bReConnect){
				pSession->CurrectChannelChanged(0, it->second.pid);
			}
	 }
}

void SessionMemberListImp::updateMemberCount(uint32_t c) 
{
		memberCount = c;
}


void SessionMemberListImp::onMemberCards(const std::vector<MemberCard>& memberCards)
{
	this->memberCards.clear();
	for (int i = 0; i < memberCards.size(); ++i) {
		uint32_t uid = memberCards[i].uid;
		this->memberCards[uid].cname = filterDisplayString(net2app(memberCards[i].cname).c_str());
		this->memberCards[uid].cidentity = memberCards[i].cidentity;
		if (this->memberCards[uid].cname.empty() == false) {
			memberNames.insert(this->memberCards[uid].cname);
		}
	}
}

void SessionMemberListImp::onUpdateMemberCard(const UID_T& uid,  
					      const xstring& cname, uint32_t cidentity) 
{
	memberNames.erase(this->memberCards[uid].cname);
	memberCards[uid].cname = filterDisplayString(cname.c_str());
	if (cname.empty() == false) {
			memberNames.insert(this->memberCards[uid].cname);
	}

	memberCards[uid].cidentity = cidentity;
}

void SessionMemberListImp::onRemoveMember(const UID_T& uid)
{
	memberNames.erase(this->memberCards[uid].cname);
	memberCards.erase(uid);
}

LPCWSTR SessionMemberListImp::getCardNick(const HULI &hid) const
{
	if (pSession->GetRoler(hid, 0) == protocol::session::NORMAL) {
		return _T("");
	}
	card_doc_t::const_iterator it = memberCards.find(hid);
	if(it != memberCards.end()){
		return it->second.cname.c_str();
	}
	return _T("");
}


LPCWSTR SessionMemberListImp::getRealNick(const HULI &hid) const 
 {
	 people_doc_t::const_iterator it = people.find(hid);
	if(it != people.end()){
		return it->second.nick.c_str();
	}

	channal_doc_t::const_iterator it2 = channels.find(hid);

	if(it2 != channels.end()){
		return it2->second.getNick();
	}

	member_doc_t::const_iterator it3 = members.find(hid);

	if(it3 != members.end()){
		return it3->second.nick.c_str();
	}

	it = get_people_from_servers_.find(hid);
	if(it != get_people_from_servers_.end()){
		return it->second.nick.c_str();
	}

	return _T("");
 }


void SessionMemberListImp::onUpdateMemberCard(XCast<PUpdateMemberCard>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)) {
		this->onUpdateMemberCard(cmd.uid, net2app(cmd.cname), cmd. cidentity);
		XConnPoint<IMemberCardEvent>::forEachWatcher3(&IMemberCardEvent::onChange, cmd.uid, net2app(cmd.cname), cmd.modifyBy);
	}
}

void SessionMemberListImp::updateMemberCards(const UID_T& uid, const xstring &name, uint32_t  nLimit)
{
	IAuthChecker* checker = pSession->getAuthChecker();
	if (nLimit > mbcl_admin || isCardsNameExist(name)) {
		return;
	}
	if (checker && checker->canDo(&SessOperation(SessOperation::UPDATE_MEMBER_CARD,	 pSession->GetSessionId(), uid))) {
		XCast<PUpdateMemberCard> req;
		req.uid = uid;
		req.cname = app2net(name);
		req.cidentity = nLimit;
		req.temp = false;
		pSession->SendNprotoRequest(req);	
	}
}

bool SessionMemberListImp::isSelfUpdateMemberCards(const UID_T& uid)
{
	card_doc_t::const_iterator it = memberCards.find(uid);
	if(it != memberCards.end()){
		return it->second.cidentity != 1;
	}
	return true;
}


bool SessionMemberListImp::isAdminUpdateMemberCards(const UID_T& uid)
{
	card_doc_t::const_iterator it = memberCards.find(uid);
	if(it != memberCards.end()){
		return it->second.cidentity != 2;
	}
	return true;
}

bool SessionMemberListImp::isCardsNameExist(const xstring &name)
{
	return !name.empty() && memberNames.find(name.c_str()) != memberNames.end() ;
}

uint32_t SessionMemberListImp::getAddTime(const UID_T &uid) const 
{
	// do ont use this for getNick
	member_doc_t::const_iterator it = members.find(uid);
	if(it != members.end()){
		return it->second.addtime;
	}
	return 0;
}

uint32_t SessionMemberListImp::getOnlineCount() const 
{
	return people.size();
}

void SessionMemberListImp::onChangeFolder(XCast<PChangeFolder>& cmd, nproto::connection* c)
{
	people_doc_t::iterator pt = people.find(pSession->GetUid());
	if(pt != people.end()){
			SID_T t = cmd.to;
			SID_T f = pt->second.pid;
			pt->second.pid = t;
			XConnPoint<IRoomEvent>::forEachWatcher3(&IRoomEvent::onMoved, 
					ITreeNode (pSession->GetUid(), ITreeNode::E_USER), ITreeNode(f), ITreeNode(t));

			if(f != t){
					pSession->CurrectChannelChanged(f, t);
			}
	}
}

void SessionMemberListImp::onSyncEffect(XCast<PSyncEffects>& cmd, nproto::connection* c)
{
	userMedals.clear();
	for (int i = 0; i < cmd.effects.size(); ++i) {
		simeffect_t& ueff = cmd.effects[i].simeffects;
		UID_T uid = cmd.effects[i].uid;
		simeffect_t& uMedal = userMedals[uid];
		for( int j = 0; j < ueff.size(); ++j) {
			if (ueff[j].type >= MEDAL_VIP_1 && ueff[j].type <= MEDAL_VIP_5) {
				people_doc_t::iterator it = people.find(uid);
				if(it != people.end()){
					VipLevel vip = (VipLevel)(ueff[j].type - MEDAL_VIP_FIRST);
					if (vip > it->second.vip) {
						it->second.vip = vip;
					}
				}
			} else if (ueff[j].tick > cmd.run) {
				ueff[j].tick -= cmd.run;
				uMedal.push_back(ueff[j]);
			}
		}
	}

	if (medal_compare_) {
		simeffect_doc_t::iterator it = userMedals.begin();
		while(it != userMedals.end()) {
			if (it->second.size() >= 2) {
				sort(it->second.begin(), it->second.end(), medal_compare_);
			}
			it++;
		}
	}
	XConnPoint<IMedalEvent>::forEachWatcher0(&IMedalEvent::onMedalRefresh);
	XConnPoint<IVIPEvent>::forEachWatcher0(&IVIPEvent::onVIPRefresh);

}

void SessionMemberListImp::onAddEffect(XCast<PAddEffects>& cmd, nproto::connection* c)
{
	bool isChangeVip = false, isChangeMedal = false;
	for (int i = 0; i < cmd.effects.size(); ++i) {
		simeffect_t& ueff = cmd.effects[i].simeffects;
		uint32_t uid = cmd.effects[i].uid;
		isChangeVip = false;
		isChangeMedal = false;
		for( int j = 0; j < ueff.size(); ++j) {
			if (ueff[j].type >= MEDAL_VIP_1 && ueff[j].type <= MEDAL_VIP_5) {
				 isChangeVip = true;
				people_doc_t::iterator it = people.find(uid);
				if(it != people.end()){
					VipLevel vip = (VipLevel)(ueff[j].type - MEDAL_VIP_FIRST);
					if (vip > it->second.vip) {
						it->second.vip = vip;
					}
				}
			} else if (ueff[j].tick > cmd.run) {
				isChangeMedal = true;
				ueff[j].tick -= cmd.run;
				bool once = true;
				for (int k = 0; k < userMedals[uid].size(); ++k) {
					if (userMedals[uid][k].eid == ueff[j].eid) {
						once = false;
					}
				}
				if (once == true) {
					
					simeffect_doc_t::iterator it_user_effect = userMedals.find(uid);
					if (it_user_effect == userMedals.end()) {
						userMedals[uid].push_back(ueff[j]);
						it_user_effect = userMedals.find(uid);
					} else {
						it_user_effect->second.push_back(ueff[j]);
					}
					assert(it_user_effect != userMedals.end());
					if (medal_compare_ && it_user_effect->second.size() >= 2) {
						sort(it_user_effect->second.begin(), it_user_effect->second.end(), medal_compare_);
					}
				}
			}
		}
		if (isChangeMedal == true) {
			XConnPoint<IMedalEvent>::forEachWatcher1(&IMedalEvent::onMedalChange, uid);
		}
		if (isChangeVip == true) {
			XConnPoint<IVIPEvent>::forEachWatcher1(&IVIPEvent::onVIPChange, uid);
		}
	}
}

void SessionMemberListImp::onRemoveEffect(XCast<PRemoveEffect>& cmd, nproto::connection* c)
{
		uint32_t uid = cmd.uid;
		simeffect_doc_t::iterator it = userMedals.find(uid);
		if (it != userMedals.end()) {
			simeffect_t::iterator eit =  it->second.begin();
			while(eit != it->second.end()) {
				if (cmd.eid == eit->eid) {
					it->second.erase(eit);
					break;
				}
				eit++;
			}
			if (medal_compare_ && it->second.size() >= 2) {
						sort(it->second.begin(), it->second.end(), medal_compare_);
			}
		}
		XConnPoint<IMedalEvent>::forEachWatcher1(&IMedalEvent::onMedalChange, uid);
		//XConnPoint<IEffectEvent>::forEachWatcher1(&IEffectEvent::onEffectChange, uid);
}

void SessionMemberListImp::setMedalCompareFunction(MedalCompareFunction medal_compare) {
	medal_compare_ = medal_compare;
}
simeffect_const_t& SessionMemberListImp::getEffect(const UID_T &uid) const
{
	simeffect_doc_t::const_iterator it = userMedals.find(uid);
	if(it != userMedals.end()) {
		return it->second;
	}
	static simeffect_t noeffect;
	return noeffect;
}

void SessionMemberListImp::onEffectTimeout()
{
	simeffect_doc_t::iterator it = userMedals.begin();
	std::vector<uint32_t> removes;  
	for(; it != userMedals.end(); ++it) {
		simeffect_t& ueff = it->second;
		bool change = false;
		bool remove = true;
		for(int j = 0; j < ueff.size(); ++j) {
			if (ueff[j].tick > EFFECT_TIME_TICK) {
				ueff[j].tick -= EFFECT_TIME_TICK;
				remove = false;
			} else if(ueff[j].tick != 0){
				ueff[j].type = 0;
				ueff[j].tick = 0;
				change = true;
			}
		}
		if (change == true) {
			XConnPoint<IMedalEvent>::forEachWatcher1(&IMedalEvent::onMedalChange, it->first);
		}
		if (remove == true) {
			removes.push_back(it->first);
		}
	}
	for(int i = 0; i < removes.size(); ++i) {
		userMedals.erase(removes[i]);
	}
}


void SessionMemberListImp::remvoeUserEffect(uint32_t uid)
{
	userMedals.erase(uid);
}


void SessionMemberListImp::mutedSelf(bool bEnable)
{
	if (bEnable == selfMuted) {
		return;
	}
	XCast<PMutedSelf> mu;
	mu.muted = bEnable;
	pSession->SendNprotoRequest(mu);
}

bool SessionMemberListImp::isSelfMuted() const 
{
	return selfMuted;
}

void SessionMemberListImp::setSelfMuted(bool m)
{
	selfMuted = m;
}


void  SessionMemberListImp::onUpdateClientInfos(XCast<PUpdateClientInfos>& cmd, nproto::connection* c)
{
	for(std::vector<PUpdateClientInfo>::iterator it = cmd.updates.begin(); it != cmd.updates.end(); ++it)
	{
		people_doc_t::iterator pit = people.find(it->uid);
		if(pit != people.end()){
			pit->second.setClientInfo(net2app(it->client_info).c_str());
			//pit->second.nick += net2app(it->client_info).c_str();
		}
	}
		
}

LPCWSTR SessionMemberListImp::getClientInfo(const UID_T &uid) const
{
		people_doc_t::const_iterator it = people.find(uid);
		if(it != people.end()){
			return it->second.client_info.c_str();
		}
		return _T(""); 
}

CxDevice SessionMemberListImp::getClientDevice(const UID_T &uid) const
{
		people_doc_t::const_iterator it = people.find(uid);
		if(it != people.end()){
			return it->second.device;
		}
		return dev_winpc; 
}


VipLevel SessionMemberListImp::getVipLevel(const UID_T& uid) const {
	people_doc_t::const_iterator it = people.find(uid);
	if(it != people.end()){
			return it->second.vip;
	}
	return vip_null; 
}

void SessionMemberListImp::OnGetMemberUserInfo() {
	if (pSession->GetRoler(pSession->GetUid(), 0) >= protocol::session::MEMBER) {
		if (uinfo_requests_map_.size() < 100) {
			member_doc_t::iterator it_member = members.begin();
			for (; it_member != members.end(); ++it_member) {
				people_doc_t::iterator it_people = people.find(it_member->first);
				if (it_people == people.end() && it_member->second.nick.size() == 0) {
					uinfo_requests_map_[it_member->first] = 2;
				}
			}
			if (!uinfo_timer_.hasStart()) {
				uinfo_timer_.start(1000);
			}
			member_uinfo_timer_.stop();
		}
	}
}

void SessionMemberListImp::setDisplayNames(const std::vector<uint32_t>& uids) {
	XCast<PGetUserInfo> get_user_infos;
	for(int i = 0; i < uids.size(); ++i) {
		std::map<uint32_t, uint8_t>::iterator it = uinfo_requests_map_.find(uids[i]);
		if (it != uinfo_requests_map_.end() && it->second < 3) {
			get_user_infos.uids.push_back(uids[i]);
			it->second += 1;
		}
	}
	if (pSession->GetServerVserion() == 200 && get_user_infos.uids.size()) { 
		pSession->SendNprotoRequest(get_user_infos);
	}
}

void SessionMemberListImp::OnGetUserInfo() {
	if (bGetMember && bSyncMemberTime > 2 && timeGetTime() - last_session_jifen_ > 5000) {
		last_session_jifen_ = timeGetTime();
		XCast<PGetSessionMemberCredit> xx;
		xx.sid_ = pSession->GetSessionId();
		if (pSession->GetServerVserion() == 200) {
				pSession->SendNprotoRequest(xx);
				bSyncMemberTime--;
		}
	}

	last_session_jifen_ = timeGetTime();
	XCast<PGetUserSessionCredit> get_user_session_credit;
	std::set<uint32_t>::const_iterator it_us = usjifen_requests_.begin();
	for(; it_us != usjifen_requests_.end(); ++it_us) {
		get_user_session_credit.uids_.push_back(*it_us);
		//if (get_user_session_credit.uids_.size() > 50) {
		//	break;
		//}
	}
	get_user_session_credit.sid_ = pSession->GetSessionId();
	if (get_user_session_credit.uids_.size()) {
		if (pSession->GetServerVserion() == 200) {
			pSession->SendNprotoRequest(get_user_session_credit);
		}
		usjifen_requests_.clear();
	}

 	if (uinfo_requests_map_.empty()) {
		uinfo_timer_.start(1000);
		return;
	}
	
	XCast<PGetUserInfo> get_user_infos;
	int count = 0;
	

	for (std::set<uint32_t>::iterator it_fast = fast_sync_uinfo_.begin(); it_fast != fast_sync_uinfo_.end() && count++ < 200; ++it_fast) {
		if (uinfo_requests_map_.find(*it_fast) != uinfo_requests_map_.end()) {
			get_user_infos.uids.push_back(*it_fast);
		}
	}

	std::map<uint32_t, uint8_t>::iterator it;

	for ( it = uinfo_requests_map_.begin(); it != uinfo_requests_map_.end() && count++ < 200;) {
		if (fast_sync_uinfo_.find(it->first) != fast_sync_uinfo_.end()) {
			continue;
		}
		get_user_infos.uids.push_back(it->first);
		it->second++;
		if (it->second > 100) {
			uinfo_requests_map_.erase(it++);
		} else {
			it++;
		}
	}
	uinfo_timer_.start(500); //修正更长的timer
	fast_sync_uinfo_.clear();
	if (pSession->GetServerVserion() == 200 && get_user_infos.uids.size()) { 
		pSession->SendNprotoRequest(get_user_infos);
	}
}

void SessionMemberListImp::UpdateUserInfos(const std::vector<UserInfo>& userinfos) {
	std::set<uint32_t> uids;
	std::vector<UserInfo>::const_iterator it;
	for(it = userinfos.begin(); it != userinfos.end(); ++it) {
		uids.insert(it->uid);
		uinfo_requests_map_.erase(it->uid);
		people_doc_t::iterator it_people = people.find(it->uid);
		if (it_people != people.end()) {
			it_people->second.nick = filterDisplayString(net2app(it->nick).c_str());
			it_people->second.sex = (protocol::uinfo::SEX)it->gender;
			it_people->second.user_jifen = it->user_jifen;
			it_people->second.usess_jifen = 0;

			//XConnPoint<IRoomEvent>::forEachWatcher2(&IRoomEvent::onChange, ITreeNode(it->uid, ITreeNode::E_USER), ITreeNode(it_people->second.pid));
		}

		people_doc_t::iterator it_people_2 = get_people_from_servers_.find(it->uid);
		if (it_people_2 != get_people_from_servers_.end()) {
			it_people_2->second.nick = filterDisplayString(net2app(it->nick).c_str());
			it_people_2->second.sex = (protocol::uinfo::SEX)it->gender;
			it_people_2->second.user_jifen = it->user_jifen;
			it_people_2->second.usess_jifen = 0;
		}


		member_doc_t::iterator it_member = members.find(it->uid);
		if (it_member != members.end()) {
			MemberItem& item = members[it->uid];
			item.nick = filterDisplayString(net2app(it->nick).c_str());
			item.gender = (protocol::uinfo::SEX)it->gender;
			item.user_jifen = it->user_jifen;
			//item.usess_jifen = 0;
		}
	}
	XConnPoint<IUserInfoEvent>::forEachWatcher1(&IUserInfoEvent::onRefresh, uids);
}

//void  SessionMemberListImp::OnSeqMemberChanges(XCast<PSeqPushSessData>& cmd, nproto::connection* c) {
//	if (core::Request::ifSuccess(cmd.resCode) == true && pSession->CheckSeq(cmd.sequence_)) {
//		RealMemberChanges(&cmd, c);
//	}
//}

void  SessionMemberListImp::RealMemberChanges(PPushSessData* push_sess_data, nproto::connection* c) {
		PPushSessData& cmd = *push_sess_data;
		for(std::vector<POnMemberAdd>::iterator it = cmd.adds.begin(); it != cmd.adds.end(); ++it)
		{
				onMemberAdd(&*it, c);
		}

		for(std::vector<UID_T>::iterator it = cmd.removes.begin(); it != cmd.removes.end(); ++it)
		{
			people_doc_t::iterator pit = people.find(*it);
			if(pit != people.end())
			{
					XConnPoint<IRoomEvent>::forEachWatcher2(&IRoomEvent::onRemove,
															ITreeNode(*it, ITreeNode::E_USER), ITreeNode(pit->second.pid));
					//some baby have leave
					member_doc_t::const_iterator it_m = members.find(*it);
					if ( it_m != members.end())	{
						XConnPoint<IMemberListEvent>::forEachWatcher1(&IMemberListEvent::onMemberChange, *it);
					}
					people.erase(pit);
					remvoeUserEffect(*it); //
					uinfo_requests_map_.erase(*it);
			}
		}

		for(std::vector<std::pair<UID_T, SID_T> >::iterator it = cmd.changes.begin(); 
				it != cmd.changes.end(); ++it){
						UID_T who = it->first;
						SID_T t = it->second;

						assert(t);

						people_doc_t::iterator pt = people.find(who);
						if(pt != people.end()){
								SID_T f = pt->second.pid;
								pt->second.pid = t;

								XConnPoint<IRoomEvent>::forEachWatcher3(&IRoomEvent::onMoved, 
										ITreeNode (who, ITreeNode::E_USER), ITreeNode(f), ITreeNode(t));

								if(f != t){
									if(who == pSession->GetUid()){
										pSession->CurrectChannelChanged(f, t);
								}
									
						}
				}
		}

		for (std::vector<uint32_t>::iterator fit = cmd.folders.begin(); fit != cmd.folders.end(); ++fit)
		{
				HULI ss = *fit;
				channal_doc_t::iterator it = channels.find(ss);
				if(it != channels.end()){
						UID_T pid = it->second.getPid();

						XConnPoint<IRoomEvent>::forEachWatcher2(&IRoomEvent::onRemove,
										ITreeNode(ss), ITreeNode(pid));

						for(channal_doc_t::iterator cit = channels.begin(); cit != channels.end();){
							if(cit->second.getPid() == ss){
								cit = channels.erase(cit);
							}else{
								++cit;
							}
						}
						channels.erase(ss);
				}else{
						//assert(false);
				}
		}
}

//void  SessionMemberListImp::OnSeqSubChannelAdd(XCast<PSeqOnSubChannelAdd>& cmd, nproto::connection* c) {
//	if (core::Request::ifSuccess(cmd.resCode) == true && pSession->CheckSeq(cmd.sequence_)) {
//		RealAddSubChannel(&cmd);
//	}
//}

void SessionMemberListImp::RealAddSubChannel(POnSubChannelAdd* sub_channel_add) {
	POnSubChannelAdd& cmd = *sub_channel_add;
	pSession->UpdateChannelInfo(cmd.sid, cmd.info);

	ChannelItem item;
	item.setNick(net2app(sox::properties::get(cmd.info, SIT_NAME)).c_str());
	item.setPid(sox::properties::getint(cmd.info, SIT_PID));
	item.setPasswd(sox::properties::getint(cmd.info, SIT_BHASPASSWD) != 0);

	HULI ss = cmd.sid;
	channels[ss] = item;

	XConnPoint<IRoomEvent>::forEachWatcher2(&IRoomEvent::onAdd, 
		ITreeNode(ss), ITreeNode(item.getPid()));

	updateChannelOrder(ss, cmd.preChannel);  //最後才update order,通知客戶端重新排序
}



// member
void SessionMemberListImp::RefreshMemberList()
{
	bGetMember = true;
	SessionImp::UserRolers& user_roles = pSession->user_rolers_;
	for(SessionImp::UserRolers::const_iterator it = user_roles.begin(); it != user_roles.end(); ++it) {
		if (it->second.size() && it->second.front().roler <= protocol::session::OWNER) {
			member_doc_t::iterator it_member = members.find(it->first);
			if (it_member != members.end()) {
				it_member->second.roler = it->second.front().roler;
			} else {
				MemberItem& item = members[it->first];
				item.roler = it->second.front().roler;

				people_doc_t::iterator it_people = people.find(it->first);
				if (it_people != people.end()) {
					item.nick = it_people->second.nick;
					item.gender = it_people->second.sex;
					item.user_jifen = it_people->second.user_jifen;
					item.usess_jifen = it_people->second.usess_jifen;
					item.addtime = 0;
				} else {
					//uinfo_requests_map_[it->first] = 2;
					item.user_jifen = 0 ;
					item.usess_jifen =  0;
					item.gender = protocol::uinfo::female;
					item.addtime = 0;
				}
			}
		}
	}
	//if (!uinfo_timer.hasStart()) {
	//	uinfo_timer_.start(1000);
	//}
	member_uinfo_timer_.start(5000);
	XConnPoint<IMemberListEvent >::forEachWatcher0(&IMemberListEvent::onMemberRefresh);
}

void SessionMemberListImp::OnGetUserSessionCreditRes(XCast<PGetUserSessionCreditRes>& cmd, nproto::connection* c) {
	std::vector<std::pair<uint32_t, uint32_t> >::const_iterator it = cmd.uids_scredit_.begin();
	for (; it !=  cmd.uids_scredit_.end(); ++it) {
		user_session_jifen_[(*it).first] = (*it).second;
	}

	XConnPoint<IMemberCreditEvent>::forEachWatcher1(&IMemberCreditEvent::onRefresh, 
		cmd.uids_scredit_);
}

int API_StringToTime(const std::string &strDateStr,time_t &timeData)
{
    char *pBeginPos = (char*) strDateStr.c_str();
    char *pPos = strstr(pBeginPos,"-");
    if(pPos == NULL)
    {
        return -1;
    }
    int iYear = atoi(pBeginPos);
    int iMonth = atoi(pPos + 1);
 
    pPos = strstr(pPos + 1,"-");
    if(pPos == NULL)
    {
        return -1;
    }
 
    int iDay = atoi(pPos + 1);
 
    struct tm sourcedate;
    memset((void*)&sourcedate, 0, sizeof(sourcedate));
    sourcedate.tm_mday = iDay;
    sourcedate.tm_mon = iMonth - 1; 
    sourcedate.tm_year = iYear - 1900;
    
    timeData = mktime(&sourcedate);  
 
    return 0;
}
/*
void SessionMemberListImp::OnGetSessionMemberCreditRes(XCast<PGetSessionMemberCreditRes>& cmd, nproto::connection* c) {
	time_t t;time(&t);

	std::string time = app2net(pSession->getSessionInfo()->getCreateTime());
	API_StringToTime(time, t);

	for(std::vector<MemberInfo>::const_iterator it = cmd.user_credit_.begin(); it != cmd.user_credit_.end(); ++it) {
		uint32_t uid = (*it).uid_;
		member_doc_t::iterator it2 = members.find(uid);
		if(it2 != members.end()){
			it2->second.usess_jifen = (*it).credit_;
			it2->second.addtime = (*it).join_time_;

			if (it2->second.roler == OWNER || it2->second.addtime == 0) {
				
				it2->second.addtime = t;
			}
		}
		user_session_jifen_[uid] = (*it).credit_;
	}
	if (bGetMember)
		bSyncMemberTime = 0;



	static std::vector<std::pair<uint32_t, uint32_t> > sc;
	XConnPoint<IMemberCreditEvent>::forEachWatcher1(&IMemberCreditEvent::onRefresh, 
		sc);
}
*/

void SessionMemberListImp::getNamesFromServer(const std::vector<uint32_t>& uids) {
	for(int i = 0; i < uids.size(); ++i) {
		people_doc_t::iterator it = get_people_from_servers_.find(uids[i]);
		if (it != get_people_from_servers_.end()) {

		} else {
			people_doc_t::iterator it2 = people.find(uids[i]);
			if (it2 != people.end()) {
				get_people_from_servers_[uids[i]] = it2->second;	
			} else {
				if (uinfo_requests_map_.find(uids[i]) == uinfo_requests_map_.end()) {
					uinfo_requests_map_[uids[i]] = 0;
				}
				PeopleItem& people = get_people_from_servers_[uids[i]];
				people.pid = 0;
			}
		}
	}
}

void SessionMemberListImp::UpdateUserInfo(uint32_t uid) {
	uinfo_requests_map_[uid] = 3;

}