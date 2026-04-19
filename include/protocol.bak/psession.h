#ifndef PCOMMON_SESSOIN_H
#define PCOMMON_SESSOIN_H
#include "common/core/base_svid.h"
#include "common/varstr.h"
#include "protocol/psessionbase.h"
#include "protocol/pzipbase.h"
#include "protocol/psinfo.h"
#include "protocol/psmemberlist.h"
#include "protocol/uuid.h"
#include "protocol/plogin.h"
#include "protocol/psmanager.h"

#include <vector>
#include <iterator>
#include <set>
#include <map>

namespace protocol {
namespace session {


struct Partner : public sox::Marshallable {
	uint32_t uid;
	uint32_t pid;
	uint32_t user_jifen;
	uint32_t usess_jifen;
	uinfo::SEX gender;
	std::string nick;
	std::string sign;
	std::string client_info;

	Partner() : user_jifen(0), usess_jifen(0) {
	}
	
	Partner(uint32_t u, const std::string &n, const std::string &s, uinfo::SEX g, 
		uint32_t p, uint32_t  uj, uint32_t sj, const std::string& cli) : uid(u), 
		pid(p),  user_jifen(uj), usess_jifen(sj), gender(g), nick(n), sign(s), client_info(cli) {
	}

	virtual void marshal(sox::Pack &p) const {
		p << uid << nick << sign << pid << user_jifen << usess_jifen << client_info;
		p.push_uint8(gender);
	}
	
	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid >> nick >> sign >> pid >> user_jifen >> usess_jifen >> client_info;
		gender = (uinfo::SEX)(p.pop_uint8());
	}
};


struct RolerPartner : public Partner{
	uint32_t channel;
	ChannelRoler roler;
	virtual void marshal(sox::Pack &p) const {
		Partner::marshal(p);
		p.push_uint32(channel).push_uint16(roler);
	}
	virtual void unmarshal(const sox::Unpack &p) {
		Partner::unmarshal(p);
		channel = p.pop_uint32();
		roler = (ChannelRoler)p.pop_uint16();
	}
};	

struct SubChannel {
	uint32_t pid;
	std::string nick;
	uint32_t sid;
	bool hasPasswd;
};


struct AuthCode: public sox::Marshallable{
		uint32_t sess_from_role;
		uint32_t sess_to_role;
		uint32_t auth_code;
		virtual void marshal(sox::Pack & p) const {
				p << sess_from_role << sess_to_role << auth_code;
		}

		virtual void unmarshal(const sox::Unpack &p) {
				p >> sess_from_role >>  sess_to_role >> auth_code;
		}
};


struct PRealJoinChannel : public sox::Marshallable {
	enum {uri = (32 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	uint32_t sid;
	uint32_t ssid;
	uint32_t version;
	std::string sha1Pass;
	std::string ssPass;
	
	virtual void marshal(sox::Pack &p) const {
		p << uid << sha1Pass << sid << ssid << ssPass << version;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid >> sha1Pass >> sid >> ssid >> ssPass >> version;
	}
};

struct PRealJoinChannelRes : public sox::Marshallable {
        enum {uri = (32 << 8 | protocol::SESSION_SVID)};
		sox::Properties	 sessInfo;
        SidMapProperties baseInfo;
        DisableVoiceMap disablevoice;
        DisableTextMap disabletext;
        std::vector<Partner> partners;
        std::vector<uint32_t> subs;
        std::vector<MRolerAndChUser> rolers;
        std::vector<AuthCode> authes;
        std::vector<uint32_t> chTextDisabled;
		std::vector<uint32_t> order;
        virtual void marshal(sox::Pack &p) const {
				p << sessInfo;
                p << baseInfo;
                p << disablevoice;
                p << disabletext;
                sox::marshal_container(p, partners);
                sox::marshal_container(p, subs);
                sox::marshal_container(p, rolers);
                sox::marshal_container(p, authes);
                sox::marshal_container(p, chTextDisabled);
				sox::marshal_container(p, order);
        }
        virtual void unmarshal(const sox::Unpack &p) {
				p >> sessInfo;
                p >> baseInfo;
                p >> disablevoice;
                p >> disabletext;
                sox::unmarshal_container(p, std::back_inserter(partners));
                sox::unmarshal_container(p, std::back_inserter(subs));
                sox::unmarshal_container(p, std::back_inserter(rolers));
                sox::unmarshal_container(p, std::back_inserter(authes));
                sox::unmarshal_container(p, std::back_inserter(chTextDisabled));
				sox::unmarshal_container(p, std::back_inserter(order));
        }
};

struct PCreateSubChannel : public PCreateChannel {
	enum {uri = (33 << 8 | protocol::SESSION_SVID)};
	uint32_t preChannel;
	PCreateSubChannel() {
	}
	PCreateSubChannel(const std::string &n, bool l, bool p,
			const std::string &h, const std::string &pwd, uint32_t parent, VoiceQC q, uint32_t pre = 0) :
		PCreateChannel(n, l, p, h, ISP_US, pwd, parent), preChannel(pre) {
	}

	virtual void marshal(sox::Pack &p) const {
		PCreateChannel::marshal(p);
		p << preChannel;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		PCreateChannel::unmarshal(p);
		p >> preChannel;
	}
};

struct POnSubChannelAdd : public sox::Marshallable {
	enum {uri = (33 << 8 | protocol::SESSION_SVID)};
	uint32_t sid;
	uint32_t preChannel;
	PSessionInfo info;

	virtual void marshal(sox::Pack &p) const {
		p << sid << info << preChannel;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> sid >> info >> preChannel;
	}
};

//create sub channel as a list add;

struct PDismissSubChannel : public PDismissChannel {
	enum {uri = (34 << 8 | protocol::SESSION_SVID)};
};

struct POnSubChannelRemove : public PSessionSid {
	enum {uri = (34 << 8 | protocol::SESSION_SVID)};
};
//dismiss sub channel as a list remove;

struct PLeaveSession : public PSessionSid {
	enum {uri = (35 << 8 | protocol::SESSION_SVID)};
};

struct PChatText : public sox::Marshallable {
	enum {uri = (36 << 8 | protocol::SESSION_SVID)};
	uint32_t from;
	uint32_t sid;
#ifdef WIN32_CLIETN_PROTO
	sox::Varstr	chat;
#else
	std::string chat;
#endif


	virtual void marshal(sox::Pack &pak) const {
		pak << from << sid << chat;
	}
	virtual void unmarshal(const sox::Unpack &pak) {
		pak >> from >> sid >> chat;
	}
};

struct PSeqChatText: public PChatText{
	enum {uri = (37 << 8 | protocol::SESSION_SVID)};
    uint32_t sequence_;

