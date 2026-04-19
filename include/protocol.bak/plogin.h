#ifndef PLOGIN_PROTOCOL_H_
#define PLOGIN_PROTOCOL_H_
#include "common/core/base_svid.h"
#include "common/packet.h"
#include "protocol/const.h"
#include <vector>


namespace protocol {
namespace login {

struct PRouteLinkD : public sox::Marshallable {
	enum {uri = protocol::LBS_SVID};
	uint32_t proto_version;
	uint32_t client_version;
	ISPType type;
		
	virtual void marshal(sox::Pack &pk) const {
		pk << proto_version << client_version;
		pk.push_uint16(type);
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> proto_version >> client_version;
		type = (ISPType)up.pop_uint16();
	}
};

struct POldLinkD : public sox::Marshallable {
	enum {uri = (1 << 8) | protocol::LBS_SVID};
		
	virtual void marshal(sox::Pack &pk) const {
	}
	virtual void unmarshal(const sox::Unpack &up) {
	}
};



struct PRouteLinkDRes : public sox::Marshallable {
	enum {uri = protocol::LBS_SVID};
	std::string ip;
	std::vector<uint16_t> ports;
		
	virtual void marshal(sox::Pack &pk) const {
		pk << ip;
		sox::marshal_container(pk, ports);
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> ip;
		sox::unmarshal_container(up, std::back_inserter(ports));
	}
};

struct PExchangeKey : public sox::Marshallable {
	enum {uri = (1 << 8) | protocol::LINKD_SVID};
	std::string publicKey;
	std::string e;
	virtual void marshal(sox::Pack &pk) const {
		pk << publicKey << e;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> publicKey >> e;
	}
};

struct PExchangeKeyRes : public sox::Marshallable {
	enum {uri = (1 << 8) | protocol::LINKD_SVID};
	std::string encSessionKey;

	virtual void marshal(sox::Pack &pk) const {
		pk << encSessionKey;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> encSessionKey;
	}

};

struct PLogin : public sox::Marshallable {
	enum {uri = (2 << 8) | protocol::LINKD_SVID};
	bool anonymous;
	std::string sha1Pass;
	std::string device_id;
	std::string client_info;
	uint32_t client_version;

	PLogin() {
	}
	PLogin(bool ba, const std::string &p, uint32_t ver) :
		anonymous(ba), sha1Pass(p), client_version(0){
	}
	virtual void marshal(sox::Pack &pk) const {
		pk << anonymous << sha1Pass << client_version << client_info << device_id;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> anonymous >> sha1Pass >> client_version >> client_info >> device_id;
	}
};

struct PLoginRes : public sox::Marshallable {
	enum {uri = (2 << 8) | protocol::LINKD_SVID};
	uint32_t 		res;
	uint32_t 		uid;
	std::string account;
	PLoginRes() {
	}
	
	PLoginRes(uint32_t r, uint32_t u, const std::string &a) :
		res(r), uid(u), account(a) {
	}
	virtual void marshal(sox::Pack &pk) const {
		pk << res << uid << account;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> res >> uid >> account;
	}
};

struct PKickOff : public sox::Voidmable {
	enum {uri = (5 << 8)|protocol::LINKD_SVID};
};

struct PLogout : public sox::Voidmable {
	enum {uri = (4 << 8) | protocol::LINKD_SVID};
};

struct PCheckVersion: public sox::Marshallable{
	enum {uri = (10 << 8 | protocol::LINKD_SVID)};
	uint32_t version;
	virtual void marshal(sox::Pack &pk) const {
		pk << version;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> version;
	}
};

struct PCheckVersionRes: public sox::Voidmable{
	enum {uri = (10 << 8 | protocol::LINKD_SVID)};
	
};

struct PServerShutDown: public sox::Marshallable{
	enum{uri = (11 << 8 | protocol::LINKD_SVID)};
	std::string word;
	virtual void marshal(sox::Pack &pk) const {
		pk << word;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> word;
	}
};

struct PPlus: public sox::Voidmable{
	enum{uri = (12 << 8 | protocol::LINKD_SVID)};
	uint32_t stampc;
	uint32_t stamps;
	virtual void marshal(sox::Pack &pk) const {
		pk << stampc << stamps;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> stampc >> stamps;
	}
};

struct PEchoLbsSession: public sox::Voidmable{
	enum{uri = (13 << 8 | protocol::LINKD_SVID)};
};

struct UserLossReportL : public sox::Marshallable {
	enum{uri = (13 << 8 | protocol::LINKD_SVID)};
	std::string uid;
	uint32_t sid,loss,total,sip;
	uint16_t sport;
	virtual void marshal(sox::Pack &pk) const {  
		pk << uid << sid << loss << total << sip << sport;
	}
	virtual void unmarshal(const sox::Unpack &up) { 
		up >> uid >> sid >> loss >> total >> sip >> sport;
	}
};


struct PServerChange : public sox::Marshallable {

