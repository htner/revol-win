#ifndef PVOTE_H_
#define PVOTE_H_

#include "common/core/base_svid.h"
#include "common/packet.h"

namespace protocol {
	namespace plugin {

		struct PFeatureBaseRes : public sox::Marshallable {
				uint32_t type;
				virtual void marshal(sox::Pack &pk) const {
					pk << type;
				}

				virtual void unmarshal(const sox::Unpack &upk) {
					upk >> type;
				}
		};

		struct PFeatureRequest : public sox::Marshallable {
			enum {uri = (1 << 8 | protocol::PLUGIN_SVID) };

			std::string data;

			virtual void marshal(sox::Pack &pk) const {
				pk.push_varstr32(data.data(),data.size());
			}

			virtual void unmarshal(const sox::Unpack &upk) {
				data = upk.pop_varstr32();
			}
		};

		struct PFeatureResponse : public PFeatureBaseRes {
			enum {uri = (2 << 8 | protocol::PLUGIN_SVID) };

			std::string data;

			virtual void marshal(sox::Pack &pk) const {
				PFeatureBaseRes::marshal(pk);
				pk.push_varstr32(data.data(),data.size());
			}

			virtual void unmarshal(const sox::Unpack &upk) {
				PFeatureBaseRes::unmarshal(upk);
				data = upk.pop_varstr32();
			}
		};

		struct PFeatureBroadCast : public PFeatureBaseRes {
			enum {uri = (3 << 8 | protocol::PLUGIN_SVID) };
			std::string desc;

			virtual void marshal(sox::Pack &pk) const {
				PFeatureBaseRes::marshal(pk);
				pk << desc;
				//pk.push_varstr32(desc.data(),data.size());
			}

			virtual void unmarshal(const sox::Unpack &upk) {
				PFeatureBaseRes::unmarshal(upk);
				//data = upk.pop_varstr32();
				upk >> desc;
			}
		};
	}
}

#endif // PVOTE_H_
