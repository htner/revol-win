#include "ProtoLink.h"
//#include "Gateway.h"
#include "Login.h"
#include "SessionStreamManager.h"

#include <common/core/request.h>
#include "client/proto/iconfig.h"
#include "client/proto/proto_obj_name.h"
#include "common/xstring.h"
#include "helper/ufile.h"
#include "helper/utf8to.h"
#include "client/proto/ismanager.h"

#include "SessionManager.h"

SessionStreamManager* SessionStreamManager::Inst() {
	static SessionStreamManager manager;
	return &manager;
}

int SessionStreamManager::GetStreamAddr(uint64_t stream_id) {

	return 1;
}
