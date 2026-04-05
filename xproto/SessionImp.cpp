#include "sessionimp.h"
#include "SessionMemberListImp.h"
#include "RulerChecker.h"
#include "SessionInfo.h"
#include "MicrophoneListImp.h"
#include "SessionManager.h"
#include "VoiceDataManager.h"
#include "sessionpluginimp.h"
#include <mmsystem.h>

#include "common/core/ilink.h"
#include "common/core/form.h"
#include "protocol/psinfo.h"

#include "protocol/psmemberlist.h"
#include "protocol/pkg_tag.h"
#include "protocol/pmsg.h"
#include "common/res_code.h"
#include "helper/utf8to.h"
#include "common/nsox/nulog.h"
#include <algorithm>
#include <numeric>
#include "WsTcpSocket.h"
#include "wsudpsocket.h"
#include "location/default.h"
#include "helper/utf8to.h"
#include <common/version.h>
#include "helper/helper.h"
#include "helper/ufile.h"
#include "report.h"





#define MAX_TEXTCHAT_BYTE_SIZE 5120
#define MAX_VOICECHAT_BYTE_SIZE 900  // < 1024
#define TOKEN_PING_TIME 200 * 1000
#define UDP_PING_TIME 3 * 1000
#define MAX_RELOG_TIME 2

using namespace protocol::session;
using namespace protocol;
using namespace misc;
using namespace core;

using namespace xproto;

extern bool	__is_voices_fatal_log_on;

CRSAKey SessionImp::rsakey_;

BEGIN_REQUEST_MAP_EXT(SessionImp)
REQUEST_HANDLER(XCast<PChatVoice>,		onVoice)
REQUEST_HANDLER(XCast<PBroadcasts>,		onBroadCasts)
REQUEST_HANDLER(XCast<PReSendVoiceRes>,	onReSendVoices)
REQUEST_HANDLER(XCast<PEmptyChatVoice>, onEmptyVoices)
REQUEST_HANDLER(XCast<PExpandTypeVoice>,	onExpandTypeVoice)
REQUEST_HANDLER(XCast<PExpandQcVoice>,	onExpandQcVoice)
REQUEST_HANDLER(XCast<PReSendTypeVoiceRes>,	onReSendTypeVoices)
REQUEST_HANDLER(XCast<PReSendQcVoiceRes>,	onReSendQcVoices)
REQUEST_HANDLER(XCast<PReSendBcVoiceRes>,	onReSendBcVoices)
REQUEST_HANDLER(XCast<PReSendVoice>,	onResendVoice)
REQUEST_HANDLER(XCast<POnVoiceRes>,	OnVoiceRes)


REQUEST_HANDLER(XCast<PChatText>,		onText)
REQUEST_HANDLER(XCast<PSeqChatText>,		onSeqText)
REQUEST_HANDLER(XCast<PBroadcastVoice>,		onBroadcastVoice)
REQUEST_HANDLER(XCast<PBroadcastText>,		onBroadcastText)

REQUEST_HANDLER(XCast<POnSessKickOff>,	onKickOff)
REQUEST_HANDLER(XCast<PRingBusy>,		onRingBusy)
REQUEST_HANDLER(XCast<PChangeFolder>,	onChangeFolder)

REQUEST_HANDLER(XCast<PSetKeyActive>,	onSetKeyActive)
REQUEST_HANDLER(XCast<PSetRoomKeyActive>, onSetRoomKeyActive)
REQUEST_HANDLER(XCast<PPInfoChanged>,	onNickChange)

REQUEST_HANDLER(XCast<PStatusChanged>,	onStatusChanged)
REQUEST_HANDLER(XCast<PSyncUStatusRes>,	onSyncUStatus)

REQUEST_HANDLER(XCast<PNotifySession>,	onNotifySession)
REQUEST_HANDLER(XCast<PDisableVoice>,	onDisableVoice)
REQUEST_HANDLER(XCast<PDisableText>,	onDisableText)
REQUEST_HANDLER(XCast<PMutedSelfRes>,	onMutedSelf)

REQUEST_HANDLER(XCast<PUdpLoginRes>,	onUdpConnected)

REQUEST_HANDLER(XCast<PRealJoinChannelRes>, onSync)
REQUEST_HANDLER(XCast<PZRealJoinChannelRes>, onZSync)

REQUEST_HANDLER(XCast<POnMemberUpdated>, onRolerChanged)
REQUEST_HANDLER(XCast<POnChannelInfoUpdated>, onChannelInfoChanged)
REQUEST_HANDLER(XCast<POnSyncManagers>, onSyncManager)
REQUEST_HANDLER(XCast<PStopVoice>,		onStop)
REQUEST_HANDLER(XCast<POnGetBans>,		onGetBans)

REQUEST_HANDLER(XCast<PUpdateBanIDRes>,	onBanUser)
REQUEST_HANDLER(XCast<PUpdateBanIpRes>,	onBanIp)
REQUEST_HANDLER(XCast<PBatchUpdateBanIDRes>,	onBatchBanUser)
REQUEST_HANDLER(XCast<PBatchUpdateBanIpRes>,	onBatchBanIp)
REQUEST_HANDLER(XCast<PTcpConnect>, onTcpConnect)


REQUEST_HANDLER(XCast<PUpdateKickOffRes>, onUpdateKickOffList) 
REQUEST_HANDLER(XCast<PZMemberList>, onMemberList) 
REQUEST_HANDLER(XCast<PZMemberHistory>, onMemberHistory) 
REQUEST_HANDLER(XCast<PMemberText>,  onMemberText)

REQUEST_HANDLER(XCast<PNotice>,  onNotice)
REQUEST_HANDLER(XCast<PRemoveKickOffRes>,  onRemoveKickOff)
REQUEST_HANDLER(XCast<PSynOrder>, onSynOrder) 
REQUEST_HANDLER(XCast<PSessionLastError>, onSessionLastError)
REQUEST_HANDLER(XCast<PZMemberCards>, onMemberCards)  
REQUEST_HANDLER(XCast<PUpdateSessInfoRes>, onUpdateSessInfo)  
REQUEST_HANDLER(XCast<PBroadcastEvent>,		onBroadCastEvent)


//REQUEST_HANDLER(XCast<PRealJoinSessionRes>, onJoinSessionRes)


REQUEST_HANDLER(XCast<PJoinSessionRes>, OnJoinSesionRes)
REQUEST_HANDLER(XCast<PGetSessionMainRes>, OnGetSessionMainRes)
REQUEST_HANDLER(XCast<PGetSessionAuthRes>, OnGetSessionAuthRes)
REQUEST_HANDLER(XCast<PGetDisableVoiceRes>, OnGetDisableVoiceRes)
REQUEST_HANDLER(XCast<PGetDisableTextRes>, OnGetDisableTextRes)
REQUEST_HANDLER(XCast<PGetUserRolesRes>, OnGetUserRolesRes)
REQUEST_HANDLER(XCast<PGetUserInfoRes>, OnGetUserInfoRes)
REQUEST_HANDLER(XCast<PGetSeqListRes>, OnGetSeqListRes)
REQUEST_HANDLER(XCast<PGetBackUpProtocolFrontsRes>, OnGetBackUpProtocolFrontsRes)
//REQUEST_HANDLER(XCast<PGetExtendChannelInfoRes>, OnGetExtendChannelInfoRes)

//REQUEST_HANDLER(XCast<PGetVoiceTokenRes>, OnGetVoiceTokenRes)
//REQUEST_HANDLER(XCast<PMemberCards>, OnGetMemberCards)

CHAIN_REQUEST_MAP_PTR_MEMBER(session_member_list_)
CHAIN_REQUEST_MAP_PTR_MEMBER(microphone_list_imp_)
CHAIN_REQUEST_MAP_PTR_MEMBER(plugin_manager_)

END_REQUEST_MAP()


SessionImp::SessionImp() {
	ReseqTextSeq();
	voice_packer_segments_ = 2;
	default_packet_duration_ = 80.0;
	conn_manager_ = new FrontConnectionManager(this);
	srand(GetStampMS());
	client_id_ = rand();
	server_version_ = 0;
	voice_token_timer_.init(this, &SessionImp::UpdateVoiceToken);
	voice_token_timer_.start(TOKEN_PING_TIME);
}

SessionImp::~SessionImp(void)
{
	nsox::xlog(NsoxInfo, "session delete, real sid: %d", session_id_);
	if (rule_checker_) {
		delete rule_checker_;
	}
	if (voice_mgr_) {
		delete voice_mgr_;
	}
	if (conn_manager_) {
		delete conn_manager_;
	}
}

void SessionImp::InitData() {
	voice_no_res_ = 0;
	is_relogin_ = false;
	is_onvaild_ = false;
	currect_channel_ = 0;
	//__connStat = INIT_STAT;

	session_member_list_ = nu_create_object_local<SessionMemberListImp>::create_inst();
	microphone_list_imp_ = nu_create_object_local<MicrophoneListImp>::create_inst();
	session_info_ = nu_create_object_local<SessionInfo>::create_inst();
	plugin_manager_ = nu_create_object_local<CPlugInManager>::create_inst();

	session_info_->setSession(this);
	rule_checker_ = new RulerChecker(this, uid_);
	voice_mgr_ = new VoiceDataManager();

	session_member_list_->setSession(this);
	microphone_list_imp_->setSession(this);
	plugin_manager_->setSession(this);
	voice_mgr_->set_packet_notify(this);
	voice_mgr_->set_voice_pumper(this);
	voice_mgr_->set_voice_quality(this);
	voice_mgr_->SetSid(this->GetSessionId());
}

void SessionImp::SetToken(const xstring& token) {
	user_token_ = token;;
}


void SessionImp::InitByToken(uint32_t s, uint32_t uid, const xstring& token, 
							 const xstring& channel_password, ISessionHandler* h,  uint32_t version) {
	 alias_id_  = s;
	 session_id_ = s;
	 session_handler_ = h;
	 uid_ = uid;
	 user_password_.clear();
	 user_token_ = token;
	 last_input_channel_password_	= app2net(channel_password);
	 client_version_ = version;
	 uid_ = uid;
	 InitData();
}

void SessionImp::Init(uint32_t s, uint32_t uid, const xstring& passwd, 
					  const xstring& channel_password, ISessionHandler* h,  uint32_t version) {
	  alias_id_  = s;
	  session_id_ = s;
	  session_handler_ = h;
	  uid_ = (uid);
	  user_password_	= (passwd);
	  last_input_channel_password_ = app2net(channel_password);
	  client_version_ = version;
	  uid_ = uid;
	  user_token_.clear();
	  InitData();
}

