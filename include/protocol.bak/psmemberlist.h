#ifndef PSMEMBERLIST_H_
#define PSMEMBERLIST_H_
#include "protocol/psessionbase.h"
#include "protocol/const.h"
#include <iterator>

namespace protocol {
namespace session {

struct POnMemberAdd : public sox::Marshallable {
	uint32_t uid;
	std::string nick;
	std::string sign;
	uinfo::SEX sex;
	uint64_t pid;
	uint32_t user_jifen;
	uint32_t usess_jifen;
	std::vector<MRolerAndCh> rolers;
	virtual void marshal(sox::Pack &p) const {
		p << uid << nick << sign << pid << user_jifen << usess_jifen;
		p.push_uint8(sex);
		sox::marshal_container(p, rolers);
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid >> nick >> sign >> pid >> user_jifen >> usess_jifen ;
		sex = (uinfo::SEX)p.pop_uint8();
		sox::unmarshal_container(p, std::back_inserter(rolers));
	}
};

struct POnMemberRemove : public sox::Marshallable {
	uint64_t uid;
	uint64_t sid;

	virtual void marshal(sox::Pack &p) const {
		p << uid << sid;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> uid >> sid;
	}
};

struct PChangeFolder : public sox::Marshallable {
	enum {uri = (98 << 8) | SESSION_SVID};
	//uint32_t uid;
	uint64_t from;
	uint64_t to;
	std::string passwd;

	virtual void marshal(sox::Pack &p) const {
		p << from << to << passwd;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> from >> to >> passwd;
	}
};


struct PChangePInfo : public sox::Marshallable {
	enum {uri = (99 << 8) | SESSION_SVID};
	std::string nick;
	uinfo::SEX gender;
	std::string sign;
	virtual void marshal(sox::Pack &p) const {
		p  << nick << sign;
		p.push_uint8(gender);
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p  >> nick >> sign;
		gender = (uinfo::SEX)p.pop_uint8();
	}
};

struct PPInfoChanged : public PChangePInfo {
	uint64_t uid;
	virtual void marshal(sox::Pack &pak) const {
		PChangePInfo::marshal(pak);
		pak << uid;
	}
	virtual void unmarshal(const sox::Unpack &pak) {
		PChangePInfo::unmarshal(pak);
		pak >> uid;
	}
};

struct PPushSessData: public sox::Marshallable{
	enum {uri = (100 << 8) | SESSION_SVID};
	std::vector<POnMemberAdd> adds;
	std::vector<uint64_t> removes;
	std::vector<std::pair<uint64_t, uint64_t> > changes;
	std::vector<uint64_t> folders;
	virtual void marshal(sox::Pack &pak) const {
		sox::marshal_container(pak, adds);
		sox::marshal_container(pak, removes);
		sox::marshal_container(pak, changes);
		sox::marshal_container(pak, folders);
	}
	virtual void unmarshal(const sox::Unpack &pak) {
		sox::unmarshal_container(pak, std::back_inserter(adds));
		sox::unmarshal_container(pak, std::back_inserter(removes));
		sox::unmarshal_container(pak, std::back_inserter(changes));
		sox::unmarshal_container(pak, std::back_inserter(folders));
	}
};

struct PMaixuJoin: public ChannelQueueMar{
	enum {uri = (101 << 8) | SESSION_SVID};
	uint64_t channelId;
	std::set<uint64_t> guests;
	bool hasGuests;
	virtual void marshal(sox::Pack &pak) const {
		ChannelQueueMar::marshal(pak);
		pak << channelId << guests;
	}
	virtual void unmarshal(const sox::Unpack &pak) {
		ChannelQueueMar::unmarshal(pak);
		pak >> channelId;
		if (pak.size() >= 4) {
			pak >> guests;
			hasGuests = true;
		} else {
			hasGuests = false;
		}
	}
};

struct PChangeStatus  : public sox::Marshallable {
	enum {uri = (103 << 8) | SESSION_SVID};
	std::string status;
	virtual void marshal(sox::Pack &p) const {
		p  << status;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p  >> status;
	}
};

struct PStatusChanged : public PChangeStatus {
	enum {uri = (104 << 8) | SESSION_SVID};
	uint64_t uid;
	virtual void marshal(sox::Pack &pak) const {
		PChangeStatus::marshal(pak);
		pak << uid;
	}
	virtual void unmarshal(const sox::Unpack &pak) {
		PChangeStatus::unmarshal(pak);
		pak >> uid;
	}
};

struct PUpdateClientInfo : public sox::Marshallable {
	enum {uri = (105 << 8) | SESSION_SVID};
	uint64_t uid;
	uint64_t version;
	std::string client_info;
	virtual void marshal(sox::Pack &p) const {
		p  << uid << version << client_info;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p  >> uid >> version >> client_info;
	}
};


struct PUpdateClientInfos: public sox::Marshallable{
	enum {uri = (106 << 8) | SESSION_SVID};
	std::vector<PUpdateClientInfo> updates;
	virtual void marshal(sox::Pack &pak) const {
		sox::marshal_container(pak, updates);
	}
	virtual void unmarshal(const sox::Unpack &pak) {
		sox::unmarshal_container(pak, std::back_inserter(updates));
	}
};


}
}
#endif /*PSMEMBERLIST_H_*/
