#ifndef PSINFO_H_
#define PSINFO_H_
#include "common/iproperty.h"
#include "protocol/const.h"
#include "protocol/psessionbase.h"
#include "pslist.h"
#include "plink.h"
#include <iterator>
#include <vector>

namespace protocol{
namespace session{
//uri 64 - 95
/*
struct PSyncSInfo: public PSessionSid{
	enum { uri = (64 << 8) | SESSION_SVID };
};
*/
struct PSessionInfo: public sox::Properties{
	
};

struct PUpdateChannelInfo : public PSessionInfo{
	enum { uri = (65 << 8) | SDB_SVID };
	uint32_t sid;
	virtual void marshal(sox::Pack &p) const{
		p << sid;
		PSessionInfo::marshal(p);
		
	}
	virtual void unmarshal(const sox::Unpack &p){
		p >> sid;
		PSessionInfo::unmarshal(p);
	}
};

struct POnChannelInfoUpdated: public PUpdateChannelInfo{
	
};


struct PUpdateChannelMember: public sox::Marshallable{
	enum { uri = (66 << 8) | SDB_SVID };

	enum RolerOp{
	    NULOP,
		ADD,
		REMOVE,
		CHANGE
	};

	uint32_t channelId;
	uint32_t uid;
	uint32_t admin;
	ChannelRoler roler; // new roler of this member
	RolerOp op;

	virtual void marshal(sox::Pack &p) const{
		p  << admin << channelId << uid ;
		p.push_uint16(roler).push_uint16(op);

	}
	virtual void unmarshal(const sox::Unpack &p){
		p >> admin >> channelId >> uid;
		roler = (ChannelRoler)p.pop_uint16();
		op = (RolerOp)p.pop_uint16();
	}

};

struct POnMemberUpdated: public PUpdateChannelMember{
	std::string nick;
	protocol::uinfo::SEX sex;

	virtual void marshal(sox::Pack &p) const{
		PUpdateChannelMember::marshal(p);
		p << nick;
		p.push_uint16(sex);
	}
	virtual void unmarshal(const sox::Unpack &p){
		PUpdateChannelMember::unmarshal(p);
		p >> nick;
		sex = (uinfo::SEX)p.pop_uint16();
	}
};

struct POnMemberUpdatedErr: public PUpdateChannelMember{

};

struct PSDBDismissSubChannel: public PSessionSid{
	enum { uri = (67 << 8) | SDB_SVID };
};


struct PUpdateBanID: public sox::Marshallable{
	enum {uri = (68 << 8) | SDB_SVID};
	uint32_t uid;
	bool isBan;
	std::string account;
	std::string reason;
	virtual void marshal(sox::Pack &p) const{
		p << uid;
		p.push_uint8(isBan);
		p << reason << account;
	}

	virtual void unmarshal(const sox::Unpack &p){
		p >> uid;
		isBan = (p.pop_uint8() != 0);
		p >> reason >> account;
	}
};

struct PUpdateBanIDRes: public PUpdateBanID{
};

struct PUpdateBanIp: public PUpdateBanID{
	enum {uri = (69 << 8) | SDB_SVID};
	uint32_t ip;
	virtual void marshal(sox::Pack &p) const{
		PUpdateBanID::marshal(p);
		p << ip;
	}

	virtual void unmarshal(const sox::Unpack &p){
		PUpdateBanID::unmarshal(p);
		p >> ip;
	}
}; 

struct PUpdateBanIpRes: public PUpdateBanIp{
	
};



struct PGetBans: public sox::Voidmable{
	enum {uri = (70 << 8 | SDB_SVID)};
};



struct POnGetBans: public sox::Voidmable{
	enum {uri = (70 << 8 | SDB_SVID)};
	std::vector<PBanUserMini> ids;
	std::vector<PBanIpMini> ips;
	virtual void marshal(sox::Pack &p) const{
		sox::marshal_container(p, ids);
		sox::marshal_container(p, ips);
	}