void SessionImp::SetManager(SessionManager *m) {
	session_manager_ = m;
}

void SessionImp::ChangeNick(const std::string &new_nick, uinfo::SEX g, const std::string &sign) {
	XCast<PChangePInfo> cn;
	cn.nick = new_nick;
	cn.gender = g;
	cn.sign = sign;
	SendNprotoRequest(cn);
}

void SessionImp::SetAliasId(SID_T asid) {
	alias_id_ = asid;
}

void SessionImp::SetSessionId(SID_T sid) {
	session_id_ = sid;
}

SID_T SessionImp::GetSessionId() const {
	return session_id_;
}

SID_T SessionImp::GetAliasId() const {
	return alias_id_;
}

UID_T SessionImp::GetUid() const {
	return uid_;
}

bool SessionImp::Start(const std::string &ip, const std::vector<uint16_t> &ports) {
	//voice_front_.SetSession(this);
	

	if(ports.empty()){
		nsox::xlog(NsoxInfo, "no ports avaliable for session, stop start");
		return false;
	}
	port_ = ports[rand() % (ports.size())];
	ip_ = ip;
	conn_manager_->Start(ip_.data(), port_);
	return true;
}



/*****************************************
 * interface can not modify
 */
SID_T SessionImp::getSid() {
	return session_id_;
}

uint32_t SessionImp::getCur()
{
	return currect_channel_;
}


bool SessionImp::setInitChannel(uint32_t channelId) {
	if (currect_channel_ == 0) {
		currect_channel_ = channelId;
		return true;
	}
	return false;
}

void SessionImp::setReception(SID_T channel_id){
	if (rule_checker_ && rule_checker_->canDo(&SessOperation(SessOperation::EDIT_CHANNEL_INFO,	 GetSessionId()))) {
		if(session_info_){
			session_info_->setReception(channel_id);
			session_info_->update();
		}
	}
}

SID_T SessionImp::getReception() {
	if (session_info_){
		uint32_t reception = session_info_->getReception();
		if  (reception != 0 && reception != GetSessionId())
			return reception;
	}
	return 0;
}

void SessionImp::setHall(SID_T channelId){
	if (rule_checker_ && rule_checker_->canDo(&SessOperation(SessOperation::EDIT_CHANNEL_INFO,	 0))) {
		if(session_info_){
			session_info_->setReception(channelId);
			session_info_->update();
		}
	}
}

SID_T SessionImp::getHall() {
	if (session_info_) {
		uint32_t hall = session_info_->getHall();
		return hall;
	}
	return 0;
}

ISessionMemberList* SessionImp::getSessionList() {
	return session_member_list_;
}

IChannelInfo* SessionImp::getChannelInfo(const SID_T &channel_id)
{
	ChannelInfo::const_iterator it = channel_infos_.find(channel_id);
	if(it != channel_infos_.end()){
		return it->second;
	}
	ASSERT(channel_id == 0);
	return NULL;
}

ISessionInfo* SessionImp::getSessionInfo()
{
	return session_info_;
}

IMicrophoneList* SessionImp::getMicrophoneList()
{
	return microphone_list_imp_;
}

IAuthChecker* SessionImp::getAuthChecker()
{
	return rule_checker_;
}

void SessionImp::changeStyle(ChannelStyle s)
{
	if (rule_checker_ && rule_checker_->canDo(&SessOperation(SessOperation::CHANGESTYLE))) {
		SessionInfo *info = (SessionInfo *)getChannelInfo(getCur());
		if(info){
			info->setStyle(s);
			//info->setOperator(__pManager->uid);
			info->update();
		}
	}
}

ChannelStyle SessionImp::getCurrentStyle()
{
	SessionInfo *info = (SessionInfo *)getChannelInfo(getCur());
	if (info) {
		return info->getStyle();
	}
	return FreeStyle;
}

void SessionImp::createSubChannel(const SessionCreateInfo &info)
{
	if (rule_checker_ && rule_checker_->canDo(&SessOperation(SessOperation::CREATE_SUBCHANNEL_CHK,	 info.pid ))) {
		PCreateSubChannel sub(app2net(info.name), info.islimit, info.isPub, app2net(info.hello),  info.password,  info.pid, info.qc, info.preCh);
		XCast<PCreateSubChannel> req(sub);
		SendNprotoRequest(req);
	}
}

void SessionImp::dismissSubChannel(const UID_T& sid)
{
	if (rule_checker_ && rule_checker_->canDo(&SessOperation(SessOperation::DISSMISS_SUBCHANNEL_CHK, sid)))
	{
		XCast<PDismissSubChannel> m;
		m.sid = sid;
		SendNprotoRequest(m);
	}
}

int SessionImp::say(const TextChat &text)
{
	
	if(rule_checker_ && rule_checker_->canDo(&SessOperation(SessOperation::TEXT) )) {	
		localStringA chatText;
		text.toTString(chatText);
#ifndef DEBUG
		if(chatText.size() > MAX_TEXTCHAT_BYTE_SIZE){
			return ISession::TEXT_OVERFLOW;
		}
#endif

		if (port_ >= 10000) {  // new version
			XCast<PSeqChatText> m ;
			m.sid = currect_channel_;
			m.chat.set(chatText.c_str(),chatText.size());
			m.from = uid_;
			SendNprotoRequest(m);
		} else {
			XCast<PChatText> m ;
			m.sid = currect_channel_;
			m.chat.set(chatText.c_str(),chatText.size());
			m.from = uid_;
			SendNprotoRequest(m);
		}
		return ISession::SENDED;
	}
	return 0;
}

int SessionImp::say(const std::string &voice, uint32_t seq) {
	voice_no_res_++;
	XCast<PChatVoice>  voice_request;

	voice_request.chat = voice;
	voice_request.seq = seq;
	voice_request.from = uid_;
	voice_request.sid = currect_channel_;

	sox::PackBuffer pb;
    sox::Pack pk(pb);
    voice_request.marshal(pk);	
	int send_ret = output_voice_queue_.Push(pk.data(), pk.size(), seq, voice_request.uri);

	if (send_ret == SENDED) {
        if (/*udp_socket_worked_ == SOCKET_WORKING*/ true) {
			protocol::session::PNormalVoiceData *voice_before_cast = output_voice_queue_.Get(seq - 2);
            if (voice_before_cast) {
                nsox::xlog(NsoxInfo,"speaking before seq %d  size %d ",voice_before_cast->seq, voice_before_cast->data.size());
				conn_manager_->SendRequest(*voice_before_cast, voice_before_cast->URI);
			}
        }
        conn_manager_->Send(voice_request);
    }

	if (voice_no_res_ > 400) {
		//conn_manager_->ChangeFirst();
	}
	return 0;
}

int SessionImp::say(const std::string &voice, uint32_t seq, uint8_t type) {
	voice_no_res_++;
	XCast<PExpandTypeVoice>  voice_request;
	voice_request.chat = voice;
	voice_request.seq = seq;
	voice_request.type = type;
	voice_request.from = uid_;
	voice_request.sid = currect_channel_;

	sox::PackBuffer pb;
    sox::Pack pk(pb);
    voice_request.marshal(pk); 
	int send_ret = output_voice_queue_.Push(pk.data(), pk.size(), seq, voice_request.uri);
	if (send_ret == SENDED) {
		bool net_work_good = conn_manager_->GetChangeMainTime() <= 2;
        if (net_work_good) {
			protocol::session::PNormalVoiceData *voice_before_cast = output_voice_queue_.Get(seq - 2);
            if (voice_before_cast) {
                nsox::xlog(NsoxInfo,"speaking before seq %d  size %d ",voice_before_cast->seq, voice_before_cast->data.size());
				conn_manager_->SendRequest(*voice_before_cast, voice_before_cast->URI);
            }
        }
        conn_manager_->Send(voice_request);
    }
	if (voice_no_res_ > 400) {
		//conn_manager_->ChangeFirst();
	}
	return 0;
}

int SessionImp::say(const std::string &voice, uint32_t seq, uint32_t qc) {
	voice_no_res_++;
	XCast<PExpandQcVoice>  voice_request;
	voice_request.chat = voice;
	voice_request.seq = seq;
	voice_request.qc = qc;
	voice_request.from = uid_;
	voice_request.sid = currect_channel_;

	sox::PackBuffer pb;
    sox::Pack pk(pb);
    voice_request.marshal(pk); 
	int send_ret = output_voice_queue_.Push(pk.data(), pk.size(), seq, voice_request.uri);
	if (send_ret == SENDED) {
        if (/*udp_socket_worked_ == SOCKET_WORKING*/ true) {
			protocol::session::PNormalVoiceData *voice_before_cast = output_voice_queue_.Get(seq - 2);
            if (voice_before_cast) {
                nsox::xlog(NsoxInfo,"speaking before seq %d  size %d ",voice_before_cast->seq, voice_before_cast->data.size());
				conn_manager_->SendRequest(*voice_before_cast, voice_before_cast->URI);
            }
        }
        conn_manager_->Send(voice_request);
    }
	if (voice_no_res_ > 400) {
		//conn_manager_->ChangeFirst();
	}
	return 0;
}

void SessionImp::leave()
{
	XCast<PLeaveSession> pl;
	pl.sid = 1;
	SendNprotoRequest(pl);
	session_manager_->initialEraseSession(this);
}

xstring SessionImp::getLastError() const
{
	return last_error_;
}

void SessionImp::stopVoice(bool broadcast)
{
	XCast<PStopVoice> stop; 
	stop.sid = currect_channel_;
	SendNprotoRequest(stop);
	SendNprotoRequest(stop);
	SendNprotoRequest(stop);
	output_voice_queue_.End();
	
	voice_no_res_ = 0;
	//voice_res_.clear();
}

void SessionImp::guangboVoice(const std::string &voice, const SID_T &chId, uint32_t seq)
{
	XCast<PBroadcastVoice>  voice_request;
	voice_request.chat = voice;
	voice_request.seq = seq;
	voice_request.from = uid_;
	voice_request.sid = chId;

	sox::PackBuffer pb;
    sox::Pack pk(pb);
    voice_request.marshal(pk); 
	int send_ret = output_voice_queue_.Push(pk.data(), pk.size(), seq, voice_request.uri);
	if (send_ret == SENDED) {
        if (/*udp_socket_worked_ == SOCKET_WORKING*/ true) {
			protocol::session::PNormalVoiceData *voice_before_cast = output_voice_queue_.Get(seq - 2);
            if (voice_before_cast) {
                nsox::xlog(NsoxInfo,"speaking before seq %d  size %d ",voice_before_cast->seq, voice_before_cast->data.size());
				conn_manager_->SendRequest(*voice_before_cast, voice_before_cast->URI);
            }
        }
        conn_manager_->Send(voice_request);
    }
}

