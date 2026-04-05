#include "login.h"
#include "helper/utf8to.h"
#include <common/res_code.h>
#include <common/core/request.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <common/version.h>
#include <location/default.h>
#include<algorithm>

#include "helper/ufile.h"
#include "helper/utf8to.h"
#include "helper/helper.h"

#include "Gateway.h"
#include "Login.h"
#include "SessionStreamManager.h"


static const char rnd_seed[] = "Our product aim to make world better !!!";


namespace xproto
{

	BEGIN_REQUEST_MAP_EXT(GatewayLink)		
		REQUEST_HANDLER(XCast<protocol::gateway::LoginResPacket>, OnLoginRes)
		REQUEST_HANDLER(XCast<protocol::gateway::GetUserMessageInfoResPacket>, OnGetUserMessage)
		REQUEST_HANDLER(XCast<protocol::gateway::UserMessageInfoPacket>, OnUserMessage)
		REQUEST_HANDLER(XCast<protocol::gateway::PingResPacket>, OnPingRes)
		CHAIN_REQUEST_MAP_PTR_MEMBER(SessionStreamManager::Inst())
	END_REQUEST_MAP()

	GatewayLink::GatewayLink() {
	}

	GatewayLink::~GatewayLink() {
	}

	void GatewayLink::Start(const char* ip, unsigned int port) {
		gateway_timer_.init(this, &GatewayLink::OnPingTimeout);
		gateway_timer_.start(3000);
		if (socket_list_.size()) {
			return;
		}
		nsox::nu_auto_ptr<CGatewaySocket> socket = nsox::nu_create_object<CGatewaySocket>::create_inst();
		socket->Create();
		socket->SetProtoHandler(this);
		socket->AsynConnect(ip, port);
		socket_list_.push_back(socket);
	}

	void GatewayLink::Cancel() {
	}

	void GatewayLink::onclose(int err, nproto::connection* c) {
		((CGatewaySocket*)c)->OnClose();

	}

	void GatewayLink::oncreate(nproto::connection* c) {
		XCast<protocol::gateway::LoginPacket> login_packet;
		login_packet.client_version_ = 1;
		login_packet.protocol_version_ = 2;
		login_packet.type_ = 2;
		login_packet.id_ = 10001;
		login_packet.dev_ = 1;
		login_packet.random_ = rand();
		login_packet.token_ = "rex-help";
		write_request(c, login_packet);
	}

	void GatewayLink::OnLoginRes(XCast<protocol::gateway::LoginResPacket>& cmd, nproto::connection* c) {
		XCast<protocol::gateway::GetUserMessageInfoPacket> get_message;
		write_request(c, get_message);
	}

	void GatewayLink::OnPingRes(XCast<protocol::gateway::PingResPacket>& cmd, nproto::connection* c) {
	}

	void GatewayLink::OnPingTimeout() {
		if (socket_list_.empty()) {
			Start("192.168.50.81", 10009);
			return;
		}
		XCast<protocol::gateway::PingPacket> ping_packet;
		
		std::vector<nsox::nu_auto_ptr<CGatewaySocket> >::iterator it;
		for (it = socket_list_.begin(); it != socket_list_.end();)
		{
			write_request(*it, ping_packet);
			if ((*it)->OutOfTime()) {
				it = socket_list_.erase(it);
			} else {
				it++;
			}
		}
	}

	void GatewayLink::OnGetUserMessage(XCast<protocol::gateway::GetUserMessageInfoResPacket>& cmd, nproto::connection* c) {
	//	assert(0);
	}
	
	void GatewayLink::OnUserMessage(XCast<protocol::gateway::UserMessageInfoPacket>& cmd, nproto::connection* c) {
	//	assert(0);
	}


}