	virtual void marshal(sox::Pack &pk) const {
        PChatText::marshal(pk);
        pk << sequence_;
	}
	virtual void unmarshal(const sox::Unpack &up) {
        PChatText::unmarshal(up);
        up >> sequence_;
	}
};







/*struct PChatVoices : public sox::Marshallable{
        enum {uri = (39 << 8 | protocol::SESSION_SVID)};
        uint32_t from;
        uint32_t sid;
			std::vector<PVoice> voices;

        virtual void marshal(sox::Pack &pak) const {
                pak << from << sid ;
				sox::marshal_container(pak, voices);
        }
        virtual void unmarshal(const sox::Unpack &pak) {
                pak >> from >> sid;
				sox::unmarshal_container(pak, std::back_inserter(voices));
        }
};*/ 



struct PUdpLogin : public sox::Marshallable {
	enum {uri = (40 << 8 | protocol::SESSION_SVID)};
	uint32_t sid;
	uint32_t uid;
	std::string pass;
	virtual void marshal(sox::Pack &pak) const {
		pak << uid << pass << sid;
	}
	virtual void unmarshal(const sox::Unpack &pak) {
		pak >> uid >> pass >> sid;
	}
	;
};

struct PUdpLoginRes : public sox::Voidmable {
	enum {uri = (40 << 8 | protocol::SESSION_SVID)};
};

struct PUdpPing : public sox::Marshallable {
	enum {uri = (41 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	uint32_t sid;
	uint32_t stampc;

	virtual void marshal(sox::Pack &pak) const {
		pak << uid << sid << stampc; 
	}
	virtual void unmarshal(const sox::Unpack &pak) {
		pak >> uid >> sid >> stampc;
	}
};

struct PChangeStyle : public sox::Marshallable {
	enum {uri = (42 << 8 | protocol::SESSION_SVID)};
	ChannelStyle style;
	uint32_t sid;
	uint32_t who;
	uint32_t microtime;

	virtual void marshal(sox::Pack &pak) const {
		pak << sid << who << microtime;
		pak.push_uint8(style);
	}
	virtual void unmarshal(const sox::Unpack &pak) {
		pak >> sid >> who >> microtime;
		style = (ChannelStyle)pak.pop_uint8();
	}
};


struct PChangeStyleRes : public PChangeStyle {

};

struct PSessKickOff : public POnMemberRemove {
	enum {uri = (43 << 8) | SESSION_SVID};
	std::string reason;
	uint32_t secs;
	virtual void marshal(sox::Pack &p) const {
		POnMemberRemove::marshal(p);
		p << reason << secs;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		POnMemberRemove::unmarshal(p);
		p >> reason >> secs;
	}
};

struct POnSessKickOff : public PSessKickOff {
	enum {uri = (43 << 8) | SESSION_SVID};
	uint32_t admin;
	uint32_t toCh;
	virtual void marshal(sox::Pack &p) const {
		PSessKickOff::marshal(p);
		p << admin << toCh;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		PSessKickOff::unmarshal(p);
		p >> admin >> toCh;
	}
};

struct PTuoRen : public sox::Marshallable {
	enum {uri = (44 << 8) | SESSION_SVID};
	uint32_t uid;
	uint32_t from;
	uint32_t to;

	virtual void marshal(sox::Pack &p) const {
		p << uid << from << to;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid >> from >> to;
	}
};


struct POnTuoren: public PTuoRen{
	enum {uri = (44 << 8) | SESSION_SVID};
	uint32_t admin;
	virtual void marshal(sox::Pack &pak) const {
		PTuoRen::marshal(pak);
		pak << admin;
	}
	virtual void unmarshal(const sox::Unpack &pak) {
		PTuoRen::unmarshal(pak);
		pak >> admin;
	}
};

struct PRingBusy : public sox::Voidmable {
	enum {uri = (45 << 8 | protocol::SESSION_SVID)};
};

struct PSetKeyActive : public sox::Marshallable {
	enum {uri = (46 << 8 | protocol::SESSION_SVID)};
	uint32_t target;
	uint32_t sid;
	virtual void marshal(sox::Pack &pak) const {
		pak << target << sid;
	}
	virtual void unmarshal(const sox::Unpack &pak) {
		pak >> target >> sid;
	}
};

struct PNotifySession : public sox::Marshallable {
	enum {uri = (47 << 8 | protocol::SESSION_SVID)};
	enum AnnounceType
	{
		EMERGENCY = 0,
		ANNOUNCEMENT,
		AD,
		NOTICE,
		WARNING,
		ERR,
		RESTART
	};
	uint32_t type;
	std::string msg;
	virtual void marshal(sox::Pack &pk) const {
		pk << type << msg;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> type >> msg;
	}
};


struct PDisableVoice : public sox::Marshallable {
	enum {uri = (48 << 8 | protocol::SESSION_SVID)};
	bool disable;
	uint32_t sid;
	uint32_t admin;
	uint32_t uid;
	virtual void marshal(sox::Pack &pk) const {
		pk << uid << disable << sid << admin;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid >> disable >> sid >> admin;
	}
};

struct PDisableText : public PDisableVoice {
	enum {uri = (49 << 8 | protocol::SESSION_SVID)};
	virtual void marshal(sox::Pack &pk) const {
		PDisableVoice::marshal(pk);
	}
	virtual void unmarshal(const sox::Unpack &up) {
		PDisableVoice::unmarshal(up);
	}
};

struct PJoinQueue : public sox::Marshallable {
	enum {uri = (50 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	virtual void marshal(sox::Pack &pk) const {
		pk << uid;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid;
	}
};

struct PLeaveQueue : public sox::Marshallable {
	enum {uri = (51 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	virtual void marshal(sox::Pack &pk) const {
		pk << uid;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid;
	}
};

struct PKickOffQueue : public sox::Marshallable {
	enum {uri = (52 << 8 | protocol::SESSION_SVID)};
	uint32_t admin;//服务器填写admin 
	uint32_t uid; 
	virtual void marshal(sox::Pack &pk) const {
		pk << uid << admin;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid >> admin;
	}
};


struct PDoubleTimeQueue : public sox::Marshallable {
	enum {uri = (53 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	uint32_t admin;//服务器填写admin 
	uint32_t time;
	virtual void marshal(sox::Pack &pk) const {
		pk << uid << time << admin;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid >> time >> admin;
	}
};

struct PMuteQueue : public sox::Marshallable {
	enum {uri = (54 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	uint32_t time;
	bool mute;
	virtual void marshal(sox::Pack &pk) const {
		pk << uid << time << mute;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid >> time >> mute;
	}
};

/*
 * 管理员移动麦序
*/
struct PMoveQueue : public sox::Marshallable {
	enum {uri = (55 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	bool down;
	virtual void marshal(sox::Pack &pk) const {
		pk << uid << down;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid >> down;
	}
};


struct PTurnQueue : public sox::Marshallable {
	enum {uri = (56 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	uint32_t time;
	virtual void marshal(sox::Pack &pk) const {
		pk << uid << time;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid >> time;
	}
};

struct PTimeOutQueue : public sox::Marshallable {
	enum {uri = (57 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	virtual void marshal(sox::Pack &pk) const {
		pk << uid;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid;
	}
};

struct PDisableQueue : public sox::Marshallable {
	enum {uri = (58 << 8 | protocol::SESSION_SVID)};
	uint32_t uid; //服务器转换为 admim
	bool disable;
	virtual void marshal(sox::Pack &pk) const {
		pk << uid << disable;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid >> disable;
	}
};

struct PSyncTime : public sox::Marshallable {
	enum {uri = (59 << 8 | protocol::SESSION_SVID)};
	uint32_t startTime;
	virtual void marshal(sox::Pack &pk) const {
		pk << startTime;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> startTime;
	}
};

struct PSyncTimeRes : public PSyncTime {
	enum {uri = (59 << 8 | protocol::SESSION_SVID)};
	uint32_t serverTime;
	virtual void marshal(sox::Pack &pk) const {
		pk << startTime << serverTime;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> startTime >>serverTime;
	}
};

struct PTuoRenQueue : public sox::Marshallable {
	enum {uri = (60 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	uint32_t admin;
	virtual void marshal(sox::Pack &pk) const {
		pk << uid << admin;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid >> admin;
	}
};


struct PRolerMini: public PListMini{
	ChannelRoler role;
	virtual void marshal(sox::Pack &pk) const {
		PListMini::marshal(pk);
		pk.push_uint16(role);
	}
	virtual void unmarshal(const sox::Unpack &up) {
		PListMini::unmarshal(up);
		role = (ChannelRoler)up.pop_uint16();
	}
};


struct PSyncManagers : public sox::Voidmable {
	enum {uri = (62 << 8 | protocol::SESSION_SVID)};
	uint32_t cid;
	virtual void marshal(sox::Pack &pk) const {
		pk << cid;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> cid;
	}
};


struct POnSyncManagers : public sox::Marshallable {
	enum {uri = (62 << 8 | protocol::SESSION_SVID)};
	std::vector<PRolerMini> managers;
	uint32_t cid;
	virtual void marshal(sox::Pack &pk) const {
		sox::marshal_container(pk, managers);
		pk << cid;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		sox::unmarshal_container(up, std::back_inserter(managers));
		up >> cid;
	}
};

struct PFilter : public sox::Marshallable {
	enum {uri = (63 << 8 | protocol::SESSION_SVID)};
	uint32_t stamp;
	uint32_t sid;
	uint32_t uid;
	virtual void marshal(sox::Pack &pk) const {
		pk << stamp << sid << uid;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> stamp >> sid >> uid;
	}
};


struct PSessionSyncInfo : public sox::Marshallable {
	enum {uri = (2 << 8 | protocol::SMANAGER_SVID)};
	uint32_t serverId;
	std::vector<protocol::slist::SListItem> infos;

