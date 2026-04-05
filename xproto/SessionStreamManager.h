#pragma once
#include "xconnpoint.h"
#include <common/nsox/default_proto_handler.h>
#include <common/nsox/nuautoptr.h>
#include <common/nsox/proto_cast.h>

#include "timer/ExtraTimer.h"
#include "timer/TimerWrap.h"
#include "common/xstring.h"
//#include "protocol/psstream.h"

class SessionStreamManager {
public:
	BEGIN_REQUEST_MAP_EXT(SessionStreamManager)

	END_REQUEST_MAP()

	static SessionStreamManager* Inst();
	int GetStreamAddr(uint64_t stream_id);
private:
	uint64_t stream_id_;
};