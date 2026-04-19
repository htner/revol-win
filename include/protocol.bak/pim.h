#ifndef PIM_H_
#define PIM_H_

#include "common/core/base_svid.h"
#include "common/packet.h"
#include "../protocol/pzipbase.h"

namespace protocol {
namespace im {
    struct PImRequest : public sox::Marshallable {
        enum {uri = (1 << 8 | protocol::IM_SVID) };

        std::string data;

        virtual void marshal(sox::Pack &pk) const {
            pk.push_varstr32(data.data(), data.size());
        }

        virtual void unmarshal(const sox::Unpack &upk) {
            data = upk.pop_varstr32();
        }
    };

    struct PImResponse : public sox::Marshallable {
        enum {uri = (2 << 8 | protocol::IM_SVID) };
 
        std::string data;

        virtual void marshal(sox::Pack &pk) const {
            pk.push_varstr32(data.data(), data.size());
        }

        virtual void unmarshal(const sox::Unpack &upk) {
            data = upk.pop_varstr32();
        }
    };

    struct PZImRequest : public protocol::session::PZipBase<PImRequest> {
        enum {uri = (3 << 8 | protocol::IM_SVID)};
    };

    struct PZImResponse : public  protocol::session::PZipBase<PImResponse> {
        enum {uri = (4 << 8 | protocol::IM_SVID)};
    };
}
}

#endif // PIM_H_