	virtual void marshal(sox::Pack &p) const {
		p << serverId;
		sox::marshal_container(p, infos);
	}

	virtual void unmarshal(const sox::Unpack &p) {
		infos.clear();
		p >> serverId;
		sox::unmarshal_container(p, std::back_inserter(infos));
	}
};

struct PSessionSyncInfoGet : public sox::Marshallable {
	enum {uri = (3 << 8 | protocol::SMANAGER_SVID)};
	uint32_t top;
	std::vector<protocol::slist::SListItem> items;
	virtual void marshal(sox::Pack &p) const {
		p << top;
		marshal_container(p, items);
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> top;
		unmarshal_container(p, std::back_inserter(items));
	}
};



/*
struct PReconnenct: public sox::Marshallable{
	enum {uri = (64 << 8 | protocol::SESSION_SVID)};
	core::MUUID uuid;
	uint32_t sid;
	//uint32_t ssid;
	virtual void marshal(sox::Pack &p) const {
		p << uuid << sid;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> uuid >> sid;
	}
};
*/

struct PStopVoice: public sox::Voidmable{
	enum{uri = (66 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	uint32_t sid;
	virtual void marshal(sox::Pack &p) const{
		p << uid << sid;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid >> sid;
	}
};


struct PEnableAllText: public sox::Marshallable{
	enum{uri = (67 << 8 | protocol::SESSION_SVID)};
	uint32_t sid;
	bool enable;
	uint32_t admin;
	virtual void marshal(sox::Pack &p) const{
		p << sid << admin << (uint16_t)enable;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> sid >> admin;
		enable = (p.pop_uint16() != 0);
	}
};

struct PSetRoomKeyActive: public sox::Marshallable{
	enum{uri = (68 << 8 | protocol::SESSION_SVID)};
	uint32_t sid;
	uint32_t admin;
	virtual void marshal(sox::Pack &p) const{
		p << sid << admin;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> sid >> admin;
	}
};

struct PEnableAllVoice: public PEnableAllText{
	enum{uri = (69 << 8 | protocol::SESSION_SVID)};
};

struct PKickAllQueue : public sox::Marshallable {
	enum{uri = (70 << 8 | protocol::SESSION_SVID)};
	uint32_t admin;
	virtual void marshal(sox::Pack &p) const {
		p << admin;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> admin;
	}
};

struct PTopQueue : public sox::Marshallable {
	enum{uri = (71 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;

	virtual void marshal(sox::Pack &p) const {
		p << uid;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid;
	}
};

/*struct PLeaveGuild: public PUpdateChannelMember{
	enum{uri = (72 << 8 | protocol::SESSION_SVID)};

};*/


struct PReSendVoice : public sox::Marshallable{
	enum{uri = (73 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	uint32_t seq;
	virtual void marshal(sox::Pack &p) const{
		p << uid << seq;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid >> seq;
	}
};


/*struct PReconnectSess: public sox::Marshallable{
	enum{uri = (74 << 8 | protocol::SESSION_SVID)};
	core::MUUID tryId;
	virtual void marshal(sox::Pack &p) const {
		p << tryId;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> tryId;
	}
};*/

struct PDirectKickOff: public PSessKickOff{
	enum{uri = (75 << 8 | protocol::SESSION_SVID)};
};



struct PBroadcastText: public PChatText{
	enum{uri = (77 << 8 | protocol::SESSION_SVID)};

};

struct PTcpPing: public PUdpPing{
	enum{uri = (78 << 8 | protocol::SESSION_SVID)};
	
};

struct PTcpPingRes: public protocol::login::PPlus{
	enum{uri = (78 << 8 | protocol::SESSION_SVID)};

};

struct PUdpClose: public sox::Voidmable{
	enum{uri = (79 << 8 | protocol::SESSION_SVID)};
};

struct PJoinQueueRes: public sox::Marshallable{
	enum{uri = (80 << 8 | protocol::SESSION_SVID)};
	std::vector<uint32_t> uids;
	virtual void marshal(sox::Pack &p) const {
		sox::marshal_container(p, uids);
	}
	virtual void unmarshal(const sox::Unpack &up) {
		sox::unmarshal_container(up, std::back_inserter(uids));
	}
};


//restart server, for region choose
struct PRestartChannel : public  sox::Marshallable {
	enum {uri = (223 << 8 | protocol::SESSION_SVID)};
	virtual void marshal(sox::Pack &p) const {
	}
	virtual void unmarshal(const sox::Unpack &p) {
	}
};

/*struct PAdminChatVoice : public PChatVoice{
	enum {uri = (225 << 8 | protocol::SESSION_SVID)};
	virtual void marshal(sox::Pack &pak) const {
		PChatVoice::marshal(pak);
	}
	virtual void unmarshal(const sox::Unpack &pak) {
		PChatVoice::unmarshal(pak);
	}
};
*/

struct PSyncUStatus  : public  sox::Marshallable {
	enum {uri = (227 << 8 | protocol::SESSION_SVID)};
	virtual void marshal(sox::Pack &p) const {
	}
	virtual void unmarshal(const sox::Unpack &p) {
	}
};

struct PSyncUStatusRes : public  sox::Marshallable {
	enum {uri = (228 << 8 | protocol::SESSION_SVID)};
	typedef std::pair<uint32_t, std::string> userstatus;
	std::list<userstatus> allstatus;
	virtual void marshal(sox::Pack &p) const {
			sox::marshal_container(p, allstatus);
	}
	virtual void unmarshal(const sox::Unpack &p) {
			sox::unmarshal_container(p, std::back_inserter(allstatus));
	}
};

struct PDumpSession : public sox::Marshallable {
        enum {uri = (229 << 8 | protocol::SESSION_SVID)};
        uint32_t sid;
        virtual void marshal(sox::Pack &p) const {
                        p << sid;
        }
        virtual void unmarshal(const sox::Unpack &p) {
                        p >> sid;
        }
};

struct PChangeOrder : public sox::Marshallable {
		enum {uri = 230 << 8 | protocol::SESSION_SVID};
		uint32_t sid; 
		std::vector<uint32_t> order;
		