void SessionImp::guangboText(const TextChat &text, const SID_T &chId)
{
	if (rule_checker_ && rule_checker_->canDo(&SessOperation(SessOperation::BROADCAST, chId))) {
		localStringA chatText;
		text.toTString(chatText);		
		if(chatText.size() < MAX_TEXTCHAT_BYTE_SIZE){
			XCast<PBroadcastText> m ;
			m.sid  = chId;
			m.chat.set(chatText.c_str(),chatText.size());
			m.from = uid_;
			SendNprotoRequest(m);
		}
	}
}

uint32_t SessionImp::getPing(void) {
	return conn_manager_->GetPing();
	//return __rtt;
}




void  SessionImp::restart()
{
	XCast<PRestartChannel> p;
	SendNprotoRequest(p);
}

void	 SessionImp::ChangeStatus(const xstring& status)
{
	XCast<PChangeStatus> p;
	std::string statusA = app2net(status);
	p.status = statusA;
	SendNprotoRequest(p);
}

void SessionImp::changeOrder(const std::vector<SID_T>& order){
	if (rule_checker_ && rule_checker_->canDo(&SessOperation(SessOperation::CHANGE_SESS_ORDER,	 getSid())))
	{
		nsox::xlog(NsoxInfo, "sort sub channel");
		XCast<PChangeOrder> req;
		req.sid = session_id_;
		req.order = order;
		SendNprotoRequest(req);
	}
}

int SessionImp::sayInMember(const TextChat &text)
{
	if (rule_checker_ && rule_checker_->canDo(&SessOperation(SessOperation::MEMBER_TEXT)))
	{
		XCast<PMemberText> req;
		req.from = uid_;
		text.toTString(req.chat);
		req.time = 0;
		if(req.chat.size() > MAX_TEXTCHAT_BYTE_SIZE){
			return ISession::TEXT_OVERFLOW;
		}else{
			SendNprotoRequest(req);
			return ISession::SENDED;
		}
	}
	return 0;
}



void	SessionImp::notice(NoticeType notice_type, const xstring& exp, uint32_t type, uint32_t resources , const xstring& info)
{

}

void   SessionImp::getKickOffList(const SID_T &cid)
{
	XCast<PUpdateKickOff> req;
	req.cid = cid;
	SendNprotoRequest(req);
}

void   SessionImp::removeKickOffList(const UID_T &uid, const SID_T &cid)
{
	if (rule_checker_ && rule_checker_->canDo(&SessOperation(SessOperation::KICKOFF, cid, uid)))
	{
		XCast<PRemoveKickOff> req;
		req.uid = uid;
		req.cid = cid;
		SendNprotoRequest(req);
	}
}


XVoiceData* SessionImp::fetchVoices(VoiceType vt)
{
	if(voice_mgr_){
		return voice_mgr_->fetch(0, vt);
	}
	return NULL;
}

XVoiceData* SessionImp::FetchBroadcastVoices()
{
	if(voice_mgr_){
		return voice_mgr_->fetch(0, using_broadcast);
	}
	return NULL;
}


void SessionImp::releaseVoice(XVoiceData* vd)
{
	if(voice_mgr_){
		voice_mgr_->releaseVoice( vd );
	}
}

void SessionImp::BroadcastsEvent(uint32_t sid, uint32_t code, const xstring& str)
{
	if (rule_checker_ && rule_checker_->canDo(&SessOperation(SessOperation::BROADCAST,	sid))) {
		XCast<PBroadcastEvent> cmd;
		cmd.from = uid_;
		cmd.code = code;
		cmd.info = app2net(str);
		cmd.sid = sid;
		SendNprotoRequest(cmd);
	}
}




void SessionImp::initVoice(bool init)
{
}

void SessionImp::setSegmentInPack(uint8_t s) {
	voice_packer_segments_ = s;
}

void SessionImp::setDefaultPackTime(double t) {
	default_packet_duration_ = t;
}

bool SessionImp::IsLimit()
{
	return getSessionInfo()->isLimit();
}


/*
void SessionImp::setRealSid(SID_T rsid) {
session_id_ = rsid;
}

void SessionImp::setAliaSid(SID_T asid) {
alias_id_ = asid;
}
*/

/*******************************
* check 
*/
bool SessionImp::IsChannelTextable(SID_T ch) {
	return session_member_list_->isChTextable(ch);
}

bool SessionImp::IsChannelVoiceable(SID_T ch) {
	return session_member_list_->isChVoiceable(ch);
}


bool SessionImp::IsVip(UID_T uid) {
	return GetRoler(uid, GetSessionId()) == VIP;
}

bool SessionImp::IsTVip(UID_T uid) {
	return GetRoler(uid, GetSessionId()) == TMPVIP;
}

void SessionImp::IgnoreText(UID_T uid, bool bi) {
	if(bi)
		text_ignores_.insert(uid);
	else
		text_ignores_.erase(uid);
}

void SessionImp::IgnoreVoice(UID_T uid, bool bi) {
	if(bi)
		voice_ignores_.insert(uid);
	else
		voice_ignores_.erase(uid);
}


bool SessionImp::IsIgnoreText(UID_T uid) {
	return text_ignores_.find(uid) != text_ignores_.end();
}

bool SessionImp::IsIgnoreVoice(UID_T uid) {
	return voice_ignores_.find(uid) != voice_ignores_.end();
}


bool SessionImp::IsRecurePid(const SID_T& subid, const SID_T& pid) {
	HULI psid = session_member_list_->innerGetChannelPid(subid);
	if(psid != SID_NULL){
		if(psid == pid){
			return true;
		}else{
			HULI ppid = session_member_list_->innerGetChannelPid(psid);
			if(!ppid != SID_NULL && ppid == pid){
				return true;
			}
		}
	}
	return false;
}

void SessionImp::StopVoiceRequest(uint32_t sid){
	XCast<PStopVoice> stop; 
	stop.sid = sid;

	SendNprotoRequest(stop);
	SendNprotoRequest(stop);
	SendNprotoRequest(stop);
}

void SessionImp::UpdateChannelInfoRequest(SID_T channel_id, sox::Properties &prop) { //更新频道资料请求
	if (rule_checker_ && rule_checker_->canDo(&SessOperation(SessOperation::EDIT_CHANNEL_INFO,	channel_id))) {
		if(channel_infos_.find(channel_id) != channel_infos_.end()){
			XCast<PUpdateChannelInfo> cmd;
			sox::properties::replace(cmd, prop);
			cmd.sid = channel_id;
			SendNprotoRequest(cmd);
		}	
	}
}

void SessionImp::UpdateSessionInfoRequest( sox::Properties &prop) { //更新群资料请求
	if (rule_checker_ && rule_checker_->canDo(&SessOperation(SessOperation::EDIT_CHANNEL_INFO,	0))) {
		XCast<PUpdateSessInfo> cmd;
		sox::properties::replace(cmd, prop);
		SendNprotoRequest(cmd);
	}
}

void SessionImp::NotifyMemberChange(const UID_T& uid)
{
	if(session_member_list_->isOnline(uid)){
		std::vector<ITreeNode> parents;
		session_member_list_->getParent(ITreeNode(uid), parents);
		for(std::vector<ITreeNode >::iterator it = parents.begin(); it != parents.end(); ++it){
			((XConnPoint<IRoomEvent>*)session_member_list_)->forEachWatcher2(&IRoomEvent::onChange, uid, it->value);
		}
		microphone_list_imp_->onChange(uid, SID_NULL);
	}
}

void SessionImp::NotifyEnableAllText(PEnableAllText *et)
{
	if(currect_channel_ == et->sid){
		XConnPoint<ISessionEvent>::forEachWatcher3(&ISessionEvent::onDisableAllText, 
			et->admin, 
			et->sid, 
			!et->enable);
	}
}

void SessionImp::NotifyTuoren(uint32_t admin, uint32_t from, uint32_t to)
{
	XConnPoint<ISessionEvent>::forEachWatcher3(&ISessionEvent::onTuoren, 
		admin, 
		from, 
		to);
}

void SessionImp::NotifySessionLbsError(ISessionHandler::Status st)
{
	session_handler_->onClose(st, this, WSAGetLastError());
}

void SessionImp::OnUserVoiceStatus(XCast<PUserVoiceStatus>& voice) {
	//if (voice_front_.Success()) {
	
	//}
	SendNprotoRequest(voice);
}

void SessionImp::OnPacketLoss(const UID_T &user, int seq)
{
	nsox::xlog(NsoxFatal, "user want to get resend voices user=%d,  seq=%d", user, seq);  //voicedubug
	XCast<PReSendVoice> resv;
	resv.uid = user;
	resv.seq = seq;
	SendNprotoRequest(resv);
}


bool SessionImp::CanSay() {
	if(rule_checker_ && rule_checker_->canDo(&SessOperation(SessOperation::VOICE)) )  {
		return true;
	}
	return false;
}

ChannelRoler SessionImp::GetMaxRoler(const UID_T &uid) {
	UserRolers::iterator it = user_rolers_.find(uid);
	ChannelRoler total = NORMAL;

	if(it != user_rolers_.end()){
		for(RolerPairVertor::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2){
			RolerPair &rp = *it2;
			if(rp.roler >= total){
				total = rp.roler;
			}
		}
	}
	return total;
}

ChannelRoler SessionImp::GetRoler(const UID_T &uid, const SID_T &channelId) {
	UserRolers::iterator it = user_rolers_.find(uid);
	ChannelRoler ret = NORMAL;
	ChannelRoler total = NORMAL;

	if(it != user_rolers_.end()){
		for(RolerPairVertor::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2){
			RolerPair &rp = *it2;
			if(rp.roler >= ret){
				if(rp.channelId == channelId || rp.channelId == SID_NULL){
					if(IsRecurePid(channelId, rp.channelId) && rp.roler == CMANAGER){
						ret = PMANAGER;
					}else{
						ret = rp.roler;
					}
				}else if(rp.channelId == SID_NULL &&rp.roler > total){
					total = rp.roler;
				}else if(IsRecurePid(channelId, rp.channelId) && rp.roler == CMANAGER){
					ret = PMANAGER;
				}else if(rp.roler == CMANAGER && ret < MEMBER){
					ret = MEMBER;
				}
			}
		}
	}
	return ret == NORMAL ? total : ret;
}

