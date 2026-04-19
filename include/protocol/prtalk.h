#ifndef PROTOCOL_RTALK_H
#define PROTOCOL_RTALK_H 

#include "common/iproperty.h"
#include "common/packet.h"

namespace protocol {
namespace rtalk {

struct RUserInfo : public sox::Marshallable {
    uint8_t status;
    uint8_t gender;
    uint32_t uid;
    std::string nick;
    sox::Properties client_info;
    virtual void marshal(sox::Pack &p) const {
		p << status << gender << uid << nick << client_info;
	}
	virtual void unmarshal(const sox::Unpack &up) {
        up >> status >> gender >> uid >> nick >> client_info; 
	}
};

struct PJoinGroup : public sox::Marshallable {
    enum {uri = (1 << 8 | protocol::SESSION_SVID)};
    uint32_t group_id;
    uint32_t uid;
    uint32_t version;	
    uint16_t passwd_version;
    std::string passwd;
    std::string token;
    struct RUserInfo uinfo;
    virtual void marshal(sox::Pack &p) const {
		p << group_id << uid << version << passwd_version << passwd << token << uinfo;
	}
	virtual void unmarshal(const sox::Unpack &up) {
        up >> group_id >> uid >> version >> passwd_version >> passwd >> token >> uinfo;
	}
};

struct PJoinGroupRes : public sox::Marshallable {
	enum {uri = (2 << 8 | protocol::SESSION_SVID)};
    uint32_t key;
    uint32_t rescode;	
    std::string info;
    virtual void marshal(sox::Pack &p) const {
		p << key << rescode << info;
	}
	virtual void unmarshal(const sox::Unpack &up) {
        up >> key >> rescode >> info;
	}
};

struct PGetGroupInfos : public sox::Marshallable {
	enum {uri = (3 << 8 | protocol::SESSION_SVID)};
    uint32_t     sid;
    uint32_t     uid;
    virtual void marshal(sox::Pack &p) const {
        p << sid << uid;
	}
	virtual void unmarshal(const sox::Unpack &up) {
        up >> sid >> uid;
	}
};

struct PGetGroupInfosRes : public sox::Marshallable {
	enum {uri = (3 << 8 | protocol::SESSION_SVID)};
    uint16_t info_version;
    sox::Properties	 group_info;
    std::vector<RUserInfo> uinfos;
    virtual void marshal(sox::Pack &p) const {
		p << info_version << group_info << uinfos;
	}
	virtual void unmarshal(const sox::Unpack &up) {
        up >> info_version >> group_info >> uinfos;
	}
};

struct PAddUser: public sox::Marshallable {
	enum {uri = (4 << 8 | protocol::SESSION_SVID)};
    uint16_t info_version;
    RUserInfo uinfos;
    virtual void marshal(sox::Pack &p) const {
		p << info_version << uinfos;
	}
	virtual void unmarshal(const sox::Unpack &up) {
        up >> info_version >> uinfos;
	}
};

struct PRemoveUser : public sox::Marshallable {
	enum {uri = (5 << 8 | protocol::SESSION_SVID)};
    uint16_t info_version;
    uint32_t uid;
    virtual void marshal(sox::Pack &p) const {
		p << info_version << uid;
	}
	virtual void unmarshal(const sox::Unpack &up) {
        up >> info_version >> uid;
	}
};

struct PGroupInfoChanged : public sox::Marshallable {
	enum {uri = (6 << 8 | protocol::SESSION_SVID)};
    uint16_t info_version;
    sox::Properties group_info_change;
    virtual void marshal(sox::Pack &p) const {
		p << info_version << group_info_change;
	}
	virtual void unmarshal(const sox::Unpack &up) {
        up >> info_version >> group_info_change;
	}

};

struct PUserLeave : public sox::Marshallable {
	enum {uri = (7 << 8 | protocol::SESSION_SVID)};
    virtual void marshal(sox::Pack &) const {
	}
	virtual void unmarshal(const sox::Unpack &) {
	}
};

struct PUserKeepAlive: public sox::Marshallable {
	enum {uri = (8 << 8 | protocol::SESSION_SVID)};
    uint32_t sid;
    uint32_t key;
    virtual void marshal(sox::Pack &p) const {
		p << sid << key;
	}
	virtual void unmarshal(const sox::Unpack &up) {
        up >> sid >> key;
	}
};

struct PGetKeepAliveKey : public sox::Marshallable {
	enum {uri = (9 << 8 | protocol::SESSION_SVID)};
    virtual void marshal(sox::Pack &) const {
	}
	virtual void unmarshal(const sox::Unpack &) {
	}
};


struct PGetKeepAliveKeyRes : public sox::Marshallable {
	enum {uri = (10 << 8 | protocol::SESSION_SVID)};
    uint32_t key;
    virtual void marshal(sox::Pack &p) const {
		p << key;
	}
	virtual void unmarshal(const sox::Unpack &up) {
        up >> key;
	}
};

struct PRVoice : public sox::Marshallable {
	enum {uri = (11 << 8 | protocol::SESSION_SVID)};
    uint32_t uid;
    uint32_t seq;
    std::string voice;
    //TODO 
    //add time_stamp debug
#ifdef RTALK_TIME_STAMP
    uint64_t time_stamp;
#endif
    virtual void marshal(sox::Pack &p) const {
#ifdef RTALK_TIME_STAMP
		p << uid << seq << voice << time_stamp ;
#else
        p << uid << seq << voice;
#endif
	}
	virtual void unmarshal(const sox::Unpack &up) {
#ifdef RTALK_TIME_STAMP
        up >> uid >> seq >> voice >> time_stamp;
#else
        up >> uid >> seq >> voice;
#endif
	}
};

struct PRVoiceRes : public sox::Marshallable {
	enum {uri = (12 << 8 | protocol::SESSION_SVID)};
    uint32_t rescode;
    uint32_t speaker;
    virtual void marshal(sox::Pack &p) const {
        p << rescode << speaker;
    }
    virtual void unmarshal(const sox::Unpack &up) {
        up >> rescode >> speaker;
    }
};

struct PRUserSpeakFailure : public sox::Marshallable {
	enum {uri = (13 << 8 | protocol::SESSION_SVID)};
    uint32_t user;
    uint32_t speaker;
    virtual void marshal(sox::Pack &p) const {
        p << user << speaker;
    }
    virtual void unmarshal(const sox::Unpack &up) {
        up >> speaker >> speaker;
    }
};

struct PRVoiceStop : public sox::Marshallable {
	enum {uri = (14 << 8 | protocol::SESSION_SVID)};
    uint32_t uid;
#ifdef RTALK_TIME_STAMP
    uint64_t time_stamp;
#endif
    virtual void marshal(sox::Pack &p) const {
#ifdef RTALK_TIME_STAMP
		p << uid << time_stamp;
#else
        p << uid;
#endif
	}
	virtual void unmarshal(const sox::Unpack &up) {
#ifdef RTALK_TIME_STAMP
        up >> uid >> time_stamp;
#else
        up >> uid ;
#endif
	}

};

// must send by tcp 
struct PUdpClose : public sox::Marshallable {
	enum {uri = (15 << 8 | protocol::SESSION_SVID)};
    uint32_t uid;
    uint32_t sid;
    uint32_t key;
    virtual void marshal(sox::Pack &p) const {
        p << uid << sid << key;
    }
    virtual void unmarshal(const sox::Unpack &up) {
        up >> uid >> sid >> key;
    }
};

struct PExcludeUser : public sox::Marshallable{
    enum {uri = (17 << 8 | protocol::SESSION_SVID)};
    virtual  void marshal(sox::Pack & ) const {
    }
    virtual void unmarshal(const sox::Unpack  &){
    }
};

struct PExcludeUserRes : public sox::Marshallable{
    enum {uri = (18 << 8 | protocol::SESSION_SVID)};       
    uint32_t      uid;
    uint32_t      rescode;
    virtual void marshal (sox::Pack &p) const {
        p << uid << rescode;    
    }
    virtual void unmarshal(const sox::Unpack &up){
        up >> uid >> rescode;
    }
};

struct PExcludeUserByUid : public sox::Marshallable{
    enum {uri = ( 19 << 8 | protocol::SESSION_SVID)};
    uint32_t                       uid;
    virtual void marshal (sox::Pack &p) const{
        p << uid;
    }
    virtual void unmarshal(const sox::Unpack &up){
        up >> uid;
    }
};

struct PGroupInfoUpdate : public sox::Marshallable {
    enum {uri = (20 << 8 | protocol::SESSION_SVID)};
    uint32_t     info_version;
    sox::Properties group_info_change;
    virtual void marshal(sox::Pack &p) const {
        p << info_version << group_info_change;
    }