		virtual void marshal(sox::Pack &p) const {
				p << sid;
				sox::marshal_container(p, order);
		}
		virtual void unmarshal(const sox::Unpack &p) {
				p >> sid;
				sox::unmarshal_container(p, std::back_inserter(order));	
		}
};
	
	
struct PSynOrder : public PChangeOrder{
		enum {uri = 231 << 8 | protocol::SESSION_SVID};
};


struct PTcpConnect : public sox::Marshallable {
		enum {uri = 232 << 8 | protocol::SESSION_SVID};
		virtual void marshal(sox::Pack &p) const {
		}
		virtual void unmarshal(const sox::Unpack &p) {
		}
};
	
struct PUpdateKickOff : public sox::Marshallable{
	enum {uri = (233 << 8) | protocol::SESSION_SVID};
	uint32_t cid;
	virtual void marshal(sox::Pack &p) const {
		p << cid;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> cid;
	}
};

struct KickData : public sox::Marshallable{
		uint32_t sid;
		uint32_t tick;
		uint32_t admin;
		uint32_t op_time;
		std::string reason;

		virtual void marshal(sox::Pack &pak) const {
			pak << sid << tick << admin << reason << op_time;
		}
		virtual void unmarshal(const sox::Unpack &pak) {
			pak >> sid >> tick >> admin >> reason >> op_time;
		}
};


typedef std::vector<KickData> kick_vec_t;

struct PKickVec : public sox::Marshallable{
		kick_vec_t vec;
		virtual void marshal(sox::Pack &p) const {
			sox::marshal_container(p, vec);
		}
		virtual void unmarshal(const sox::Unpack &p) {
			sox::unmarshal_container(p, std::back_inserter(vec));
		}
};	

	
struct PUpdateKickOffRes : public sox::Marshallable
{
	enum {uri = (234 << 8) | protocol::SESSION_SVID};
	typedef std::map<uint32_t, PKickVec > kuser_map_t;
	uint32_t cid;
	
	std::vector<PListMini> kuserinfos;
	kuser_map_t kicks;

	virtual void marshal(sox::Pack &p) const {
		p << cid;
		
		sox::marshal_container(p, kuserinfos);

		p.push_uint32(uint32_t( kicks.size()));
		for(kuser_map_t::const_iterator it1=kicks.begin(); it1!=kicks.end(); it1++) {   
		    p << it1->first << it1->second;
		}
		
	}
	
	virtual void unmarshal(const sox::Unpack &p) {
		p >> cid;
		sox::unmarshal_container(p, std::back_inserter(kuserinfos));

		for (uint32_t count1 = p.pop_uint32(); count1 > 0; --count1) {
			uint32_t uid;
			p >> uid;
			PKickVec& pk = kicks[uid];
           	p >> pk;
        } 
		
	}
};

struct PRemoveKickOff : public sox::Marshallable{
	enum {uri = (235 << 8) | protocol::SESSION_SVID};
	uint32_t uid;
	uint32_t cid;
	virtual void marshal(sox::Pack &p) const {
		p << uid << cid;
	}

	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid >> cid;
	}
};

struct PRemoveKickOffRes : PRemoveKickOff
{
	enum {uri = (236 << 8) | protocol::SESSION_SVID};
};

struct PMemberText: public PHistoryData{
	enum{uri = (237 << 8 | protocol::SESSION_SVID)};
};


struct PMemberInfo : public sox::Marshallable{
	uint32_t uid;
	ChannelRoler roler;
	std::string account;
	std::string nick;
	std::string sign;
	uinfo::SEX gender;
	uint32_t user_jifen;
	uint32_t usess_jifen;
	uint32_t addtime;
	virtual void marshal(sox::Pack &p) const {
		p <<  uid << account <<  nick	<< sign << user_jifen << usess_jifen << addtime;
		p.push_uint16(roler);
		p.push_uint8(gender);
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid >> account >> nick >>  sign >> user_jifen >> usess_jifen >> addtime;
		roler =  (ChannelRoler)p.pop_uint16();
		gender = (uinfo::SEX)p.pop_uint8();
	}
};

struct PGetMemberList : public sox::Marshallable {
	enum {uri = (238 << 8 | protocol::SESSION_SVID)};
	virtual void marshal(sox::Pack &p) const {
	}

	virtual void unmarshal(const sox::Unpack &up) {
	}
};


struct PMemberList: public sox::Marshallable{
	enum { uri = (238 << 8 | protocol::SESSION_SVID)};
	std::vector<PMemberInfo> members;
	virtual void marshal(sox::Pack &p) const {
		sox::marshal_container(p, members);
	}

	virtual void unmarshal(const sox::Unpack &up) {
		sox::unmarshal_container(up, std::back_inserter(members));
	}
};

struct PMemberHistory: public sox::Marshallable{
	enum { uri = (239 << 8 | protocol::SESSION_SVID)};
	std::vector<PHistoryData> historys;
	virtual void marshal(sox::Pack &p) const {
		sox::marshal_container(p, historys);
	}

