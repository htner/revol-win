#ifndef PSESSIONBASE_H_
#define PSESSIONBASE_H_
#include "common/packet.h"
#include "common/iproperty.h"
#include "protocol/const.h"
#include <vector>

namespace protocol {
namespace session {
	enum GuildType{
		gt_game = 0,
		gt_fun,
		gt_other,
	};

	enum VoiceType {
			using_default = 0,
			using_chaninfo = 1,
			using_type =2,
			using_expandQC = 3,
			using_broadcast
	};

	typedef uint32_t VoiceQC;

struct PSessionSid : public sox::Marshallable {
	uint32_t sid;
	PSessionSid() {}
	PSessionSid(uint32_t s): sid(s){}
	virtual void marshal(sox::Pack &p) const {
		p << sid;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> sid;
	}
};

///////////////////////////////////////////////////////////////////
typedef std::map<uint64_t, std::set<uint64_t> > is_map_t;

struct ChannelUserMap : public sox::Marshallable
{
    is_map_t us; 
		virtual void marshal(sox::Pack & p) const {   
        p.push_uint32(uint32_t(us.size()));
        for(is_map_t::const_iterator it1=us.begin(); it1!=us.end(); it1++) {   
            std::set<uint64_t> const &refset=it1->second;
            p.push_uint32(uint32_t( refset.size() ) );
            for(std::set<uint64_t>::const_iterator it2=refset.begin(); it2!=refset.end(); it2++) {   
                p.push_uint64( uint64_t(it1->first) );
                p << *it2;
            }   
        }   
    }   
		virtual void unmarshal(const sox::Unpack &p) {   
        for (uint32_t count1 = p.pop_uint32(); count1 > 0; --count1) {   
            for (uint32_t count2 = p.pop_uint32(); count2 > 0; --count2) {   
				uint64_t key;
				uint64_t value;
                p >> key;
				std::set<uint64_t> &refset = us[key];
                p >> value;
                refset.insert(value);
            }   
        }   
    }   
		virtual std::ostream & trace(std::ostream & os) const {   
        return os; 
    }   	
};

struct DisableVoiceMap : public ChannelUserMap {};
struct DisableTextMap : public ChannelUserMap {};
///////////////////////////////////////////////////////////////////	
struct ChannelQueueMar : public sox::Marshallable {
	bool mute;
	bool disable;
	uint32_t ring;
	uint32_t count;
	uint32_t validring;
protected:
	std::list<uint64_t> userlist;
public:
	ChannelQueueMar():mute(false),disable(false),ring(300),count(0),validring(300){}	

	virtual void marshal(sox::Pack & p) const {
		p << mute << disable << ring << count << validring;
		sox::marshal_container(p, userlist);
	}

	virtual void unmarshal(const sox::Unpack & p) {
		p >> mute >> disable >> ring >> count >> validring;
		sox::unmarshal_container(p, std::back_inserter(userlist));
	}


	const std::list<uint64_t> &getUserList() const{return userlist;}

	std::list<uint64_t> &getUserListRef() {return userlist;}
};

typedef std::map< uint64_t, ChannelQueueMar > channelqueuemar_map_t;
///////////////////////////////////////////////////////////////////	

struct SidMapProperties : public sox::Marshallable {
	sox::sid2props_map_t mapp;

	virtual void marshal(sox::Pack & p) const {
		sox::marshal_container(p, mapp);
	}

	virtual void unmarshal(const sox::Unpack &p) {
		sox::unmarshal_container(p, sox::properties::replace_iterator(mapp));
	}
};

enum ChannelRoler {
	NUL_ROLE = 0,
	NORMAL = 25,		//U
	DELETED = 50,		//unuse
	TMPVIP = 66,		//TMPVIP
	VIP = 88,			//VIP
	MEMBER = 100,		//R
	CMANAGER = 150,		//CA
	PMANAGER = 175,		//PA
	MANANGER = 200,		//MAN
	OWNER = 255,		//OW
	KEFU = 300,			//CS
	SA = 1000,			//SA
};

typedef ChannelRoler ChannelRole;

struct Member : public sox::Marshallable {
	uint64_t uid;
	uint64_t sid;
	ChannelRoler role;
	Member(){}
	Member(uint64_t u, uint64_t s, ChannelRoler r): uid(u), sid(s), role(r){}
	virtual void marshal(sox::Pack & p) const {
		p << uid << sid;
		p.push_uint16((uint8_t)role);
	}

	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid >> sid;
		role = (ChannelRoler)p.pop_uint16();
	}
};


struct MRolerAndCh: public sox::Marshallable{
	uint64_t cid;
	ChannelRoler roler;
	MRolerAndCh(){}
	MRolerAndCh(uint32_t c, ChannelRoler r):cid(c), roler(r){}

