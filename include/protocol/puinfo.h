#define PUINFO_PROTOCOL_H
#ifndef PUINFO_PROTOCOL_H
#define PUINFO_PROTOCOL_H
#include "common/core/base_svid.h"
#include "common/iproperty.h"
namespace protocol{
	namespace uinfo{
		struct GetUInfoByAccount : public sox::Marshallable
		{
			enum { uri = (1 << 8) | UINFO_SVID};
			std::string account;
			uint32_t ver;

			GetUInfoByAccount(){}
			GetUInfoByAccount(const std::string &u, uint32_t _v = 0) :account(u), ver(_v) { }


			virtual void marshal(sox::Pack &p) const {p << ver << account;}
			virtual void unmarshal(const sox::Unpack &p) { p >> ver >> account;}
			virtual std::ostream &trace(std::ostream &os) const { return os << "ver = " << ver; }
		};

		struct RepGetUInfo : public sox::Properties
		{
			enum { uri = (1 << 8) | UINFO_SVID };
		};

		struct UpdateUInfo : public sox::Properties
		{
			enum { uri = (2 << 8) | UINFO_SVID };
		};

		struct RepUpdateUInfo : public UpdateUInfo{
		};

		struct PReportSpy: public sox::Marshallable{
			enum{uri = (3 << 8 | protocol::UINFO_SVID)};
			std::string mac;
			std::string localip;
			bool hasUT;
			bool hasIS;

			virtual void marshal(sox::Pack &pk) const{
				pk << mac << localip << (uint16_t)hasUT << (uint16_t)hasIS;
			}

			virtual void unmarshal(const sox::Unpack &up){
				up >> mac >> localip;
				hasUT = up.pop_uint16() != 0;
				hasIS = up.pop_uint16() != 0;
			}
		};

		struct GetUInfoByUid : public sox::Marshallable
		{
			enum { uri = (4 << 8) | UINFO_SVID};
			GetUInfoByUid(){}
			GetUInfoByUid(uint32_t u, uint32_t _v = 0) :uid(u), ver(_v) { }

			uint32_t uid;
			uint32_t ver;

			virtual void marshal(sox::Pack &p) const {p << ver << uid;}
			virtual void unmarshal(const sox::Unpack &p) { p >> ver >> uid;}
			virtual std::ostream &trace(std::ostream &os) const { return os << "ver = " << ver; }
		};
	}
	namespace udb{
		struct PQueryEmail : public sox::Marshallable {
			enum {uri = (6 << 8) | protocol::UDB_SVID};
			
			std::string account;
			std::string sha1Pass;
			
			virtual void marshal(sox::Pack &pk) const {
				pk << account << sha1Pass;
			}
			virtual void unmarshal(const sox::Unpack &up) {
				up >> account >> sha1Pass;
			}
		};

		struct PQueryEmailRes: public sox::Marshallable {
			enum {uri = (6 << 8) | protocol::UDB_SVID};
			std::string email;
			
			virtual void marshal(sox::Pack &pk) const {
				pk << email;
			}
			virtual void unmarshal(const sox::Unpack &up) {
				up >> email;
			}
		};

	}
}
#endif /*PUINFO_*/