	virtual void unmarshal(const sox::Unpack &up) {
		sox::unmarshal_container(up, std::back_inserter(historys));
	}
};

struct PNotice : public sox::Marshallable{
	enum { uri = (240 << 8 | protocol::SESSION_SVID)};
	enum Notice_TYPE {
		allUser = 1,
		allMember = 2,
		allAdmin = 3,
		onlyexp_sess = 4,
		onlyexp_auto = 5,
		onlyexp_link = 6
	};
	Notice_TYPE notice_type;
	uint32_t exp;
	uint32_t type;
	uint32_t resources;
	std::string info;
	virtual void marshal(sox::Pack &p) const {
		p << exp << type << resources << info;
		p.push_uint8(notice_type);
	}
	
	virtual void unmarshal(const sox::Unpack &up) {
		up >> exp >> type >> resources >> info;
		notice_type = (Notice_TYPE)up.pop_uint8();
	}
};

struct PForceAdminCommand : public sox::Marshallable{
	enum { uri = (241 << 8 | protocol::SESSION_SVID)};
	uint32_t sid;
	uint32_t uid;
	std::string command;
	virtual void marshal(sox::Pack &p) const {
		p << uid << sid << command;
	}

	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid >> sid >> command;
	}
};

struct PZRealJoinChannelRes : public PZipBase<PRealJoinChannelRes> {
	enum {uri = (243 << 8 | protocol::SESSION_SVID)};
};


struct PZMemberList : public PZipBase<PMemberList> {
	enum {uri = (245 << 8 | protocol::SESSION_SVID)};
};

struct PZMemberHistory : public PZipBase<PMemberHistory> {
	enum {uri = (246 << 8 | protocol::SESSION_SVID)};
};

struct MemberCard : public  sox::Marshallable{
		uint32_t uid;
		std::string cname;
		std::string property;
		uint32_t cidentity;
		virtual void marshal(sox::Pack &p) const {
			p <<  uid <<  cname << property;
			p.push_uint32(cidentity);
		}
		virtual void unmarshal(const sox::Unpack &p) {
			p >> uid >> cname >> property;
			cidentity =  p.pop_uint32();
		}
};

struct PMemberCards : public  sox::Marshallable{
	std::vector<MemberCard> memberCards;
	virtual void marshal(sox::Pack &p) const {
		sox::marshal_container(p, memberCards);
	}
	virtual void unmarshal(const sox::Unpack &up) {
		sox::unmarshal_container(up, std::back_inserter(memberCards));
	}
};
struct PGetMemberCards : public sox::Marshallable {
	enum {uri = (247 << 8 | protocol::SESSION_SVID)};
	virtual void marshal(sox::Pack &p) const {
        }
        virtual void unmarshal(const sox::Unpack &up) {
        }

};

struct PZMemberCards : public PZipBase<PMemberCards> {
	enum {uri = (247 << 8 | protocol::SESSION_SVID)};
};

struct PUpdateMemberCard : public MemberCard {
	enum {uri = (248 << 8 | protocol::SESSION_SVID)};
	uint32_t modifyBy;
	bool temp;
	virtual void marshal(sox::Pack &p) const {
			p <<  modifyBy << temp;
			MemberCard::marshal(p);
	}
	virtual void unmarshal(const sox::Unpack &p) {
			p >> modifyBy >> temp;
			MemberCard::unmarshal(p);
	}
};


struct PCommandElem  :  public  sox::Marshallable  {
	std::string		__data;
	uint32_t		__exp;
	
	PCommandElem(){}
	PCommandElem(const std::string& data, uint32_t exp = 0) : __data(data), __exp(exp)
	{
	}
	virtual void marshal(sox::Pack &p) const {
			p.push_varstr32(__data.data(), __data.size());
			p << __exp;
	}
	virtual void unmarshal(const sox::Unpack &p) {
			__data = p.pop_varstr32();
			p >> __exp;
	}
};



struct PBroadcasts :  public  sox::Marshallable {
	enum {uri = (249 << 8 | protocol::SESSION_SVID)};
	std::vector<PCommandElem> commands;
	virtual void marshal(sox::Pack &p) const {
		sox::marshal_container(p, commands);
	}
	virtual void unmarshal(const sox::Unpack &up) {
		sox::unmarshal_container(up, std::back_inserter(commands));
	}
};

struct getMemberListInfo :  public  sox::Marshallable {
	enum {uri = (249 << 8 | protocol::SESSION_SVID)};

	virtual void marshal(sox::Pack &p) const {
	}
	virtual void unmarshal(const sox::Unpack &up) {
	}
};

struct PRealJoinSessionRes : public sox::Marshallable {
	enum {uri = ( 251 << 8 | protocol::SESSION_SVID)};
	uint32_t version;
	SidMapProperties baseInfo;
	std::vector<uint32_t> subs;
	std::vector<AuthCode> authes;
	std::vector<uint32_t> order;
	