	virtual void marshal(sox::Pack &p) const {
		p << cid;
		p.push_uint16(roler);
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> cid;
		roler = (ChannelRoler)p.pop_uint16();
	}
};

struct MRolerAndChUser: public MRolerAndCh{
	uint64_t uid;
	MRolerAndChUser(){}
	MRolerAndChUser(uint64_t u, uint64_t c, ChannelRoler r):MRolerAndCh(c, r), uid(u){}

	virtual void marshal(sox::Pack &p) const {
		p << uid;
		MRolerAndCh::marshal(p);
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid;
		MRolerAndCh::unmarshal(p);
	}
};

enum ItemType{
	MAN,
	CHANNEL
};

enum ChannelStyle{
	FreeStyle,
	ChairStyle,
	MicrophoneStyle
};

struct PListMini : public sox::Marshallable {
	uint32_t uid;
	std::string nick;
	uinfo::SEX gender;
	virtual void marshal(sox::Pack &pk) const {
		pk << uid << nick;
		pk.push_uint8(gender);
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> uid >> nick;
		gender = (protocol::uinfo::SEX)up.pop_uint8();
	}
};

struct PBanUserMini : public PListMini {
	std::string account;
	std::string reason;
	virtual void marshal(sox::Pack &pk) const {
		PListMini::marshal(pk);
		pk << reason << account;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		PListMini::unmarshal(up);
		up >> reason >>  account;
	}
};

struct PBanIpMini: public PBanUserMini{
	uint32_t ip;
	virtual void marshal(sox::Pack &pk) const {
		PBanUserMini::marshal(pk);
		pk << ip;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		PBanUserMini::unmarshal(up);
		up >> ip;
	}
};

struct PHistoryData : public sox::Marshallable{
		uint32_t time;
		uint32_t from;
		std::string chat;
		virtual void marshal(sox::Pack &pak) const {
			pak << time << from << chat;
		}
		virtual void unmarshal(const sox::Unpack &pak) {
			pak >> time >> from >> chat;
		}
};

struct PSimpleEffect : public sox::Marshallable {
	uint32_t eid;
	uint32_t type;
	uint32_t tick;
	virtual void marshal(sox::Pack &p) const {
		p << eid << type << tick;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> eid >> type >> tick;
	}
	
};

struct PVoice : sox::Marshallable{
	uint32_t seq;
#ifdef WIN32_CLIETN_PROTO
	sox::Varstr	chat;
#else
	std::string chat;
#endif	
	virtual void marshal(sox::Pack &pak) const {
          pak << seq << chat;
    }
    virtual void unmarshal(const sox::Unpack &pak) {
           pak >> seq >> chat;
    }
};

class PBaseSessVoice : public PVoice{
public: // as a struct
	enum {uri = 0};
	bool isResend;
	VoiceType vt;
	uint32_t from;
        uint32_t sid;
	PBaseSessVoice() {
		isResend = false;
		vt = using_default;
	}
	virtual ~PBaseSessVoice() {
	}
	virtual PBaseSessVoice* clone () const  {
			return realClone();
	}
	virtual PBaseSessVoice* clone(uint32_t  u)  const{
		if (uri == u) {
			return realClone();
		}
		return NULL;
	}

	virtual PBaseSessVoice* realClone()  const = 0;
	virtual uint32_t getUri() = 0;

        virtual void marshal(sox::Pack &pak) const {
		PVoice::marshal(pak);
                pak << from << sid;
        }
        virtual void unmarshal(const sox::Unpack &pak) {
		PVoice::unmarshal(pak);
                pak >>  from >> sid;
        }
};
typedef std::vector<PSimpleEffect> simeffect_t;
typedef const std::vector<PSimpleEffect> simeffect_const_t;


}
}
#endif /*PSESSIONBASE_H_*/