ChannelRoler SessionImp::GetChannelRoler(const UID_T &uid, const SID_T &channelId) {
	UserRolers::iterator it = user_rolers_.find(uid);
	if(it != user_rolers_.end()){
		ChannelRoler ret = NORMAL;
		for(RolerPairVertor::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2){
			RolerPair &rp = *it2;
			if(rp.channelId == channelId && rp.roler > ret){
				ret = rp.roler;
			}
		}
		return ret;
	}else{
		return NORMAL;
	}
}

void SessionImp::NotifyResult(int op, int res)
{
	XConnPoint<IActionEvent>::forEachWatcher2(&IActionEvent::onResult,op,res);
}


void    SessionImp::ChannelInfoKickOffAdded(const UID_T& uid, const SID_T sid) {
	//ChannelInfo::iterator fit = channel_infos_.find(GetSessionId());
	ChannelInfo::iterator fit = channel_infos_.find(sid);
	if(fit != channel_infos_.end()){
		fit->second->addKickOff(uid,  sid);
	}
	if (sid == 0) {
		this->session_info_->addKickOff(uid,  sid);
	}
}

/*
uint32_t SessionImp::GetLastErrorClick() const
{
	return last_error_Click;
}
*/

void SessionImp::AddRoler(UID_T uid, std::vector<MRolerAndCh> &rolers)
{
	RolerPairVertor rps;
	ChannelRoler max = NUL_ROLE;
	for(std::vector<MRolerAndCh>::iterator it = rolers.begin(); it != rolers.end(); ++it) {
		RolerPair rp;
		rp.channelId = it->cid;
		rp.roler = it->roler;
		if (max < rp.roler) {
			max = rp.roler;
		}
		rps.push_back(rp);
	}

	user_rolers_[uid] = rps;

	if (max >= MEMBER) {
		session_member_list_->addMember(uid,  _T(""), _T(""), protocol::uinfo::female, 0, 0, max);
	} else {
		session_member_list_->removeMember(uid);
	}
}

void SessionImp::CurrectChannelChanged(SID_T old_channel , SID_T currect_channel)
{
	//voice_front_.Start("192.168.50.81", 20001);
	uint64_t stream_id = session_id_;
	stream_id = stream_id << 32 + currect_channel_;
	//voice_front_.SetStreamId(stream_id);


	GetChannelExtendInfo(currect_channel);
	ReseqTextSeq();
	ResetVoiceManager();
	microphone_list_imp_->clearQueue();
	currect_channel_ = currect_channel;

	if (getCurrentStyle() == MicrophoneStyle) {
		XCast<PGetMicrophoneQueue> cmd;
		SendNprotoRequest(cmd);
	}
	//if (old_channel != 0) {
	//	XConnPoint<ISessionEvent>::forEachWatcher2(&ISessionEvent::onStyleChange, SID_NULL, true);
	//}
	XConnPoint<ISessionEvent>::forEachWatcher1(&ISessionEvent::onPosChange, old_channel);
	XConnPoint<ISessionEvent>::forEachWatcher2(&ISessionEvent::onStyleChange, SID_NULL, true);
	//if (old_channel == 0) {
	//	XConnPoint<ISessionEvent>::forEachWatcher2(&ISessionEvent::onStyleChange, SID_NULL, true);
	//}
}

void SessionImp::onBroadCasts(XCast<PBroadcasts>& cmd, nproto::connection* c)
{
	for (int i = 0; i < cmd.commands.size(); ++i) {
		if (cmd.commands[i].__exp != uid_) {
			int		plen = nproto::request::peek_len( cmd.commands[i].__data.c_str());
			uint32_t uri = nproto::request::peek_uri( cmd.commands[i].__data.c_str());
			if (plen <= cmd.commands[i].__data.size()) {
				this->process_request(uri,  cmd.commands[i].__data.c_str(), 
					cmd.commands[i].__data.size(), c);
			}
		}
	}
}




void SessionImp::ResetVoiceManager()
{
	voice_mgr_->clear();
	voice_mgr_->start();
}

void SessionImp::UpdateChannelInfo(uint32_t cid, sox::Properties props)
{
	ChannelInfo::iterator fit = channel_infos_.find(cid);
	if(fit != channel_infos_.end()){
		fit->second->setProperty(props);
		fit->second->onInfoChange();
	} else {
		channel_infos_[cid] = nsox::nu_create_object<SessionInfo>::create_inst();
		channel_infos_[cid]->setSession(this);
		channel_infos_[cid]->setProperty(props);
	}
}

void SessionImp::UpdateSessInfo(sox::Properties &prop)
{

}

void SessionImp::onUpdateSessInfo(XCast<PUpdateSessInfoRes>& cmd, nproto::connection* c) {


	if(core::Request::ifSuccess(cmd.resCode)) {
		SID_T old_reception = session_info_->getReception();
		session_info_->setProperty(cmd);
		session_info_->onInfoChange();
		UpdateSList();
		SID_T new_reception = session_info_->getReception();
		if (old_reception != new_reception) {
				XConnPoint<ISessionEvent>::forEachWatcher2(&ISessionEvent::onReceptionChange, 
										old_reception, 
										new_reception);
		}
		if (cmd.props.find(SIT_OPENPLUGIN) != cmd.props.end()) {
			plugin_manager_->openChange(session_info_->getOpenPlugin());
		}
	}
}



void SessionImp::onSessionLastError(XCast<PSessionLastError>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		last_error_ = net2app(cmd.lastError);
	}
}
 
//void SessionImp::OnGetMemberCards(XCast<PMemberCards>& cmd, nproto::connection* c)
//{
//	session_member_list_->onMemberCards(cmd.memberCards);
//}

void SessionImp::onMemberCards(XCast<PZMemberCards>& cmd, nproto::connection* c)
{
	session_member_list_->onMemberCards(cmd.memberCards);
}

VoiceQC SessionImp::GetUserChannelQC(const UID_T& uid)
{
	return ((SessionInfo *)getChannelInfo(getSessionList()->innerGetUserPid(uid)))->getVoiceQC();
}

ISessionPluginManager* SessionImp::getSessionPluginManager()
{
	return plugin_manager_;
}

uint32_t SessionImp::GetVoiceQuality()
{
	SessionInfo *info = (SessionInfo *)getChannelInfo(getCur());
	if (info) {
		return info->getVoiceQC();
	}
	return 0;
}

uint8_t SessionImp::GetSegmentInPack() {
	return voice_packer_segments_;
}

double SessionImp::GetDefaultPackTime() {
	return default_packet_duration_;
}


void SessionImp::UpdateSList() 
{
	SItem s;
	s.sid = session_id_;
	s.asid = session_info_->getAliaseId();
	s.nick = session_info_->getName();
	s.owner = session_info_->getCreator();
	s.intro = session_info_->getIntroduce();
	s.users = session_member_list_->getOnlineCount();
	s.slogan = session_info_->getSlogan();
	s.bLimit = session_info_->isLimit();
	s.bPub = session_info_->isPub();
	session_manager_->updateSlist(session_id_, s);
}


void  SessionImp::getImmediateStatus(std::list<VoiceStatisticsData>* status)
{
	if(voice_mgr_){
		voice_mgr_->getImmediateStatus( status);
	}
}

void SessionImp::UpdateLastChannelPasswd(const std::string& chPasswd) {
	last_input_channel_password_ = chPasswd;
}

void  SessionImp::OnGetBackUpProtocolFrontsRes(XCast<PGetBackUpProtocolFrontsRes>& cmd, nproto::connection* c) {
	if (core::Request::ifSuccess(cmd.resCode) == false) {
		return;
	}
	for(size_t i = 0; i < cmd.front_addrs_.size(); ++i) {
		if (i >= 4 ) {
			break;
		}
		conn_manager_->Start(cmd.front_addrs_[i].first.data(), cmd.front_addrs_[i].second);
	}
}

void SessionImp::NotifyLoginRescode(int err)
{

	if (err == 0 || err == 200) {
		//assert(times_[1] > times_[0]);
		//assert(times_[2] > times_[0]);
		//assert(times_[3] > times_[0]);
		//assert(times_[4] > times_[0]);
		//assert(times_[5] > times_[0]);
	}

	if (times_[1] != 0) {
		ReportManager::GetInst()->Report(2, 0, "f=jsession;sid=%d;sm=%d;c=%d;exk=%d;res=%d;main=%d;err=%d",  GetSessionId(),
				(times_[1] - times_[0]), (times_[2] - times_[0]) , (times_[3] - times_[0]), 
				(times_[4] - times_[0]), (times_[5] - times_[0]), err);
		InitReportTimes();
	}
	

	ISessionHandler::Status status;
	switch(err){
			case RES_ENONEXIST:
				status = ISessionHandler::INVALID;
				break;
			case RES_EPERM:
				status = ISessionHandler::JOIN_KICKOFF;;
				break;
			case SS_ETOOMANYUSER:
				status = ISessionHandler::TOOMANYUSR;
				break;
			case AUTH_EUIDPASS:
				status = ISessionHandler::PASSWDERROR;
				break;
			case RES_ENOTENOUGH:
				status = ISessionHandler::NOTENOUGHJIFEN;
				break;
			case PP_ENOTMEMBER:
				status = ISessionHandler::NOTMEMBER;
				break;
			case RES_EDATANOSYNC:
				if (is_onvaild_ == true) {
					NotifySessionStatus(ISessionHandler::RECONNECTED);
					session_handler_->onReEnterSucc(this);
				} else {
					is_onvaild_ = true;
					session_handler_->onValid(this);
				}
				return ;
			default:
				is_onvaild_ = true;
				session_handler_->onValid(this);
				return;
	}
	NotifySessionStatus(status);
}

void SessionImp::NotifySessionStatus(ISessionHandler::Status st)
{
	session_handler_->onStatus(st, this);
}