	virtual void marshal(sox::Pack &p) const {
		p << version;
		p << baseInfo;
		sox::marshal_container(p, subs);
		sox::marshal_container(p, authes);
		sox::marshal_container(p, order);
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> version;
		p >> baseInfo;
		sox::unmarshal_container(p, std::back_inserter(subs));
		sox::unmarshal_container(p, std::back_inserter(authes));
		sox::unmarshal_container(p, std::back_inserter(order));
	}
};

/*struct PUserBatchRes : public sox::Marshallable {
        enum {uri = ( 252 << 8 | protocol::SESSION_SVID)};
        std::vector<Partner> partners;  //¿¿¿¿
        virtual void marshal(sox::Pack &p) const {
                sox::marshal_container(p, partners);
        }
        virtual void unmarshal(const sox::Unpack &p) {
                sox::unmarshal_container(p, std::back_inserter(partners));
        }
};

//¿¿¿¿¿¿
struct PZipUserRolerRes : public PZipBase<PUserBatchRes> {
        enum {uri = (253 << 8 | protocol::SESSION_SVID)};
};

//¿¿¿¿¿¿
struct PUserRolerRes : public sox::Marshallable {
        enum {uri = ( 254 << 8 | protocol::SESSION_SVID)};
        std::vector<MRolerAndChUser> rolers;  //¿¿¿¿
        virtual void marshal(sox::Pack &p) const {
                sox::marshal_container(p, rolers);
        }
        virtual void unmarshal(const sox::Unpack &p) {
                sox::unmarshal_container(p, std::back_inserter(rolers));
        }
};



//¿¿¿¿¿¿
struct PDisablesRes : public sox::Marshallable {
        enum {uri = ( 255 << 8 | protocol::SESSION_SVID)};
        DisableVoiceMap disablevoice;   //¿¿¿¿
        DisableTextMap disabletext;   //¿¿¿¿
        std::vector<uint32_t> chTextDisabled; //¿¿¿¿¿¿¿
        virtual void marshal(sox::Pack &p) const {
                p << disablevoice;
                p << disabletext;
                sox::marshal_container(p, chTextDisabled);
        }
        virtual void unmarshal(const sox::Unpack &p) {
                p >> disablevoice;
                p >> disabletext;
                sox::unmarshal_container(p, std::back_inserter(chTextDisabled));
        }
};

struct PFrontEnd : public sox::Marshallable {
        enum {uri = ( 255 << 8 | protocol::SESSION_SVID)};
	std::string ip;
        std::vector<uint16_t> ports;
        virtual void marshal(sox::Pack &p) const {
                p << ip;
                sox::marshal_container(p, ports);
        }
        virtual void unmarshal(const sox::Unpack &p) {
                p >> ip;
                sox::unmarshal_container(p, std::back_inserter(ports));
        }
};

struct PReFrontEnd : public PFrontEnd {
        enum {uri = ( 255 << 8 | protocol::SESSION_SVID)};
};

struct PFrontPing : public sox::Marshallable{
};
*/
struct PForwardToPlug  : public sox::Marshallable {
	enum {uri = (253 << 8 | protocol::SESSION_SVID)};
	uint32_t 	PIType;
	std::string cmd;
	virtual void marshal(sox::Pack &p) const {
		p << PIType;
		p.push_varstr32(cmd.data(), cmd.size());
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> PIType;
		cmd = p.pop_varstr32();
	}
	
};

struct PPlugToClient  : public sox::Marshallable {
	enum {uri = (254 << 8 | protocol::SESSION_SVID)};
	uint32_t 	PIType;
	std::string cmd;
	virtual void marshal(sox::Pack &p) const {
		p << PIType;
		p.push_varstr32(cmd.data(), cmd.size());
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> PIType;
		cmd = p.pop_varstr32();
	}
	
};

struct PUpdateSessInfo : public PSessionInfo {
	enum {uri = (256 << 8 | protocol::SESSION_SVID)};
};


struct PUpdateSessInfoRes : public PUpdateSessInfo {
	enum {uri = (257 << 8 | protocol::SESSION_SVID)};
};

struct PSessionLastError : public sox::Marshallable {
	enum {uri = (258 << 8 | protocol::SESSION_SVID)};
	uint32_t 	lastErrno;
	uint32_t	time;
	std::string lastError;
	virtual void marshal(sox::Pack &p) const {
		p << lastErrno << time << lastError;;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> lastErrno >> time >> lastError;
	}
};

struct PBroadcastEvent : public sox::Marshallable {
	enum {uri = (259 << 8 | protocol::SESSION_SVID)};
	uint32_t 	from;
	uint32_t	sid;
	uint32_t	code;
	std::string info;
	virtual void marshal(sox::Pack &p) const {
		p << from << sid << code << info;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> from >> sid >> code >> info;
	}
	
};


struct PUserEffect : public sox::Marshallable {
	uint32_t uid;
	std::vector<PSimpleEffect> simeffects;   // only type

	virtual void marshal(sox::Pack &p) const {
		p << uid;
		sox::marshal_container(p, simeffects);
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid;
		sox::unmarshal_container(p, std::back_inserter(simeffects));
	}
	
};

struct PSyncEffects : public sox::Marshallable {
	enum {uri = (260 << 8 | protocol::SESSION_SVID)};
	uint32_t run;
	std::vector<PUserEffect> effects;
	virtual void marshal(sox::Pack &p) const {
		sox::marshal_container(p, effects);
		p << run;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		sox::unmarshal_container(p, std::back_inserter(effects));
		p >> run;
	}
	
};


struct PAddEffects : public sox::Marshallable {
	enum {uri = (261 << 8 | protocol::SESSION_SVID)};
	uint32_t run;
	std::vector<PUserEffect> effects;
	virtual void marshal(sox::Pack &p) const {
		sox::marshal_container(p, effects);
		p << run;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		sox::unmarshal_container(p, std::back_inserter(effects));
		p >> run;
	}
	
};


struct PAddEffect : public PSimpleEffect {
	enum {uri = (262 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	virtual void marshal(sox::Pack &p) const {
		PSimpleEffect::marshal(p);
		p << uid;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		PSimpleEffect::unmarshal(p);
		p >> uid;
	}
	
};

struct PEmptyChatVoice : public sox::Marshallable{
        enum {uri = (263 << 8 | protocol::SESSION_SVID)};
        uint32_t from;
        uint32_t sid;
		uint32_t seq;

        virtual void marshal(sox::Pack &pak) const {
                pak << from << sid << seq;
        }
        virtual void unmarshal(const sox::Unpack &pak) {
                pak >> from >> sid >> seq;
        }
};

struct PRemoveEffect : public sox::Marshallable {
	enum {uri = (264 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	uint32_t eid;
	virtual void marshal(sox::Pack &p) const {
		p << uid << eid;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid >> eid;
	}
	
};

struct PMutedSelf : public sox::Marshallable {
        enum {uri = (265 << 8 | protocol::SESSION_SVID)};
        bool muted;
        virtual void marshal(sox::Pack &p) const {
                p << muted;
        }
        virtual void unmarshal(const sox::Unpack &p) {
                p >> muted;
        }
};

struct PMutedSelfRes : public PMutedSelf {
        enum {uri = (266 << 8 | protocol::SESSION_SVID)};
};


struct PChatVoice : public PBaseSessVoice{
        enum {uri = (38 << 8 | protocol::SESSION_SVID)};

	virtual uint32_t getUri() {
		return uri;
	}
	virtual PBaseSessVoice* realClone() const {
			return new PChatVoice(*this);
	}
	PChatVoice() {
		vt = (protocol::session::VoiceType)using_chaninfo;
	}
	PChatVoice(PChatVoice *pcv) {
		*this = *pcv;  	
	}
	virtual ~PChatVoice() {
	}
        virtual void marshal(sox::Pack &pak) const {
		PBaseSessVoice::marshal(pak);
        }
        virtual void unmarshal(const sox::Unpack &pak) {
		PBaseSessVoice::unmarshal(pak);
        }
};

struct PReSendVoiceRes: public PChatVoice{
	enum{uri = (74 << 8 | protocol::SESSION_SVID)};
	PReSendVoiceRes() {
		isResend = true;
	}
};

struct PBroadcastVoice: public PChatVoice{
	enum{uri = (76 << 8 | protocol::SESSION_SVID)};
	virtual uint32_t getUri() {
		return uri;
	}

	PBroadcastVoice() {
		vt = (protocol::session::VoiceType)using_broadcast;
	}

};


struct PExpandTypeVoice : public PBaseSessVoice{
        enum {uri = (267 << 8 | protocol::SESSION_SVID)};
	uint8_t type;
	virtual uint32_t getUri() {
		return uri;
	}
	virtual PBaseSessVoice* realClone() const{
			return new PExpandTypeVoice(*this);
	}
	PExpandTypeVoice() {
		vt = (protocol::session::VoiceType)using_type;
	}
	PExpandTypeVoice(const PExpandTypeVoice *petv) {
		*this = *petv;    	
	}

