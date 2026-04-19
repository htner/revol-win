#ifndef PFORWARD_H_
#define PFORWARD_H_

#include "common/core/base_svid.h"
#include "common/packet.h"

namespace protocol {
namespace link {
    struct PForward : public sox::Marshallable {
        enum {uri = (100 << 8 | protocol::LINKD_SVID)};

        uint32_t uid;
        std::string cmd;

        virtual void marshal(sox::Pack &pk) const {
            pk << uid << cmd;
        }

        virtual void unmarshal(const sox::Unpack &upk) {
            upk >> uid >> cmd;
        }
    };

    struct PReply : public sox::Marshallable {
    	enum {uri = (101 << 8 | protocol::LINKD_SVID)};

    	uint32_t uid;
    	std::string cmd;

    	virtual void marshal(sox::Pack &pk) const {
    		pk << uid << cmd;
    	}

    	virtual void unmarshal(const sox::Unpack &upk) {
    		upk >> uid >> cmd;
    	}
    };

    struct PForward2 : public sox::Marshallable {
        enum {uri = (102 << 8 | protocol::LINKD_SVID)};

        uint32_t sid;
        std::string cmd;

        virtual void marshal(sox::Pack &pk) const {
            pk << cmd << sid;
        }

        virtual void unmarshal(const sox::Unpack &upk) {
            upk >> cmd >> sid;
        }
    };

    struct PServerChoose : public sox::Marshallable {
        enum {uri = (103 << 8 | protocol::LINKD_SVID)};

        protocol::svid svid;

        virtual void marshal(sox::Pack &pk) const {
            pk.push_uint8(svid);
        }

        virtual void unmarshal(const sox::Unpack &upk) {
            svid = (protocol::svid) upk.pop_uint8();
        }
    };

    struct PServerChooseRes : public sox::Marshallable {
        enum {uri = (103 << 8 | protocol::LINKD_SVID)};

        protocol::svid svid;
        uint32_t sid;

        virtual void marshal(sox::Pack &pk) const {
            pk.push_uint8(svid);
            pk.push_uint32(sid);
        }

        virtual void unmarshal(const sox::Unpack &upk) {
            svid = (protocol::svid) upk.pop_uint8();
            sid = upk.pop_uint32();
        }
    };

}}

#endif

