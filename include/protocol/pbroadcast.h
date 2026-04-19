#ifndef PBROADCAST_H
#define PBROADCAST_H

#include "common/packet.h"
#include <vector>
#include <iterator>
#include <common/core/base_svid.h>


namespace protocol{
namespace broadcast{

struct PBroadcastOnline : public sox::Marshallable {
	enum {uri = (1 << 8 | protocol::BROADCAST_SVID)};
	std::string 		info;
	virtual void marshal(sox::Pack &p) const {
		p.push_varstr32(info);
	}
	virtual void unmarshal(const sox::Unpack& p) {
		info = p.pop_varstr32();
	}
};

struct PBroadcastByUids : public PBroadcastOnline{
	enum {uri = (2 << 8 | protocol::BROADCAST_SVID)};
	
	std::vector<uint32_t> 	uids;

	virtual void marshal(sox::Pack &p) const {
		PBroadcastOnline::marshal(p);
		marshal_container(p, uids);
	}
	virtual void unmarshal(const sox::Unpack &p) {
		PBroadcastOnline::unmarshal(p);
		unmarshal_container(p, std::back_inserter(uids));
	}
};

struct PBroadcastByServer : public sox::Marshallable {
	enum {uri = (3 << 8 | protocol::BROADCAST_SVID)};
	uint32_t serverid;
	std::vector<uint32_t> uids;
	virtual void marshal(sox::Pack &p) const {
		p << serverid;
		marshal_container(p, uids);
	}
	virtual void unmarshal(const sox::Unpack& p) {
		p >> serverid;
		unmarshal_container(p, std::back_inserter(uids));
	}
};

struct PBroadcastByServerIds : public PBroadcastOnline {
	enum {uri = (4 << 8 | protocol::BROADCAST_SVID)};
	std::vector<PBroadcastByServer> 	servers;
	virtual void marshal(sox::Pack &p) const {
		PBroadcastOnline::marshal(p);
		marshal_container(p, servers);
	}
	virtual void unmarshal(const sox::Unpack& p) {
		PBroadcastOnline::unmarshal(p);
		unmarshal_container(p, std::back_inserter(servers));
	}
};

struct PBroadcastByDc : public sox::Marshallable{
	enum {uri = (5 << 8 | protocol::BROADCAST_SVID)};
	std::string function;
	virtual void marshal(sox::Pack &p) const {
		p << function;
	}
	virtual void unmarshal(const sox::Unpack &p) {
		p >> function;
	}
};

}
}

#endif