	virtual ~PExpandTypeVoice() {
	}

        virtual void marshal(sox::Pack &pak) const {
		PBaseSessVoice::marshal(pak);
		pak.push_uint8(type);
        }
        virtual void unmarshal(const sox::Unpack &pak) {
		PBaseSessVoice::unmarshal(pak);
		type = pak.pop_uint8();
        }
};

struct PExpandQcVoice : public PBaseSessVoice{
        enum {uri = (268 << 8 | protocol::SESSION_SVID)};
	uint32_t qc;

	virtual uint32_t getUri() {
		return uri;
	}
	virtual PBaseSessVoice* realClone() const{
			return new PExpandQcVoice(this);
	}

	virtual ~PExpandQcVoice() {
	}
	PExpandQcVoice() {
		vt = (protocol::session::VoiceType)using_expandQC;
	}
	PExpandQcVoice(const PExpandQcVoice* v) {
		*this = *v;
	}
        virtual void marshal(sox::Pack &pak) const {
		PBaseSessVoice::marshal(pak);
                pak << qc;
        }
        virtual void unmarshal(const sox::Unpack &pak) {
		PBaseSessVoice::unmarshal(pak);
                pak >> qc;
        }
};

struct PReSendTypeVoiceRes : public PExpandTypeVoice{
	enum{uri = (269 << 8 | protocol::SESSION_SVID)};
	PReSendTypeVoiceRes() {
		isResend = true;
	}
};


struct PReSendQcVoiceRes : public PExpandQcVoice{
	enum{uri = (270 << 8 | protocol::SESSION_SVID)};
	PReSendQcVoiceRes() {
		isResend = true;
	}
};

struct PReSendBcVoiceRes : public PBroadcastVoice{
	enum{uri = (271 << 8 | protocol::SESSION_SVID)};
	PReSendBcVoiceRes() {
		isResend = true;
	}
};

struct PVoiceStatus  : public  sox::Marshallable {
	uint32_t uid;
	uint16_t succ_percentage;
	uint16_t loss_percentage;
	uint16_t error_percentage;
	uint16_t first_percentage;
	uint16_t resend_succ_percentage;
	uint16_t resend_late_percentage;
	uint16_t resend_error_percentage;
	virtual void marshal(sox::Pack &p) const {
		 p << uid << succ_percentage << loss_percentage << error_percentage << first_percentage;
		 p << resend_succ_percentage << resend_late_percentage << resend_error_percentage;
        }
        virtual void unmarshal(const sox::Unpack &pak) {
		 pak >> uid >> succ_percentage >> loss_percentage >> error_percentage >> first_percentage;
		 pak >> resend_succ_percentage >> resend_late_percentage  >> resend_error_percentage;
        }
};

struct PUserVoiceStatus  : public  sox::Marshallable {
	enum{uri = (272 << 8 | protocol::SESSION_SVID)};
	uint32_t ping;
	uint32_t jitter;
	std::vector<PVoiceStatus> voices;
	virtual void marshal(sox::Pack &p) const {
		p << ping << jitter;
		sox::marshal_container(p, voices);
        }
        virtual void unmarshal(const sox::Unpack &pak) {
		pak >> ping >> jitter;
		sox::unmarshal_container(pak, std::back_inserter(voices));
        }
};

struct PQueueAddGuest : public sox::Marshallable {
	enum {uri = (273 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	uint32_t admin;
	virtual void marshal(sox::Pack &pk) const {
		pk << uid << admin;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid >> admin;
	}
};

struct PQueueRemoveGuest : public sox::Marshallable {
	enum {uri = (274 << 8 | protocol::SESSION_SVID)};
	uint32_t uid;
	uint32_t admin;
	virtual void marshal(sox::Pack &pk) const {
		pk << uid << admin;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid >> admin;
	}
};

struct PQueueClearGuest : public sox::Marshallable {
	enum {uri = (275 << 8 | protocol::SESSION_SVID)};
	uint32_t admin;
	virtual void marshal(sox::Pack &pk) const {
		pk << admin;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> admin;
	}
};

struct PRealJoinSession: public sox::Marshallable {
        enum {uri = (276 << 8 | protocol::SESSION_SVID)};
        uint32_t uid;
        uint32_t sid;
        uint32_t ssid;
        uint32_t version;
        uint32_t verify_style;
        std::string verify_token;
        std::string ssPass;