    virtual void unmarshal(const sox::Unpack &up){
        up >> info_version >> group_info_change;
    }
};

struct PGroupInfoUpdateRes : public sox::Marshallable{
    enum {uri = (21 << 8 | protocol::SESSION_SVID)};
    uint32_t                 rescode;
    uint32_t                 info_version;
    sox::Properties          info;
    std::vector<uint32_t>    modify_list;
//    uint32_t                 modify_list;
    virtual void marshal(sox::Pack   &p) const {
        p << rescode << info_version << info << modify_list;
    }
    virtual void unmarshal(const sox::Unpack &up){
        up >> rescode >> info_version >>info >> modify_list;
    }
};
/*************************************************
 *
 * udp protocol
 *
 ************************************************/

struct PUdpLogin : public sox::Marshallable {
	enum {uri = (115 << 8 | protocol::SESSION_SVID)};
    uint32_t uid;
    uint32_t sid;
    uint32_t key;
    virtual void marshal(sox::Pack &p) const {
		p << uid << sid << key;
	}
	virtual void unmarshal(const sox::Unpack &up) {
        up >> uid >> sid >> key;
	}
};

struct PUdpPingRes : public sox::Marshallable {
    enum {uri = (132 << 8 | protocol::SESSION_SVID)};   
    uint32_t       counter;
    virtual void marshal(sox::Pack &p) const {
        p << counter;
    }
    virtual void unmarshal(const sox::Unpack &up){
        up >> counter;
    }
};

struct PUdpLoginRes : public PUdpLogin {
    enum {uri = (121 << 8 | protocol::SESSION_SVID)};
};

struct PUdpKeepAlive : public sox::Marshallable {
	enum {uri = (116 << 8 | protocol::SESSION_SVID)};
    uint32_t uid;
    uint32_t sid;
    uint32_t key;
    virtual void marshal(sox::Pack &p) const {
        p << uid << sid << key;
    }
    virtual void unmarshal(const sox::Unpack &up) {
        up >> uid >> sid >> key;
    }
};

/****
 *
 * only udp protocol will lost packet
 *
 ****/

struct PUdpServerSeqLost : public sox::Marshallable {
	enum {uri = (117 << 8 | protocol::SESSION_SVID)};
    uint32_t seq;
    virtual void marshal(sox::Pack &p) const {
        p << seq;
    }
    virtual void unmarshal(const sox::Unpack &up) {
        up >> seq;
    }
};

struct PUdpServerSeqLostRes : public PRVoice {
	enum {uri = (118 << 8 | protocol::SESSION_SVID)};
};


struct PUdpClientSeqLost : public sox::Marshallable {
	enum {uri = (119 << 8 | protocol::SESSION_SVID)};
    uint32_t uid;
    uint32_t seq;
    virtual void marshal(sox::Pack &p) const {
        p << uid << seq;
    }
    virtual void unmarshal(const sox::Unpack &up) {
        up >> uid >> seq;
    }
};

struct PUdpClientSeqLostRes : public PRVoice {
	enum {uri = (120 << 8 | protocol::SESSION_SVID)};
};

struct PGetRVoiceRight : public sox::Marshallable{
    enum {uri = (16 << 8) | protocol::SESSION_SVID};    
    uint32_t    uid;
    uint32_t    sid;
    //TODO
    //add time_stamp
#ifdef RTALK_TIME_STAMP
    uint64_t    time_stamp;
#endif
    virtual void marshal(sox::Pack &p) const {
#ifdef RTALK_TIME_STAMP
        p << uid << sid << time_stamp;
#else
        p << uid << sid ;
#endif
    }

    virtual void unmarshal(const sox::Unpack &up) {
#ifdef RTALK_TIME_STAMP
        up >> uid >> sid >> time_stamp;
#else
        up >> uid >> sid;
#endif
    }

};

struct PUdpPing : public sox::Marshallable {
    enum {uri = (130 << 8 | protocol::SESSION_SVID)};
    uint32_t        counter;   
    virtual void marshal(sox::Pack &p) const {
        p << counter;
    }
    virtual void unmarshal (const sox::Unpack &up){
        up >> counter;
    }
};

}//end namespace rtalk
}//end namespace protocol 


#endif