void SessionImp::ReplaceRoler(uint32_t uid, const SID_T &strSid, ChannelRoler nr)
{
	UserRolers::iterator it = user_rolers_.find(uid);
	RolerPair nrp;
	nrp.channelId = strSid;
	nrp.roler = nr;

	if(it != user_rolers_.end()){
		if(nr == CMANAGER){
			RolerPairVertor vecs;

			for(RolerPairVertor::iterator it2 = it->second.begin(); it2 !=it->second.end(); ++it2){
				RolerPair &rp = *it2;
				if(rp.roler == CMANAGER && rp.channelId != strSid){
					vecs.push_back(rp);
				}
			}
			vecs.push_back(nrp);
			it->second = vecs;
		}else{
			ChannelRoler or = GetRoler(uid, strSid);
			if(or == CMANAGER && nr == MEMBER){
				for(RolerPairVertor::iterator it2 = it->second.begin(); it2 !=it->second.end(); ++it2){
					RolerPair &rp = *it2;
					if(rp.roler == CMANAGER && rp.channelId == strSid){
						it->second.erase(it2);
						break;
					}
				}
				if(it->second.empty()){
					it->second.push_back(nrp);
				}
			}else{
				it->second.clear();
				it->second.push_back(nrp);
			}
		}
	}else{
		user_rolers_[uid].push_back(nrp);
	}
}

void SessionImp::RemoveRoler(const UID_T &uid,  const SID_T &channelId)
{
	UserRolers::iterator it = user_rolers_.find(uid);
	if(it != user_rolers_.end()){
		it->second.clear();
	}
	session_member_list_->onRemoveMember(uid);
}





///////////////////////////////////////////////////////////////
// Old version and will be abandon, so not change too much


void SessionImp::_onSync(PRealJoinChannelRes* pcmd, nproto::connection* c)
{
	PRealJoinChannelRes& cmd = *pcmd;
	session_info_->setSession(this);
	session_info_->setProperty(cmd.sessInfo);
	plugin_manager_->openChange(session_info_->getOpenPlugin());
	session_member_list_->updateMemberCount(session_info_->getMemberCount());
	//order must update before sessInfo set, for hall
	session_member_list_->UpdateOrder(cmd.order);


	for(size_t  xxx=0; xxx<cmd.authes.size(); ++xxx)
	{
		rule_checker_->push_auth(cmd.authes[xxx]);
	}

	for(sox::sid2props_map_t::iterator it = cmd.baseInfo.mapp.begin(); it != cmd.baseInfo.mapp.end(); ++it){
		if (channel_infos_.find(it->first) == channel_infos_.end()) {
			channel_infos_[it->first] = nsox::nu_create_object<SessionInfo>::create_inst();
		}
		if (channel_infos_.find(it->first) != channel_infos_.end()) {
			channel_infos_[it->first]->setSession(this);
			channel_infos_[it->first]->setProperty(it->second);
			channel_infos_[it->first]->onInfoChange();
		}
	}

	user_rolers_.clear();

	for(std::vector<MRolerAndChUser>::iterator it = cmd.rolers.begin(); it != cmd.rolers.end(); ++it)
	{
		RolerPair rp;
		rp.channelId = it->cid;
		rp.roler = it->roler;
		user_rolers_[it->uid].push_back(rp);
	}

	std::vector<SubChannel> cs;

	for(std::vector<uint32_t>::iterator it = cmd.subs.begin(); it != cmd.subs.end(); ++it){
		sox::sid2props_map_t::iterator sit = cmd.baseInfo.mapp.find(*it);
		if(sit != cmd.baseInfo.mapp.end()){
			SubChannel item;
			item.nick = sox::properties::get(sit->second, SIT_NAME);
			item.pid = sox::properties::getint(sit->second, SIT_PID);
			item.sid = sox::properties::getint(sit->second, SIT_SID);
			item.hasPasswd = sox::properties::getint(sit->second, SIT_BHASPASSWD) != 0;
			cs.push_back(item);
		}
	}
	if (is_relogin_) {
		session_member_list_->onSync(cmd.partners, cs, cmd.disablevoice.us, cmd.disabletext.us, cmd.chTextDisabled, true);
		NotifyLoginRescode(RES_EDATANOSYNC);
	}else {
		session_member_list_->onSync(cmd.partners, cs, cmd.disablevoice.us, cmd.disabletext.us, cmd.chTextDisabled, false);
		NotifyLoginRescode(0);
	}
	
	session_info_->setIsSess(true);
	SetAliasId(session_info_->getAliaseId());
	session_member_list_->endSync(is_relogin_);
	session_manager_->updateMyList(session_id_, GetRoler(uid_, 0) >= MEMBER);
	UpdateSList();


	conn_manager_->LoginSuccess(c);
	UpdateMemberList();

}


bool SessionImp::Login(nsox::nu_auto_ptr<FrontConnection> connect) {
	SetTime(3);
	if (port_ > 10000 && !user_token_.empty()) {
		XCast<PJoinSession> join_session ;
		join_session.uid = uid_;
		join_session.sha1Pass = app2net(user_token_);
		join_session.sid	 = session_id_;
		join_session.ssid = currect_channel_;
		join_session.ssPass = last_input_channel_password_;
		join_session.version = client_version_;
		join_session.client_id_ = client_id_;
		conn_manager_->SendByConnection(connect, join_session);
		server_version_ = 200;
	} else if (!user_token_.empty()) {
		XCast<PRealJoinSession> join_session ;
		join_session.uid = uid_;
		join_session.verify_token = app2net(user_token_);
		join_session.verify_style = 2;
		join_session.sid	 = session_id_;
		join_session.ssid = currect_channel_;
		join_session.ssPass = last_input_channel_password_;
		join_session.version	= client_version_;
		conn_manager_->SendByConnection(connect, join_session);
	} else if (port_ < 10000) {
		XCast<PRealJoinChannel> join_session ;
		join_session.uid = uid_;
		join_session.sha1Pass = app2net(user_password_);
		join_session.sid	 = session_id_;
		join_session.ssid = currect_channel_;
		join_session.ssPass = last_input_channel_password_;
		join_session.version	= client_version_;
		conn_manager_->SendByConnection(connect, join_session);
	} else {
		XCast<PJoinSession> join_session ;
		join_session.uid = uid_;
		join_session.sha1Pass = app2net(user_token_);
		join_session.sid	 = session_id_;
		join_session.ssid = currect_channel_;
		join_session.ssPass = last_input_channel_password_;
		join_session.version	= client_version_;
		join_session.client_id_ = client_id_;
		conn_manager_->SendByConnection(connect, join_session);
		server_version_ = 200;
	}
	return true;
}

bool SessionImp::RegisterUdp(nsox::nu_auto_ptr<FrontConnection> c) {
	XCast<PUdpLogin> register_udp;
	register_udp.sid = GetSessionId();
	register_udp.uid = GetUid();
	//Sleep(2);
	conn_manager_->SendByConnection(c, register_udp);
	conn_manager_->SendByConnection(c, register_udp);
	conn_manager_->SendByConnection(c, register_udp);
	return true;
}

bool SessionImp::Relogin() {
	is_relogin_ = true;
	ReSessionLbs();
	return true;
}

bool SessionImp::GetBackUpFronts(int count) {
	if (server_version_ == 200) {
		XCast<PGetBackUpProtocolFronts> req;
		req.number_ = count;
		SendNprotoRequest(req);
	}
	return true;
}

uint32_t SessionImp::GetServerVserion() {
	return server_version_;
}

bool SessionImp::UdpClose(nsox::nu_auto_ptr<FrontConnection> c) {
	return true;
}

CRSAKey* SessionImp::GetRsaKey() {
	return &rsakey_;
}

void SessionImp::ReseqTextSeq() {
	text_sequence_ = 0;
	for(int i = 0; i < MAX_TEXT_SEQ_ROMM; ++i) {
		sequence_cache_[i] = 0;
	}
}

int SessionImp::CheckSeqStatus(uint32_t seq) {
	int index = seq % MAX_TEXT_SEQ_ROMM;
	if (sequence_cache_[index] == seq) {
		return -1;
	}
	sequence_cache_[index] = seq;
	if (seq >= text_sequence_ + 1 || text_sequence_ == 0 || seq == 0) {
		text_sequence_ = seq;
	}
	return 0;
}

void SessionImp::onSeqText(XCast<PSeqChatText>& cmd, nproto::connection* c) {
	if(core::Request::ifSuccess(cmd.resCode) && !IsIgnoreText(cmd.from)) {
		try {
			//if (cmd.sequence_ >= text_sequence_ + 1 || text_sequence_ == 0 || cmd.sequence_ == 0) {
 			text_sequence_ = cmd.sequence_;
			if (CheckSeqStatus(cmd.sequence_) >= 0) {
				if (cmd.from != uid_) {
					XConnPoint<IChatWatch>::forEachWatcher3(&IChatWatch::onText, cmd.from, cmd.sid,
						TextChat::fromString(cmd.chat));
				}
			}
			//} else {
			//	if (text_sequence_ > cmd.sequence_  && text_sequence_ - cmd.sequence_ > 8) {
			//		text_sequence_ = 0;
			//	}
				//assert(0);
			//  }
		} catch (...) {
			assert(0);
		}
	}
}

void SessionImp::onText(XCast<PChatText>& cmd, nproto::connection* c){
	if(core::Request::ifSuccess(cmd.resCode) && !IsIgnoreText(cmd.from)) {
		try {
			XConnPoint<IChatWatch>::forEachWatcher3(&IChatWatch::onText, cmd.from, cmd.sid,
				TextChat::fromString(cmd.chat));
		} catch (...) {
			assert(0);
		}
	}
}

void SessionImp::onBroadcastVoice(XCast<PBroadcastVoice>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode) &&  !IsIgnoreVoice(cmd.from)) {
		voice_mgr_->push(&cmd);
	}
}


void SessionImp::onBroadcastText(XCast<PBroadcastText>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode) && 
		!IsIgnoreText(cmd.from))
	{
		try {
			XConnPoint<IChatWatch>::forEachWatcher3(&IChatWatch::onBroadcasText,
				cmd.from,  
				cmd.sid,
				TextChat::fromString(cmd.chat));
		} catch (...) {
			assert(0);
		}
	}
}

void SessionImp::onVoice(XCast<PChatVoice>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode) &&  !IsIgnoreVoice(cmd.from)) {
		if (getSessionList()->innerGetUserPid(cmd.from) != getSessionList()->innerGetUserPid(uid_)) {
			return;
		}
		voice_mgr_->push(&cmd);
	}
}

void SessionImp::onExpandTypeVoice(XCast<PExpandTypeVoice>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode) &&  !IsIgnoreVoice(cmd.from))
	{
		static int probability = -1;
		if (getSessionList()->innerGetUserPid(cmd.from) != getSessionList()->innerGetUserPid(uid_)) {
			return;
		}
		voice_mgr_->push(&cmd);
	}
}