		enum{uri = (14 << 8 | protocol::LINKD_SVID)};
		std::string balance_ip;
		virtual void marshal(sox::Pack &pk) const {  
				pk << balance_ip;
		}
		virtual void unmarshal(const sox::Unpack &up) { 
				up >> balance_ip;
		}
};

struct PHttpProxy: public sox::Marshallable{
	enum{uri = (15 << 8 | protocol::LINKD_SVID)};
	std::string query;
	uint32_t proxyId;
	virtual void marshal(sox::Pack &pk) const {  
		pk << query << proxyId;
	}
	virtual void unmarshal(const sox::Unpack &up) { 
		up >> query >> proxyId;
	}
};

struct PHttpProxyRes: public sox::Marshallable{
	enum{uri = (15 << 8 | protocol::LINKD_SVID)};
	std::string httpData;
	uint32_t proxyId;
	virtual void marshal(sox::Pack &pk) const {  
		pk << httpData << proxyId;
	}
	virtual void unmarshal(const sox::Unpack &up) { 
		up >> httpData >> proxyId;
	}
};

struct PHttpProxyEnd: public sox::Voidmable{
	enum{uri = (16 << 8 | protocol::LINKD_SVID)};
};

struct PClientHttpProxyEnd: public sox::Marshallable{
	enum{uri = (16 << 8 | protocol::LINKD_SVID)};
	uint32_t proxyId;
	virtual void marshal(sox::Pack &pk) const {  
		pk << proxyId;
	}
	virtual void unmarshal(const sox::Unpack &up) { 
		up >> proxyId;
	}
};

struct PClientDcProxy: public sox::Marshallable{
        enum{uri = (17 << 8 | protocol::LINKD_SVID)};
		uint32_t idd;
        std::string method;
        std::string params;
        std::string postdata;
        virtual void marshal(sox::Pack &pk) const {
                pk << idd << method << params << postdata;
        }
        virtual void unmarshal(const sox::Unpack &up) {
                up >> idd >> method >> params >> postdata;
        }
};

struct PClientDcProxyRes: public sox::Marshallable{
        enum{uri = (18 << 8 | protocol::LINKD_SVID)};
		uint32_t code;
		uint32_t idd;
        std::string method;
        std::string retStr;
        virtual void marshal(sox::Pack &pk) const {
                pk << idd << code << method << retStr;
        }
        virtual void unmarshal(const sox::Unpack &up) {
                up >> idd >> code >> method >> retStr;
        }
};


struct PLoginByAccount : public PLogin {
	enum {uri = (19 << 8) | protocol::LINKD_SVID};
	std::string account;
	PLoginByAccount() {
	}
	PLoginByAccount(const std::string &a, bool ba, const std::string &p, uint32_t v) :
		PLogin(ba, p, v), account(a){
	}
	virtual void marshal(sox::Pack &pk) const {
		PLogin::marshal(pk);
		pk << account;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		PLogin::unmarshal(up);
		up >> account;
	}
};


struct PLoginByUid : public PLogin {
	enum {uri = (20 << 8) | protocol::LINKD_SVID};
	uint64_t uid;
	PLoginByUid() {
	}
	PLoginByUid(uint32_t u, bool ba, const std::string &p, uint32_t v) :
		PLogin(ba, p, v), uid(u){
	}
	virtual void marshal(sox::Pack &pk) const {
		PLogin::marshal(pk);
		pk << uid;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		PLogin::unmarshal(up);
		up >> uid;
	}
};



struct PSessKey: public sox::Marshallable{
	enum{uri = (200 << 8 | protocol::LINKD_SVID)};
	std::string sesskey;
	virtual void marshal(sox::Pack &pk) const {  
		pk << sesskey;
	}
	virtual void unmarshal(const sox::Unpack &up) { 
		up >> sesskey;
	}
};

struct PLoginBase : public sox::Marshallable {
	enum {uri = (201 << 8) | protocol::LINKD_SVID};
	bool anonymous;
	std::string sha1Pass;
	std::string device_id;
	std::string client_info;
	uint32_t client_version;