        virtual void marshal(sox::Pack &p) const {
                p << uid << sid << ssid << ssPass << version << verify_style << verify_token;
        }
        virtual void unmarshal(const sox::Unpack &p) {
                p >> uid >> sid >> ssid >> ssPass >> version >> verify_style >> verify_token;
        }
};
//
//struct PFront


//std::vector<Partner> partners;
//std::vector<uint32_t> subs;

struct PJoinSession : public PRealJoinChannel {
	enum {uri = (300 << 8 | protocol::SESSION_SVID)};
	uint32_t client_id_;
	 virtual void marshal(sox::Pack &pack) const {
		PRealJoinChannel::marshal(pack);
		pack << client_id_;
    }
    virtual void unmarshal(const sox::Unpack &unpack) {
		PRealJoinChannel::unmarshal(unpack);
        unpack >> client_id_;
    }
};

struct PJoinSessionRes : public sox::Marshallable {
	enum {uri = (301 << 8 | protocol::SESSION_SVID)};
	uint32_t join_rescode;
    virtual void marshal(sox::Pack &p) const {
		p << join_rescode;
    }
    virtual void unmarshal(const sox::Unpack &p) {
        p >> join_rescode;
    }
};

struct PGetSessionMain : public sox::Marshallable {
       enum {uri = (302 << 8 | protocol::SESSION_SVID)};
        virtual void marshal(sox::Pack &p) const {
        }
        virtual void unmarshal(const sox::Unpack &up) {
        }
};

struct PGetSessionMainRes  : public sox::Marshallable {
       enum {uri = (303 << 8 | protocol::SESSION_SVID)};
	    uint16_t currect_seq_;
		sox::Properties	 session_info;
        SidMapProperties channel_info;
		std::vector<uint32_t> orders;
		std::vector<std::pair<uint32_t, uint32_t> > user_channel;
        virtual void marshal(sox::Pack &p) const {
			p << currect_seq_ << session_info << channel_info << orders << user_channel;
        }
        virtual void unmarshal(const sox::Unpack &up) {
			up >> currect_seq_ >> session_info >> channel_info >> orders >> user_channel; 
        }
};

struct PGetSessionAuth : public sox::Marshallable {
	enum {uri = (304 << 8 | protocol::SESSION_SVID)};
	virtual void marshal(sox::Pack &p) const {
    }
    virtual void unmarshal(const sox::Unpack &up) {
    }
};

struct PGetSessionAuthRes : public sox::Marshallable {
	enum {uri = (305 << 8 | protocol::SESSION_SVID)};
	std::vector<AuthCode> auth_codes;
	virtual void marshal(sox::Pack &p) const {
		p << auth_codes;
    }
    virtual void unmarshal(const sox::Unpack &up) {
		up >> auth_codes;
    }
};

struct PGetDisableVoice : public sox::Marshallable {
	enum {uri = (306 << 8 | protocol::SESSION_SVID)};
	virtual void marshal(sox::Pack &p) const {
	}
	virtual void unmarshal(const sox::Unpack &up) {
	}
};

struct PGetDisableVoiceRes : public sox::Marshallable {
	enum {uri = (307 << 8 | protocol::SESSION_SVID)};
	DisableVoiceMap disable_voices;
	virtual void marshal(sox::Pack &p) const {
		p << disable_voices;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> disable_voices;
	}
};

struct PGetDisableText : public sox::Marshallable {
	enum {uri = (308 << 8 | protocol::SESSION_SVID)};
	virtual void marshal(sox::Pack &p) const {
	}
	virtual void unmarshal(const sox::Unpack &up) {
	}
};

struct PGetDisableTextRes : public sox::Marshallable {
	enum {uri = (309 << 8 | protocol::SESSION_SVID)};
	DisableTextMap disable_texts;
	virtual void marshal(sox::Pack &p) const {
		p << disable_texts;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> disable_texts;
	}
};

struct PGetUserRoles : public sox::Marshallable {
	enum {uri = (310 << 8 | protocol::SESSION_SVID)};
	virtual void marshal(sox::Pack &p) const {
	}
	virtual void unmarshal(const sox::Unpack &up) {
	}
};

struct PGetUserRolesRes : public sox::Marshallable {
	enum {uri = (311 << 8 | protocol::SESSION_SVID)};
	std::vector<MRolerAndChUser> user_roles;
	virtual void marshal(sox::Pack &p) const {
		p << user_roles;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> user_roles;
	}
};


struct UserInfo : public sox::Marshallable {
	uint32_t uid;
	uint32_t user_jifen;
	uint8_t gender;
	uint16_t dev;
	std::string nick;
	virtual void marshal(sox::Pack &p) const {
		p << uid << user_jifen << gender << dev << nick;
	}
	
	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid >> user_jifen >> gender >> dev >> nick;
	}
};



struct PGetUserInfo : public sox::Marshallable {
	enum {uri = (312 << 8 | protocol::SESSION_SVID)};
	std::vector<uint32_t> uids;
	virtual void marshal(sox::Pack &p) const {
		p << uids;
    }
    virtual void unmarshal(const sox::Unpack &up) {
		up >> uids;
    }
};

struct PGetUserInfoRes : public sox::Marshallable {
	enum {uri = (313 << 8 | protocol::SESSION_SVID)};
	std::vector<UserInfo> user_infos_;
	std::vector<uint32_t> uids_nofound_;
	virtual void marshal(sox::Pack &p) const {
		p << user_infos_ << uids_nofound_;
    }
    virtual void unmarshal(const sox::Unpack &up) {
		up >> user_infos_ >> uids_nofound_;
    }
};


struct PGetMicrophoneQueue : public sox::Marshallable {
    enum {uri = (314 << 8 | protocol::SESSION_SVID)};
    virtual void marshal(sox::Pack &p) const {
    }
    virtual void unmarshal(const sox::Unpack &up) {
    }
};


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


struct PGetBackUpProtocolFronts : public sox::Marshallable {
    enum {uri = (315 << 8 | protocol::SESSION_SVID)};
    uint8_t number_;
    virtual void marshal(sox::Pack &p) const {
        p << number_;
    }
    virtual void unmarshal(const sox::Unpack &up) {
        up >> number_;
    }
};

struct PGetBackUpProtocolFrontsRes : public sox::Marshallable {
    enum {uri = (316 << 8 | protocol::SESSION_SVID)};
    std::vector<std::pair<std::string, uint16_t> > front_addrs_;
    virtual void marshal(sox::Pack &p) const {
        p << front_addrs_;
    }
    virtual void unmarshal(const sox::Unpack &up) {
        up >> front_addrs_;
    }
};

struct PUserSubcribeMessage: public sox::Marshallable{
    enum {uri = (317 << 8 | protocol::SESSION_SVID)};
	enum SubcribeMessageType{
         NUL_MESSAGE,
         ALL_MESSAGE,
         SEQUENCE_MESSAGE
     };
    uint32_t message_type_;

    virtual void marshal(sox::Pack &p) const {
        p << message_type_;
    }
    virtual void unmarshal(const sox::Unpack &up) {
        up >> message_type_;
    }
};

struct PGetSeqList: public sox::Marshallable {
    enum {uri = (318 << 8) | SESSION_SVID};
    virtual void marshal(sox::Pack &pack) const {
    }
    virtual void unmarshal(const sox::Unpack &unpack) {
    }
};

struct PGetSeqListRes : public sox::Marshallable {
    enum {uri = (319 << 8) | SESSION_SVID};
    std::vector<uint32_t> uris_;
    virtual void marshal(sox::Pack &pack) const {
        pack << uris_;
    }
    virtual void unmarshal(const sox::Unpack &unpack) {
        unpack >> uris_;
    }
};


struct PNotifyDisconnect: public sox::Marshallable {
    enum {uri = (320 << 8) | SESSION_SVID};

    virtual void marshal(sox::Pack &pack) const {
    }    
    virtual void unmarshal(const sox::Unpack &unpack) {
    }    
};

struct PGetUserSessionCredit : public sox::Marshallable {
	enum {uri = (321 << 8) | SESSION_SVID};
    std::vector<uint32_t> uids_;
	uint32_t sid_;
    virtual void marshal(sox::Pack &pack) const {
        pack << uids_ << sid_;
    }
    virtual void unmarshal(const sox::Unpack &unpack) {
        unpack >> uids_ >> sid_;
    }
};

struct PGetUserSessionCreditRes : public sox::Marshallable {
	enum {uri = (322 << 8) | SESSION_SVID};
	std::vector<std::pair<uint32_t, uint32_t> > uids_scredit_;
	uint32_t sid_;
    virtual void marshal(sox::Pack &pack) const {
        pack << uids_scredit_ << sid_;
    }
    virtual void unmarshal(const sox::Unpack &unpack) {
        unpack >> uids_scredit_ >> sid_;
    }
};

struct POnVoiceRes : public sox::Marshallable {
	enum {uri = (323 << 8) | SESSION_SVID};
	uint32_t seq_;
    virtual void marshal(sox::Pack &pack) const {
        pack << seq_;
    }
    virtual void unmarshal(const sox::Unpack &unpack) {
        unpack >> seq_;
    }
};

}
}
#endif /*PCOMMON_SESSION_H_*/
 