void SessionImp::onExpandQcVoice(XCast<PExpandQcVoice>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode) &&  !IsIgnoreVoice(cmd.from))
	{
		static int probability = -1;
		if (getSessionList()->innerGetUserPid(cmd.from) != getSessionList()->innerGetUserPid(uid_)) {
			return;
		}
		voice_mgr_->push(&cmd);
	}
}


void SessionImp::onEmptyVoices(XCast<PEmptyChatVoice>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode) &&  !IsIgnoreVoice(cmd.from))
	{
		voice_mgr_->pushEmpty(&cmd);
	}
}


void SessionImp::onReSendVoices(XCast<PReSendVoiceRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode) &&  !IsIgnoreVoice(cmd.from))
	{
		nsox::xlog(NsoxFatal, "recv onReSendVoices, packet info: seq=%d, from=%d", cmd.seq, cmd.from);  //voicedubug
		voice_mgr_->push(&cmd);
	} else {
		nsox::xlog(NsoxDebug, "recv onReSendVoices err");  //voicedubug
	}
}


void SessionImp::onReSendTypeVoices(XCast<PReSendTypeVoiceRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode) &&  !IsIgnoreVoice(cmd.from))
	{
		nsox::xlog(NsoxFatal, "recv onReSendVoices, packet info: seq=%d, from=%d", cmd.seq, cmd.from);  //voicedubug
		voice_mgr_->push(&cmd);
	} else {
		nsox::xlog(NsoxDebug, "recv onReSendVoices err");  //voicedubug
	}
}


void SessionImp::onReSendQcVoices(XCast<PReSendQcVoiceRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode) &&  !IsIgnoreVoice(cmd.from))
	{
		nsox::xlog(NsoxFatal, "recv onReSendVoices, packet info: seq=%d, from=%d", cmd.seq, cmd.from);  //voicedubug
		voice_mgr_->push(&cmd);
	} else {
		nsox::xlog(NsoxDebug, "recv onReSendVoices err");  //voicedubug
	}
}

void SessionImp::onReSendBcVoices(XCast<PReSendBcVoiceRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode) &&  !IsIgnoreVoice(cmd.from))
	{
		nsox::xlog(NsoxFatal, "recv onReSendVoices, packet info: seq=%d, from=%d", cmd.seq, cmd.from);  //voicedubug
		voice_mgr_->push(&cmd);
	} else {
		nsox::xlog(NsoxDebug, "recv onReSendVoices err");  //voicedubug
	}
}

void SessionImp::onUdpConnected(XCast<PUdpLoginRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)) {	
		conn_manager_->UdpRegisterSuccess(c);
	}
}


void SessionImp::onChannelInfoChanged(XCast<POnChannelInfoUpdated>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		//TODO
		ChannelInfo::iterator fit = channel_infos_.find(cmd.sid);
		if(fit == channel_infos_.end()){
			return;
		}
		SessionInfoPtr channel_info = fit->second;
		ChannelStyle old_channel_style = channel_info->getStyle();
		uint32_t old_voice_quilty = channel_info->getVoiceQC();

		if(cmd.props.find(SIT_NAVIGATION) != cmd.props.end()){
			std::string old_navigation = app2net(channel_info->getNavigation());
			std::string new_navigation = sox::properties::get(cmd, SIT_NAVIGATION);
			if( old_navigation != new_navigation ) {
				XConnPoint<ISessionEvent>::forEachWatcher1(&ISessionEvent::onNavigateChange, cmd.sid );				
			}
		}
		channel_info->setProperty(cmd);


		//std::string w = __infos[cmd.sid].props[SIT_NAME];

		if (currect_channel_ == cmd.sid) {
			if(cmd.props.find(SIT_STYLE) != cmd.props.end()){
				if(channel_info->getStyle() != old_channel_style) {
					microphone_list_imp_->OnChangeStyle(currect_channel_, channel_info->getStyle(), 0);
					UID_T op = sox::properties::getint(cmd, SIT_INFO_OPERATOR);
					XConnPoint<ISessionEvent>::forEachWatcher2(&ISessionEvent::onStyleChange, op, false);
				}
			}
			if (cmd.props.find(SIT_VOICEQC) != cmd.props.end()) {
				if (channel_info->getVoiceQC() != old_voice_quilty) {
					voice_mgr_->clear(); // 換音質時清楚所有的数据
				}
			}
		}		
		session_member_list_->infoAffectList(cmd.sid, cmd);
		channel_info->onInfoChange();
	}else{
		XConnPoint<IActionEvent>::forEachWatcher2(&IActionEvent::onResult,
			UPDATE_SINFO,
			cmd.resCode);
	}
}

void SessionImp::onRolerChanged(XCast<POnMemberUpdated>& cmd, nproto::connection* c)
{
	if(Request::ifSuccess(cmd.resCode)){
		//TODO
		if(session_member_list_->isFolder(cmd.channelId) || cmd.channelId == 0){
			SessionInfoPtr channel_info = NULL;
			ChannelRoler oldr = GetRoler(cmd.uid, cmd.channelId);
			ChannelInfo::iterator fit = channel_infos_.find(cmd.channelId);
			for(ChannelInfo::iterator fit = channel_infos_.begin(); fit != channel_infos_.end(); ++fit)
			{
				channel_info = (SessionInfo *)fit->second;
				channel_info->refreshManager(&cmd);
			}

			UserRolers::iterator it = user_rolers_.find(cmd.uid);
			RolerPairVertor tmp;
			if (it != user_rolers_.end()) {
				tmp = it->second;
			}
			//在線維護身份
			if(true || session_member_list_->isOnline(cmd.uid)) { // 新版本已经知道所有人的身份
				ChannelRoler r = GetChannelRoler(cmd.uid, cmd.channelId);
				RolerPair rp;
				rp.channelId = cmd.channelId;
				rp.roler = cmd.roler;
				switch(cmd.op){
						case POnMemberUpdated::ADD:{
							if(cmd.roler != NORMAL){
								ReplaceRoler(cmd.uid, cmd.channelId, rp.roler);
							}else
								assert(false);
												   }
												   break;
						case POnMemberUpdated::REMOVE:
							if(NORMAL == cmd.roler)
								RemoveRoler(cmd.uid,  cmd.channelId);
							else
								assert(false);
							break;
						case POnMemberUpdated::CHANGE:
							ReplaceRoler(cmd.uid, cmd.channelId, rp.roler);
							break;
				}
				NotifyMemberChange(cmd.uid);
			}			

			if(channel_info){
				channel_info->onInfoChange();
			}

			//更新會員列表
			if (cmd.roler >= MEMBER && (cmd.op == POnMemberUpdated::ADD
				|| cmd.op == POnMemberUpdated::CHANGE)) {
							session_member_list_->addMember(cmd.uid,  net2app(cmd.nick), _T(""), 
									cmd.sex, 0, 0,  cmd.roler);
			} else {
				session_member_list_->removeMember(cmd.uid);
			}
			ChannelRoler newr = GetRoler(cmd.uid, cmd.channelId);
			uint32_t cid = cmd.channelId;
			if (cmd.channelId == 0 && newr == oldr && newr== MEMBER) {
				if (tmp.size() == 1) {
					cid = tmp[0].channelId;
					oldr = tmp[0].roler;
				}
			}
			XConnPoint<ISessionEvent>::forEachWatcher5(&ISessionEvent::onRolerChange, cmd.admin, cmd.uid, cid, newr, oldr);
		}else{
			assert(false);	
		}
	}else{
		//更新失敗
		ChannelInfo::iterator fit = channel_infos_.find(cmd.channelId);
		if(fit != channel_infos_.end()){
			SessionInfoPtr infoImp = fit->second;
			infoImp->updateManagerErr(cmd.resCode);
		}
		XConnPoint<IActionEvent>::forEachWatcher2(&IActionEvent::onResult,
			UPDATE_MEMBERROLER,
			cmd.resCode);
	}
}

void SessionImp::onRingBusy(XCast<PRingBusy>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		XConnPoint<ISessionEvent>::forEachWatcher0(&ISessionEvent::onRingBusy);
	}
}


void SessionImp::onSetKeyActive(XCast<PSetKeyActive>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		XConnPoint<ISessionEvent>::forEachWatcher1(&ISessionEvent::onSetKeyActive, 
			cmd.target);
	}else{
		XConnPoint<IActionEvent>::forEachWatcher2(&IActionEvent::onResult,
			SETKEYACTIVE,
			cmd.resCode);
	}
}

void SessionImp::onSetRoomKeyActive(XCast<PSetRoomKeyActive>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		if(currect_channel_ == cmd.sid){
			XConnPoint<ISessionEvent>::forEachWatcher1(&ISessionEvent::onSetRoomKeyActive, 
				cmd.admin);
		}else{
			nsox::xlog(NsoxWarn, "set room key active sid: %d， but current sid:", cmd.sid, currect_channel_);
		}
	}else{
		XConnPoint<IActionEvent>::forEachWatcher2(&IActionEvent::onResult,
			SETKEYACTIVE,
			cmd.resCode);
	}
}

void SessionImp::onKickOff(XCast<POnSessKickOff>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){

		std::vector<ITreeNode> mpids;
		std::vector<ITreeNode> ypids;

		session_member_list_->getParent(ITreeNode(uid_, ITreeNode::E_USER), mpids);
		session_member_list_->getParent(ITreeNode(cmd.uid, ITreeNode::E_USER), ypids);


		ChannelInfoKickOffAdded(cmd.uid, cmd.toCh);
		XConnPoint<ISessionEvent>::forEachWatcher6(&ISessionEvent::onKickOff, 
			cmd.uid,
			ypids.size() == 0 ? 0 : ypids.front().value,
			cmd.toCh,
			cmd.admin,
			net2app(cmd.reason), 
			cmd.secs / 60);

		if(cmd.uid == GetUid() && cmd.toCh == 0){
			session_handler_->onClose(ISessionHandler::KICKOFF, this, 0);
		}

		if(cmd.toCh == 0){
			//TODO
			if(cmd.uid == session_manager_->uid){
				last_error_ = net2app(cmd.reason);
				return;
			}
		}
	}else{
		XConnPoint<IActionEvent>::forEachWatcher2(&IActionEvent::onResult,
			KICKOFF,
			cmd.resCode);
	}
}


