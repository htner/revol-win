#ifndef PSMANAGER_H_
#define PSMANAGER_H_
#include "common/core/base_svid.h"
#include "protocol/psessionbase.h"
#include "protocol/plink.h"
#include "protocol/const.h"
#include <vector>
#include <iterator>

namespace protocol {
namespace session {
//uri 0 - 31
struct PCreateChannel : public sox::Marshallable {
	enum {uri = 1 << 8 | protocol::SDB_SVID};

	bool 		bLimit;
	bool 		bPub;
	uint32_t 	pid; // parent channel id
	uint32_t 	tid; // topmost channel id
	uint16_t 	gameType;
	VoiceQC 	qc;
	ISPType 	isp;
	std::string 	password;
	std::string 	name;
	std::string 	hello;

	PCreateChannel() {
	}
	PCreateChannel(const std::string &n, bool l, bool p, const std::string &h,
			ISPType sp = ISP_US, const std::string &pwd = "", uint32_t parent = 0, VoiceQC q = 0, uint16_t gType = 0) :
		bLimit(l), bPub(p), pid(parent), gameType(gType), isp(sp),  password(pwd),name(n) , hello(h){
	}

	virtual void marshal(sox::Pack &p) const {
		p << name << bLimit << bPub << hello << pid << tid << password << gameType;
		p.push_uint16(qc);
		p.push_uint16(isp);
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p>> name >> bLimit >> bPub >> hello >> pid >> tid >> password >> gameType;
		qc = (VoiceQC)p.pop_uint16();
		isp = (ISPType)p.pop_uint16();
	}

};

struct PCreateChannelRes : public protocol::link::PAddSessRes {
	enum {uri = 1 << 8 | protocol::SDB_SVID};
};


struct PCreateChannelOverFlow : public sox::Marshallable {
	enum {uri = (1 << 8 | protocol::SDB_SVID)};
	uint32_t max;
	virtual void marshal(sox::Pack &p) const {
		p << max;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> max;
	}
};

struct PDismissChannel : public PSessionSid {
	enum {uri = (4 << 8 | protocol::SDB_SVID)};
};

struct PDismissChannelRes : public sox::Voidmable {
	enum {uri = (4 << 8 | protocol::SDB_SVID)};
};

struct PDestroySession: public sox::Marshallable {
	enum {uri = (5 << 8 | protocol::SDB_SVID)};
    uint32_t sid_;
    uint32_t ticks_;

	virtual void marshal(sox::Pack &p) const {
		p << sid_ << ticks_;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> sid_ >> ticks_;
	}
};

struct PNotifySessionManagerSessionDestroy: public sox::Marshallable {
	enum {uri = (6 << 8 | protocol::SDB_SVID)};
    uint32_t sid_;
    uint64_t node_id_;

	virtual void marshal(sox::Pack &p) const {
		p << sid_ << node_id_;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> sid_ >> node_id_;
	}
};


}
}
#endif /*PSMANAGER_H_*/
