#include "SInfoFront.h"


namespace session {

BEGIN_REQUEST_MAP_EXT(SInfoFront)
/*
REQUEST_HANDLER(XCast<protocol::login::PExchangeKeyRes>, OnExchangeKeyRes)
REQUEST_HANDLER(XCast<protocol::login::PPlus>, OnUdpKeepAliveRes)
REQUEST_HANDLER(XCast<protocol::session::PTcpPingRes>,OnTcpKeepAliveRes)
CHAIN_REQUEST_MAP_PTR_MEMBER(conn_manager_)
*/
END_REQUEST_MAP()


SInfoFront::SInfoFront() {
}

SInfoFront::~SInfoFront() {
}

void SInfoFront::onclose(int err, nproto::connection* c) {
}

void SInfoFront::oncreate(nproto::connection* c) {
}

void SInfoFront::start(const std::string& ip, uint16_t port) {
	if(tcp_socket_main_){
		tcp_socket_main_->close_conn();
		tcp_socket_main_->SetProtoHandler(NULL);
		tcp_socket_main_ = NULL;
	}
	
	tcp_socket_main_ = nsox::nu_create_object<CEncryTcpSocket, nsox::nu_multi_thread_model>::create_inst();
	tcp_socket_main_->Create();
	tcp_socket_main_->SetProtoHandler(this);
	tcp_socket_main_->AsynConnect(ip_.c_str(), port_);

	int size = 100 * 1024;
	tcp_socket_->SetSockOpt(SO_SNDBUF, &size, sizeof(size));
}

}