void SessionImp::onSyncManager(XCast<POnSyncManagers>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		//IChannelInfo *info = (IChannelInfo *)getChannelInfo(cmd.cid);
		for(std::vector<PRolerMini>::iterator it = cmd.managers.begin(); it != cmd.managers.end(); ++it){
			RolerPair rp;
			rp.channelId = cmd.cid;
			rp.roler = it->role;

			if(user_rolers_.find(it->uid) == user_rolers_.end()){
				user_rolers_[it->uid].push_back(rp);
			}else{
				RolerPairVertor &rs = user_rolers_[it->uid];
				RolerPairVertor::iterator ir = rs.begin();
				for(; ir != rs.end(); ir++){
					if(ir->channelId == cmd.cid){
						break;	
					}
				}

				if(ir == rs.end()){
					rs.push_back(rp);
				}
			}
		}

		ChannelInfo::const_iterator iit = channel_infos_.find(cmd.cid);
		if(iit != channel_infos_.end()){
			iit->second->onGetManagers(&cmd);
		}
	}
}

void SessionImp::onStop(XCast<PStopVoice>& cmd, nproto::connection* c)
{
	voice_mgr_->stop(cmd.uid);// 该用户停止语音
	XConnPoint<IChatWatch>::forEachWatcher1(&IChatWatch::onStopVoice, cmd.uid);
}

void SessionImp::onGetBans(XCast<POnGetBans>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		//ChannelInfo::iterator fit = channel_infos_.find(GetSessionId());
		//if(fit != channel_infos_.end()){
		session_info_->updateBans(cmd.ids, cmd.ips);
	}
}


void SessionImp::onBanUser(XCast<PUpdateBanIDRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		session_info_->banUser(cmd.uid, cmd.isBan, net2app(cmd.account));
	}
}

void SessionImp::onBanIp(XCast<PUpdateBanIpRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		session_info_->banIp(cmd.ip, cmd.isBan);
	}
}

void SessionImp::onBatchBanUser(XCast<PBatchUpdateBanIDRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		session_info_->batchBanUser(cmd.uids, cmd.isBan);
	}
}

void SessionImp::onBatchBanIp(XCast<PBatchUpdateBanIpRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		session_info_->batchBanIp(cmd.ips, cmd.isBan);
	}
}

void SessionImp::onNotifySession(XCast<PNotifySession> &cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){

		XConnPoint<ISessionEvent>::forEachWatcher2(&ISessionEvent::onNotifySession,
			(ISessionEvent::AnnounceType)cmd.type,
			net2app(cmd.msg));

	}
} 

void SessionImp::onDisableVoice(XCast<PDisableVoice> &cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		session_member_list_->setVoiceAble(cmd.uid, !cmd.disable);

		std::vector<ITreeNode> parents;
		session_member_list_->getParent(ITreeNode(cmd.uid, ITreeNode::E_USER), parents);
		XConnPoint<ISessionEvent>::forEachWatcher4(&ISessionEvent::onDisableVoice,
			parents.size() == 0 ? 0 : parents.front().value,
			cmd.admin,
			cmd.uid,
			cmd.disable);
	}
}

void SessionImp::onDisableText(XCast<PDisableText> &cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{

		std::vector<ITreeNode> parents;
		session_member_list_->getParent(ITreeNode(cmd.uid, ITreeNode::E_USER), parents);

		session_member_list_->setTextAble(cmd.uid,!cmd.disable);

		XConnPoint<ISessionEvent>::forEachWatcher4(&ISessionEvent::onDisableText,
			parents.size() == 0 ? 0 : parents.front().value,
			cmd.admin,
			cmd.uid,
			cmd.disable);
	}
}

void SessionImp::onMutedSelf(XCast<PMutedSelfRes> &cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)) {
		session_member_list_->setSelfMuted(cmd.muted);
	}
}


void SessionImp::onStatusChanged(XCast<PStatusChanged>& cmd, nproto::connection* c)
{
	session_member_list_->onStatusChange(cmd.uid, net2app(cmd.status), cmd.resCode);
}

void SessionImp::onSyncUStatus(XCast<PSyncUStatusRes>& cmd, nproto::connection* c)
{
	std::list<PSyncUStatusRes::userstatus>::iterator it = cmd.allstatus.begin();
	std::list<PSyncUStatusRes::userstatus>::iterator itEnd = cmd.allstatus.end();
	for (; it != itEnd; it++) {
		session_member_list_->onStatusChange(it->first, net2app(it->second), cmd.resCode);
	}	
}


void SessionImp::onNickChange(XCast<PPInfoChanged>& cmd, nproto::connection* c)
{
	//session_member_list_->onPInfoChange(&cmd, cmd.resCode);
	session_member_list_->UpdateUserInfo(cmd.uid);
}

void SessionImp::onChangeFolder(XCast<PChangeFolder>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		session_member_list_->onChangeFolder(cmd, c);
	}else{
		//TODO: xxxx 由於密碼原因,留給服務器處理
		ResetVoiceManager();
		XConnPoint<IActionEvent>::forEachWatcher2(&IActionEvent::onResult,
			CHANGE_FOLDER,
			cmd.resCode);
	}
}

void SessionImp::onResendVoice(XCast<PReSendVoice>& cmd, nproto::connection* c)
{
	int seq = cmd.seq;
	protocol::session::PNormalVoiceData *voice_before_cast = output_voice_queue_.Get(seq);
    if (voice_before_cast) {
        nsox::xlog(NsoxInfo,"resend voice get, seq %d  size %d ",voice_before_cast->seq, voice_before_cast->data.size());
		nproto::write_request(c, voice_before_cast, voice_before_cast->URI);
    }
}

void SessionImp::OnVoiceRes(XCast<POnVoiceRes>& cmd, nproto::connection* c) {
	
	voice_res_.insert(cmd.seq_);
	
	int max = 0;
	int min = 0;
	for(std::set<uint32_t>::iterator it = voice_res_.begin(); it != voice_res_.end(); ++it) {
		if ((*it) > max || max == 0)
			max = *it;
		if ((*it) < min || min == 0)
			min = *it;
	}
	if (voice_res_.size() >= 100 || (1 + ((max - min) / 2)) > 100 || voice_no_res_ > 200) {
		if (max - min < 500) {
			double should = 1 + ((max - min) / 2);	
			double loss_ret = (should - voice_res_.size()) / should;
			//int loss =  1 + ((max - min) / 2) - voice_res_.size();
			//double loss_ret = loss % voice_res_.size();
			ReportManager::GetInst()->Report(5, 0, "f=ds;sid=%d;ls=%d", GetSessionId(), (int)(loss_ret*100));
			if (loss_ret > 0.03/* || true*/) {
					conn_manager_->ChangeFirstNextTime();
			} else {

			}
		}
		voice_res_.clear();
		voice_no_res_ = 0;
	}
}


void SessionImp::onSynOrder(XCast<PSynOrder>& cmd, nproto::connection* c)
{
	if(!core::Request::ifSuccess(cmd.resCode)){
		nsox::xlog(NsoxInfo, "sync res code: %d", cmd.resCode);
		return ;
	}
	session_member_list_->UpdateOrder(cmd.order);
}

void SessionImp::onTcpConnect(XCast<PTcpConnect>& cmd, nproto::connection* c)
{
	//TODO
	//StopUdp();
	nsox::xlog(NsoxDebug, "PTcpConnect : close udp");
}


void SessionImp::onUpdateKickOffList(XCast<PUpdateKickOffRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		if (cmd.cid != 0) {
			ChannelInfo::iterator fit = channel_infos_.find(cmd.cid);
			if(fit != channel_infos_.end()){
				fit->second->updateKickOffs(cmd.cid, cmd.kuserinfos, cmd.kicks);
				return;
			}
		}
		session_info_->updateKickOffs(cmd.cid, cmd.kuserinfos, cmd.kicks);
	}
}

void SessionImp::onMemberText(XCast<PMemberText>& cmd, nproto::connection* c)
{
	if (core::Request::ifSuccess(cmd.resCode)) {
		sox::Varstr chat(cmd.chat.c_str(),cmd.chat.size());
		try {
			XConnPoint<IMemberChatWatch>::forEachWatcher3(&IMemberChatWatch::onMemberText,
				cmd.from,
				cmd.time,
				TextChat::fromString(chat));
		} catch (...) {
		}
	}
}

void SessionImp::onMemberHistory(XCast<PZMemberHistory>& cmd, nproto::connection* c)
{
	/*
	for(int count = cmd.historys.size() - 1 ; count >= 0 ; --count) 
	{
		sox::Varstr chat(cmd.historys[count].chat.c_str(),cmd.historys[count].chat.size());
		XConnPoint<IMemberChatWatch>::forEachWatcher3(&IMemberChatWatch::onMemberText,
			cmd.historys[count].from,
			cmd.historys[count].time,
			TextChat::fromString(chat));
		nsox::xlog(NsoxDebug, "member IMemberChatWatch, size:%d", 	XConnPoint<IMemberChatWatch>::handlers.size());
	}
	*/
	XConnPoint<IMemberChatWatch>::forEachWatcher1(&IMemberChatWatch::onBatchMemberText, cmd.historys);
}


void SessionImp::onMemberList(XCast<PZMemberList>& cmd, nproto::connection* c)
{
	session_member_list_->onSyncMembers(cmd.members);
	//__bGetMemberList = true;
	nsox::xlog(NsoxDebug, "sync member lists, size:%d", cmd.members.size());
}

void  SessionImp::onNotice(XCast<PNotice>& cmd, nproto::connection* c)
{
	if (core::Request::ifSuccess(cmd.resCode)) {
		XConnPoint<INoticeWatch>::forEachWatcher3(&INoticeWatch::onNotice,
			cmd.type,
			cmd.resources,
			net2app(cmd.info));
	}
}


void SessionImp::onRemoveKickOff(XCast<PRemoveKickOffRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		if (cmd.cid != 0) {
			ChannelInfo::iterator fit = channel_infos_.find(cmd.cid);
			if(fit != channel_infos_.end()){
				fit->second->removeKickOff(cmd.uid, cmd.cid);
			}
		}
		session_info_->removeKickOff(cmd.uid, cmd.cid);
	}
}

void SessionImp::onSync(XCast<PRealJoinChannelRes>& cmd, nproto::connection* c)
{
	if(!core::Request::ifSuccess(cmd.resCode)){
		nsox::xlog(NsoxInfo, "sync res code: %d", cmd.resCode);
		NotifyLoginRescode(cmd.resCode);
		return ;
	}
	_onSync(&cmd, c);
}

