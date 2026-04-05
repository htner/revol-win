#pragma once
#include "xconnpoint.h"
#include <common/nsox/default_proto_handler.h>
#include <common/nsox/nuautoptr.h>
#include <common/nsox/proto_cast.h>
#include <protocol/const.h>
#include <protocol/plogin.h>
#include <protocol/pim.h>
#include "timer/ExtraTimer.h"
#include "timer/TimerWrap.h"
#include "common/xstring.h"
//#include "protocol/psession.h"

namespace session
{

class SInfoFront : public nproto::default_proto_handler {
public:
	SInfoFront();
	virtual ~SInfoFront();

	DECLARE_REQUEST_MAP

public:
	virtual void onclose(int err, nproto::connection* c);
	virtual void oncreate(nproto::connection* c);

public:
	void start(const std::string& ip, uint16_t);

private:
	nsox::nu_auto_ptr<CEncryTcpSocket> tcp_socket_main_;
}

}