	PLoginBase() {
	}
	PLoginBase(bool ba, const std::string &p, uint32_t ver) :
		anonymous(ba), sha1Pass(p), client_version(0){
	}
	virtual void marshal(sox::Pack &pk) const {
		pk << anonymous << sha1Pass << client_version << client_info << device_id;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		up >> anonymous >> sha1Pass >> client_version >> client_info >> device_id;
	}
};


struct PLogin2ByAccount : public PLoginBase {
	enum {uri = (202 << 8) | protocol::LINKD_SVID};
	std::string account;
	PLogin2ByAccount() {
	}
	PLogin2ByAccount(const std::string &a, bool ba, const std::string &p, uint32_t v) :
		PLoginBase(ba, p, v), account(a){
	}
	virtual void marshal(sox::Pack &pk) const {
		PLoginBase::marshal(pk);
		pk << account;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		PLoginBase::unmarshal(up);
		up >> account;
	}
};


struct PLogin2ByUid : public PLoginBase {
	enum {uri = (203 << 8) | protocol::LINKD_SVID};
	uint64_t uid;
	PLogin2ByUid() {
	}
	PLogin2ByUid(uint32_t u, bool ba, const std::string &p, uint32_t v) :
		PLoginBase(ba, p, v), uid(u){
	}
	virtual void marshal(sox::Pack &pk) const {
		PLoginBase::marshal(pk);
		pk << uid;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		PLoginBase::unmarshal(up);
		up >> uid;
	}
};



struct PLogin3ByAccount : public PLoginBase {
	enum {uri = (204 << 8) | protocol::LINKD_SVID};
	std::string account;
	std::string newPasswd;
	PLogin3ByAccount() {
	}
	PLogin3ByAccount(const std::string &a, bool ba, const std::string &p, uint32_t v) :
		PLoginBase(ba, p, v), account(a){
	}
	virtual void marshal(sox::Pack &pk) const {
		PLoginBase::marshal(pk);
		pk << account << newPasswd;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		PLoginBase::unmarshal(up);
		up >> account >> newPasswd;
	}
};


struct PLogin3ByUid : public PLoginBase {
	enum {uri = (205 << 8) | protocol::LINKD_SVID};
	uint64_t uid;
	std::string newPasswd;
	PLogin3ByUid() {
	}
	PLogin3ByUid(uint32_t u, bool ba, const std::string &p, uint32_t v) :
		PLoginBase(ba, p, v), uid(u){
	}
	virtual void marshal(sox::Pack &pk) const {
		PLoginBase::marshal(pk);
		pk << uid << newPasswd;
	}
	virtual void unmarshal(const sox::Unpack &up) {
		PLoginBase::unmarshal(up);
		up >> uid >> newPasswd;
	}
};

struct PLoginByToken : public PLoginBase {
        enum {uri = (206 << 8) | protocol::LINKD_SVID};
        uint64_t uid;
	std::string account;
	std::string kind;
        std::string token;
        PLoginByToken() {
        }
        PLoginByToken(uint32_t u, bool ba, const std::string &p, uint32_t v) :
                PLoginBase(ba, p, v), uid(u){
        }
        virtual void marshal(sox::Pack &pk) const {
                PLoginBase::marshal(pk);
                pk << uid << account << kind << token;
        }
        virtual void unmarshal(const sox::Unpack &up) {
                PLoginBase::unmarshal(up);
                up >> uid >> account >> kind >> token;
        }
};

struct PGetToken : public sox::Marshallable {
        enum{uri = (207 << 8 | protocol::LINKD_SVID)};
        uint32_t type;
        std::string kind;
        virtual void marshal(sox::Pack &pk) const {
                pk << type << kind;
        }
        virtual void unmarshal(const sox::Unpack &up) {
                up >> type >> kind;
        }
};

struct PGetTokenRes : public sox::Marshallable {
        enum{uri = (208 << 8 | protocol::LINKD_SVID)};
        uint32_t type;
        std::string kind;
        std::string token;
        virtual void marshal(sox::Pack &pk) const {
                pk << type << kind << token;
        }
        virtual void unmarshal(const sox::Unpack &up) {
                up >> type >> kind >> token;
        }
};

struct PServerPing: public sox::Marshallable {
     enum {uri = (351 << 8) | protocol::LINKD_SVID};
     uint64_t haomiao;
 
     virtual void marshal(sox::Pack &pk) const{
         pk << haomiao;
     }
 
     virtual void unmarshal(const sox::Unpack &up){
         up >> haomiao;
     }
 };
 
 
 
 struct PServerPingRes: public sox::Marshallable {
     enum {uri = (352 << 8) | protocol::LINKD_SVID};
     uint64_t haomiao;
 
     virtual void marshal(sox::Pack &pk) const{
         pk << haomiao;
     }
 
     virtual void unmarshal(const sox::Unpack &up){
         up >> haomiao;
     }

};


}
}
#endif /*PLOGIN_H_*/
