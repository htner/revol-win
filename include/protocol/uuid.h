#ifndef REX_UUID
#define REX_UUID
#ifndef WIN32
#include <uuid/uuid.h>
typedef uuid_t muuid_t;
#else
// for marshal
typedef unsigned char muuid_t[16];
#endif
#include "common/packet.h"

namespace core{
struct MUUID: public sox::Marshallable{
	muuid_t id;
	MUUID(muuid_t u){
#ifdef WIN32
// only for compile
		memcpy(id, u, sizeof(muuid_t));
#else
		uuid_copy(id, u);
#endif
	}
	MUUID(){
		memset(id, 0, sizeof(muuid_t));
	}

	bool operator== (const MUUID &u2) const{
#ifdef WIN32
// only for client
		return memcmp(id, u2.id, sizeof(muuid_t)) == 0;
#else
		return uuid_compare(id, u2.id) == 0;
#endif
	}

	bool operator< (const MUUID &u2) const{
#ifdef WIN32
// only for client 
		return memcmp(id, u2.id, sizeof(char)) < 0;
#else
		return uuid_compare(id, u2.id) < 0;
#endif
	}

	MUUID &operator = (const MUUID &u2){
#ifdef WIN32
		memcpy(id, u2.id, sizeof(muuid_t));
		return *this;
#else
		uuid_copy(id, u2.id);
		return *this;
#endif
	}

	void marshal(sox::Pack &pk) const{
		pk.push(id, sizeof(muuid_t));
	}

	void unmarshal(const sox::Unpack &up){
		const char *str = up.pop_fetch_ptr(sizeof(muuid_t));
		memcpy(id, str, sizeof(muuid_t));
	}

	std::string toString(){
#ifdef WIN32
		return "";
#else
		char up[37];
		uuid_unparse(id, up);
		return std::string(up, strlen(up));
#endif
	}
};
}

#endif