void SessionImp::onZSync(XCast<PZRealJoinChannelRes>& cmd, nproto::connection* c)
{
	SetTime(4);
	SetTime(5);
	nsox::xlog(NsoxInfo, "join channel anwsered with res : %d .....................(9 zip)", cmd.resCode);

	if(!core::Request::ifSuccess(cmd.resCode)){
		nsox::xlog(NsoxInfo, "sync res code: %d", cmd.resCode);
		NotifyLoginRescode(cmd.resCode);
		return ;
	}
	_onSync(&cmd, c);
}




void SessionImp::onBroadCastEvent(XCast<PBroadcastEvent>& cmd, nproto::connection* c)
{
	XConnPoint<IChatWatch>::forEachWatcher4(&IChatWatch::onEvent, cmd.from, cmd.sid, cmd.code, net2app(cmd.info));
}

void  SessionImp::OnJoinSesionRes(XCast<PJoinSessionRes>& cmd, nproto::connection* c) {
	SetTime(4);
	if(!core::Request::ifSuccess(cmd.resCode)){
		nsox::xlog(NsoxInfo, "sync res code: %d", cmd.resCode);
		NotifyLoginRescode(cmd.resCode);
		return ;
	}
	bool is_first_success = (conn_manager_->HasTcpLoginReady() == false);
	conn_manager_->LoginSuccess(c);
	if (is_first_success) {


		

		XCast<PGetSessionMain> cmd1;
		
		XCast<PGetSessionAuth> cmd2;
		XCast<PGetDisableVoice> cmd3;
		XCast<PGetDisableText> cmd4;
		XCast<PGetUserRoles> cmd5;
		XCast<PGetSeqList> cmd6;
		
		XCast<PGetMemberCards> cmd8;

		XCast<PUserSubcribeMessage> cmd7;
		cmd7.message_type_ = PUserSubcribeMessage::ALL_MESSAGE;

		//DirectSendRequest(c, cmd1);
		//DirectSendRequest(c, cmd2);
		//DirectSendRequest(c, cmd3);
		//DirectSendRequest(c, cmd4);
		//DirectSendRequest(c, cmd5);
		//DirectSendRequest(c, cmd6);
		//DirectSendRequest(c, cmd7);

		conn_manager_->SetConntionMessage(c, PUserSubcribeMessage::ALL_MESSAGE);
		nproto::write_request(c, &cmd7, PUserSubcribeMessage::uri);


		XCast<PGetVoiceToken> cmd_0;
		conn_manager_->Send(cmd_0);


		conn_manager_->Send(cmd1);

		GetChannelExtendInfo(0);
	


		conn_manager_->Send(cmd2);
		conn_manager_->Send(cmd3);
		conn_manager_->Send(cmd4);
		conn_manager_->Send(cmd5);
		conn_manager_->Send(cmd6);
		conn_manager_->Send(cmd8);

		XCast<protocol::session::PTcpPing> plus;
		plus.stampc = ::timeGetTime();
		plus.uid = GetUid();
		plus.sid = GetSessionId();
		conn_manager_->Send(plus);

	} else {
		XCast<PUserSubcribeMessage> cmd7;
		cmd7.message_type_ = PUserSubcribeMessage::SEQUENCE_MESSAGE;
		conn_manager_->SetConntionMessage(c, PUserSubcribeMessage::SEQUENCE_MESSAGE);
		nproto::write_request(c, &cmd7,PUserSubcribeMessage::uri);
	}
}

void  SessionImp::OnGetSessionMainRes(XCast<PGetSessionMainRes>& cmd, nproto::connection* c) {
	SetTime(5);
	session_info_->setProperty(cmd.session_info);
	session_info_->setIsSess(true);

	// update other info relational to session info
	SetAliasId(session_info_->getAliaseId());
	
	plugin_manager_->openChange(session_info_->getOpenPlugin());
	session_member_list_->updateMemberCount(session_info_->getMemberCount());
	session_member_list_->UpdateOrder(cmd.orders);

	//
	std::vector<SubChannel> channel_tree_infos;
	for(sox::sid2props_map_t::iterator it = cmd.channel_info.mapp.begin(); it != cmd.channel_info.mapp.end(); ++it){
		//sox::properties::replace(__infos[it->first], it->second);
		ChannelInfo::iterator it_channel =  channel_infos_.find(it->first);
		SessionInfoPtr channel_info = NULL;
		if (it_channel == channel_infos_.end()) {
			channel_info = nsox::nu_create_object<SessionInfo>::create_inst();
			channel_infos_[it->first] = channel_info;
		} else {
			channel_info = it_channel->second;
		}

		if (channel_info != NULL) {
			channel_info->setSession(this);
			channel_info->setProperty(it->second);
			channel_info->onInfoChange();
		} else {
			assert(0);
		}

		SubChannel item;
		item.nick = sox::properties::get(it->second, SIT_NAME);
		item.pid = sox::properties::getint(it->second, SIT_PID);
		item.sid = sox::properties::getint(it->second, SIT_SID);
		item.hasPasswd = sox::properties::getint(it->second, SIT_BHASPASSWD) != 0;
		channel_tree_infos.push_back(item);
	}
	session_member_list_->UpdateChannelTreeInfo(channel_tree_infos);
	session_member_list_->OnInitUserChannel(cmd.user_channel);

	if (is_relogin_) {
		NotifyLoginRescode(RES_EDATANOSYNC);
	} else {
		NotifyLoginRescode(0);
		
	}
	session_member_list_->endSync(is_relogin_);
	GetBackUpFronts(1);
	//conn_manager_->SetSequence(cmd.currect_seq_);
	UpdateSList();
}

void  SessionImp::OnGetSessionAuthRes(XCast<PGetSessionAuthRes>& cmd, nproto::connection* c) {
	for(size_t i=0; i < cmd.auth_codes.size(); ++i) {
		rule_checker_->push_auth(cmd.auth_codes[i]);
	}
}

void SessionImp::OnGetDisableVoiceRes(XCast<PGetDisableVoiceRes>& cmd, nproto::connection* c) {
	session_member_list_->UpdateDisableVoice(cmd.disable_voices.us);
}

void SessionImp::OnGetDisableTextRes(XCast<PGetDisableTextRes>& cmd, nproto::connection* c) {
	session_member_list_->UpdateDisableText(cmd.disable_texts.us);
}

void SessionImp::OnGetUserRolesRes(XCast<PGetUserRolesRes>& cmd, nproto::connection* c) {
	user_rolers_.clear();
	for(std::vector<MRolerAndChUser>::iterator it = cmd.user_roles.begin(); it != cmd.user_roles.end(); ++it) {
		if (it->uid == 0) {
			continue;
		}
		RolerPair rp;
		rp.channelId = it->cid;
		rp.roler = it->roler;
		user_rolers_[it->uid].push_back(rp);
	}
	session_manager_->updateMyList(session_id_, GetRoler(uid_, 0) >= MEMBER);
	session_member_list_->RefreshMemberList();



	XCast<PGetSessionMemberCredit> getmc_cmd;
	getmc_cmd.sid_ = session_id_;
	conn_manager_->Send(getmc_cmd);
}

void SessionImp::OnGetUserInfoRes(XCast<PGetUserInfoRes>& cmd, nproto::connection* c) {
	session_member_list_->UpdateUserInfos(cmd.user_infos_);
}

void  SessionImp::OnGetSeqListRes(XCast<PGetSeqListRes>& cmd, nproto::connection* c) {
	conn_manager_->SetSeqUris(cmd.uris_);
}

void SessionImp::on_packet_loss(const UID_T& user, int seq) {
	OnPacketLoss(user, seq);
}

void SessionImp::OnPacketLossChangeBad() {
	if (conn_manager_->ChangeMain()) {
		//XCast<PServerStatusCheck> server_status_check;
		//SendNprotoRequest(server_status_check);
	}
}

void SessionImp::OnPacketLossTooMuch() {
	if (conn_manager_->ChangeMain()) {
		//XCast<PServerStatusCheck> server_status_check;
		//SendNprotoRequest(server_status_check);
	}
	
}


bool SessionImp::ReSessionLbs() {
	if(!session_manager_->reconnect(true)){
		session_handler_->onClose(ISessionHandler::INVALID, this, WSAGetLastError());
	} else {
		NotifySessionStatus(ISessionHandler::RECONNECTING);
	}
	return true;
}

void SessionImp::UpdateMemberList()
{
		XCast<PGetMemberList> cmd;
		conn_manager_->Send(cmd);
}

void SessionImp::SetRtt(uint32_t rtt) {
	voice_mgr_->SetRTT(rtt);
	voice_mgr_->SetSid(this->GetSessionId());
}


void SessionImp::InitReportTimes() {
	for(int i = 0; i<JSE_TIME; ++i) {
		times_[i] = 0;
	}
	times_[0] = ::timeGetTime();
	voice_mgr_->initJitter();
}

void SessionImp::SetTime(int index) {
	if (JSE_TIME > index && index > 0) {
		times_[index] = ::timeGetTime();
	}
}

/*
void SessionImp::OnGetExtendChannelInfoRes(XCast<PGetExtendChannelInfoRes>& res, nproto::connection* c) {
	if (res.info_.props.find(SIT_COMPRESS_BULLETIN) != res.info_.props.end()) {
		compresser::uncompress(res.info_.props[SIT_BULLETIN], res.info_.props[SIT_COMPRESS_BULLETIN]);
		if (res.info_.props[SIT_BULLETIN].empty()) {
		}
		UpdateChannelInfo(res.channel_id_, res.info_);
	}
	
}
*/

void SessionImp::GetChannelExtendInfo(uint32_t channel_id) {
	if (channel_id) {
		ChannelInfo::const_iterator it = channel_infos_.find(channel_id);
		if(it != channel_infos_.end()){
			if (it->second->getBulletin().empty() == false) {
				return;
			}
			if (it->second->GetExtend() == true) {
				return;
			}
			it->second->SetExtend(true);
		} else {
			// no channel info not need update
			return;
		}
	}

	XCast<PGetExtendChannelInfo> cmd_extend_channelinfo;
	cmd_extend_channelinfo.channel_id_ = channel_id;
	conn_manager_->Send(cmd_extend_channelinfo);
}


void SessionImp::UpdateVoiceToken() {
	XCast<PGetVoiceToken> cmd_0;
	conn_manager_->Send(cmd_0);
}

