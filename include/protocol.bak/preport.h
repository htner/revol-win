#ifndef PREPORT_H_
#define PREPORT_H_

#include "common/core/base_svid.h"
#include "common/packet.h"
#include <vector>
#include <string>

namespace protocol {
	namespace preport {

		struct PReportData : public sox::Marshallable {
			enum {uri = (1 << 8 | protocol::SESSION_SVID)};
			uint32_t uid;
			uint32_t id;
			uint8_t type;
			std::vector<std::string> infos;
			virtual void marshal(sox::Pack &pk) const {
				pk << uid << type << id << infos;
			}
			virtual void unmarshal(const sox::Unpack &upk) {
				upk >> uid >> type >> id >> infos;
			}
		};

		struct PReportDataRes : public sox::Marshallable {
			enum {uri = (2 << 8 | protocol::SESSION_SVID)};
			uint32_t id;
			virtual void marshal(sox::Pack &pk) const {
				pk << id;
			}
			virtual void unmarshal(const sox::Unpack &upk) {
				upk >> id;
			}
		};

	
	}
}

#endif // 