	virtual void unmarshal(const sox::Unpack &p){
		sox::unmarshal_container(p, std::back_inserter(ids));
		sox::unmarshal_container(p, std::back_inserter(ips));
	}
};

/*struct PSyncSInfo: public sox::Marshallable{
	enum {uri = (71 << 8 | protocol::SDB_SVID)};	
	uint32_t sid;
	void marshal(sox::Pack & pk) const{
		pk << sid;
	}
	void unmarshal(const sox::Unpack &up){
		up >> sid;
	}
};

struct PSyncSInfoRes: public sox::Properties{
	enum {uri = (71 << 8 | protocol::SDB_SVID)};	
};
*/

struct PGetGuildInfo: public sox::Marshallable{
	enum {uri = (72 << 8 | protocol::SDB_SVID)};
	uint32_t uid;
	void marshal(sox::Pack & pk) const{
		pk << uid;
	}
	void unmarshal(const sox::Unpack &up){
		up >> uid;
	}
};

struct GuildInfo: public sox::Marshallable{
	uint32_t channelId;
	std::string channelName;
	ChannelRoler roler;

	void marshal(sox::Pack & pk) const{
		pk << channelId << channelName << (uint16_t) roler;
	}
	void unmarshal(const sox::Unpack &up){
		up >> channelId >> channelName;
		roler = (ChannelRoler)up.pop_uint16();
	}
};

struct PGetGuildInfoRes: public sox::Marshallable{
	enum {uri = (72 << 8 | protocol::SDB_SVID)};
	uint32_t uid;
	std::vector<GuildInfo> infos;

	void marshal(sox::Pack & pk) const{
		pk << uid;
		sox::marshal_container(pk, infos);
	}
	void unmarshal(const sox::Unpack &up){
		up >> uid;
		sox::unmarshal_container(up, std::back_inserter(infos));
	}
};

struct PLeaveGuild: public sox::Marshallable{
	enum {uri = (73 << 8 | protocol::SDB_SVID)};
	uint32_t sid;
	void marshal(sox::Pack & pk) const{
		pk << sid;
	}
	void unmarshal(const sox::Unpack &up){
		up >> sid;
	}
};

struct PUpdateChannelMemberByAccount: public PUpdateChannelMember{
	enum { uri = (74 << 8) | SDB_SVID };
	std::string account;
	// 
	virtual void marshal(sox::Pack &p) const{
		PUpdateChannelMember::marshal(p);
		p  << account ;

	}
	virtual void unmarshal(const sox::Unpack &p){
		PUpdateChannelMember::unmarshal(p);
		p >>  account;
	}

};


struct PUpdateBanAccount: public sox::Marshallable{
	enum {uri = (75 << 8) | SDB_SVID};
	std::string account;
	bool isBan;
	std::string reason;
	virtual void marshal(sox::Pack &p) const{
		p << account;
		p.push_uint8(isBan);
		p << reason;
	}

	virtual void unmarshal(const sox::Unpack &p){
		p >> account;
		isBan = (p.pop_uint8() != 0);
		p >> reason;
	}
}; 


struct PUpdateBanAccountRes: public PUpdateBanIDRes{
	
};

struct PBatchUpdateBanID: public sox::Marshallable{
	enum {uri = (76 << 8) | SDB_SVID};
	std::vector<uint32_t> uids;
	bool isBan;
	std::string reason;
	virtual void marshal(sox::Pack &p) const{
		sox::marshal_container(p, uids);
		p.push_uint8(isBan);
		p << reason;
	}

	virtual void unmarshal(const sox::Unpack &p){
		sox::unmarshal_container(p, std::back_inserter(uids));
		isBan = (p.pop_uint8() != 0);
		p >> reason;
	}
};

struct PBatchUpdateBanIDRes: public PBatchUpdateBanID{
};

struct PBatchUpdateBanIp: public PBatchUpdateBanID{
	enum {uri = (77 << 8) | SDB_SVID};
	std::vector<uint64_t> ips;
	virtual void marshal(sox::Pack &p) const{
		PBatchUpdateBanID::marshal(p);
		sox::marshal_container(p, ips);
	}

	virtual void unmarshal(const sox::Unpack &p){
		PBatchUpdateBanID::unmarshal(p);
		sox::unmarshal_container(p, std::back_inserter(ips));
	}
}; 

struct PBatchUpdateBanIpRes: public PBatchUpdateBanIp{
	
};




struct PSyncCatalog: public sox::Voidmable{
	enum {uri = (1 << 8 | protocol::CATALOG_SVID)};
};

struct PSyncCatalogRes : public sox::Marshallable {
	enum {uri = (1 << 8 | protocol::CATALOG_SVID)};
	std::vector<protocol::slist::SListItem> items;
	virtual void marshal(sox::Pack &p) const {
		marshal_container(p, items);
	}
	virtual void unmarshal(const sox::Unpack &p) {
		unmarshal_container(p, std::back_inserter(items));
	}
};

}
}
#endif /*PSINFO_H_*/
