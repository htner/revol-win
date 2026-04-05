#include "front_connection.h"
#include "sessionimp.h"
#include "helper/helper.h"
#include "helper/ufile.h"

#define CHECK_SEQ() if (CheckHeadSequence(data, len) == false) {return false;};
#define CHECK_CONNECTION(c) CheckConnection(c);
#define TCP_PING_TIME 10 * 1000
#define UDP_PING_TIME 3 * 1000

namespace protocol {
namespace session {

BEGIN_REQUEST_MAP_EXT(FrontConnection)
CHECK_CONNECTION(c)
REQUEST_HANDLER(XCast<protocol::login::PExchangeKeyRes>, OnExchangeKeyRes)
REQUEST_HANDLER(XCast<protocol::login::PPlus>, OnUdpKeepAliveRes)
REQUEST_HANDLER(XCast<protocol::session::PTcpPingRes>,OnTcpKeepAliveRes)
CHAIN_REQUEST_MAP_PTR_MEMBER(conn_manager_)
END_REQUEST_MAP()



BEGIN_REQUEST_MAP_EXT(FrontConnectionManager)
CHECK_SEQ()
//REQUEST_HANDLER(XCast<protocol::login::PExchangeKeyRes>, OnExchangeKeyRes)
CHAIN_REQUEST_MAP_PTR_MEMBER(session_imp_)
END_REQUEST_MAP()

FrontConnection::FrontConnection() {
	//InitializeCriticalSection(&__cs);
	//CAutoLock lock(&__cs);
	tcp_status_ = FS_INIT;
	udp_status_ = FS_INIT;
	tcp_socket_ = NULL;
	udp_socket_ = NULL;
	tcp_ping_ = 10000;
	udp_ping_ = 10000;
	udp_timer_.init(this, &FrontConnection::UdpKeepAlive);
	tcp_timer_.init(this, &FrontConnection::TcpKeepAlive);
	last_tcp_time_ = 0;
	last_udp_time_ = 0;
	message_status_ = 0;
	
}

void FrontConnection::SetManager(FrontConnectionManager* manager) {
	conn_manager_ = manager;
}

void FrontConnection::SetIpPort(const char* ip, uint16_t port) {
	ip_ = ip;
	port_ = port;
}

FrontConnection::~FrontConnection() {
	Close();
	//DeleteCriticalSection(&__cs);
}

bool g_is_udp = false;

bool FrontConnection::CheckConnection(nproto::connection* c) {
	if (tcp_socket_ == c) {
		last_tcp_time_ = timeGetTime();
		g_is_udp = false;
	} else if (udp_socket_ == c) {
		last_udp_time_ = timeGetTime();
		g_is_udp = true;
	} else {
		assert(0);
	}
	return true;
}

void FrontConnection::StartTcp() {
	//CAutoLock lock(&__cs);
	if(tcp_socket_){
		tcp_socket_->close_conn();
		tcp_socket_->SetProtoHandler(NULL);
		tcp_socket_ = NULL;
	}
	
	tcp_socket_ = nsox::nu_create_object<CEncryTcpSocket, nsox::nu_multi_thread_model>::create_inst();
	tcp_socket_->Create();
	tcp_socket_->SetProtoHandler(this);
	tcp_socket_->AsynConnect(ip_.c_str(), port_);

	struct linger so_linger;
	so_linger.l_onoff = TRUE;
	so_linger.l_linger = 5;
	tcp_socket_->SetSockOpt(SO_LINGER, &so_linger,sizeof(so_linger));

	 
	int size = 100 * 1024;
	tcp_socket_->SetSockOpt(SO_SNDBUF, &size,sizeof(size));

	size =  2000;
	tcp_socket_->SetSockOpt(SO_RCVBUF, &size,sizeof(size));

	
	tcp_status_ = FS_TCP_INIT;
}

void FrontConnection::StartUdp() {

	xstring xx = getAppPath();
	xx += _T("\\rcEnv.ini");
	bool udp = GetPrivateProfileIntW(L"xproto",	L"udp", 1, xx.c_str());
	if (udp == 0) {
		return;
	}
	if(udp_socket_){
		udp_socket_->close_conn();
		udp_socket_->setProtoHandler(NULL);
		udp_socket_  = NULL;
	}
	udp_socket_ = nsox::nu_create_object<CWsUdpSocket, nsox::nu_multi_thread_model>::create_inst();
	udp_socket_->setProtoHandler(this);
	udp_socket_->create();
	udp_socket_->connect(ip_.c_str(), port_);

	int size = 100 * 1024;
	udp_socket_->SetSockOpt(SO_SNDBUF, &size,sizeof(size));

	size = 100 * 1024;
	udp_socket_->SetSockOpt(SO_RCVBUF, &size,sizeof(size));

	size = 0;
	int size_len = sizeof(size);
	udp_socket_->GetSockOpt(SO_RCVBUF, &size,&size_len);

	udp_timer_.start(UDP_PING_TIME);
	udp_status_ = FS_UDP_INIT;
	udp_status_ = FS_UDP_CONNECT;
	conn_manager_->UdpConnectionReady(this);
	last_udp_time_ = timeGetTime();
}

void FrontConnection::Close() {
	//CAutoLock lock(&__cs);
	if(tcp_socket_){
		tcp_socket_->close_conn();
		tcp_socket_->SetProtoHandler(NULL);
		tcp_socket_ = NULL;
		tcp_status_ = FS_INIT;
	}
	if(udp_socket_){
		udp_socket_->close_conn();
		udp_socket_->setProtoHandler(NULL);
		udp_socket_ = NULL;
		udp_status_ = FS_INIT;
	}
}

bool FrontConnection::Send(nproto::request* c, int URI) {
	if (SendByUdp(c, URI)) {
		return true;
	}
	if (SendByTcp(c, URI)) {
		return true;
	}
	return false;
}

bool FrontConnection::SendByTcp(nproto::request* c, int URI) {
	nsox::nu_auto_ptr<CEncryTcpSocket> tcp_no_delete = tcp_socket_;
	//CAutoLock lock(&__cs);
	if (tcp_socket_) {
		if (tcp_status_ == FS_TCP_LOGIN) {
			nproto::write_request(tcp_socket_,c, URI);
			if (URI == PLeaveSession::uri) {
				tcp_socket_->SendBuffer();
			}
		} else {
			switch(URI) {
				case PRealJoinSession::uri:
				case PRealJoinChannel::uri:
				case PJoinSession::uri:
					if (tcp_status_ == FS_TCP_ENCODE) {
						nproto::write_request(tcp_socket_,c, URI);
					}
					return true;
					break;
				case protocol::login::PExchangeKey::uri: {
					if (tcp_status_ == FS_TCP_CONNECT) {
						nproto::write_request(tcp_socket_,c, URI);
					}
					return true;
				}
				break;
				default:
					break;
			};
		}
	}
	return false;
}

bool FrontConnection::SendByUdp(nproto::request* c, int URI) {
	if (udp_socket_) {
		if (udp_status_ == FS_UDP_LOGIN) {
			nproto::write_request(udp_socket_,c, URI);
			return true;
		} else {
			switch(URI) {
				case PUdpLogin::uri:
					nproto::write_request(udp_socket_,c, URI);
					return true;
				break;
				default:
					break;
			};
		}
	}
	return false;
}

void FrontConnection::TcpKeepAlive() {
	if (timeGetTime() - last_tcp_time_ < 3 * TCP_PING_TIME || true) {
		XCast<protocol::session::PTcpPing> plus;
		plus.stampc = ::timeGetTime();
		plus.uid = conn_manager_->GetSessionImp()->GetUid();
		plus.sid = conn_manager_->GetSessionImp()->GetSessionId();
		nsox::xlog(NsoxDebug, "session ping");
		SendByTcp(&plus, plus.uri);
	} else {
	}
}

void FrontConnection::UdpKeepAlive() {
	if (timeGetTime() - last_udp_time_ < 10 * UDP_PING_TIME) {
		XCast<protocol::session::PUdpPing> ping;
		ping.uid = conn_manager_->GetSessionImp()->GetUid();
		ping.sid = conn_manager_->GetSessionImp()->GetSessionId();
		ping.stampc = timeGetTime();
		SendByUdp(&ping, ping.uri);
	} else {
		XCast<protocol::session::PUdpClose> close;
		SendByTcp(&close, close.uri);
		udp_status_ = FS_INIT;
		udp_timer_.stop();
		udp_ping_ = 10000;
	}
}

bool FrontConnection::OnTcpKeepAliveRes(XCast<protocol::session::PTcpPingRes>& cmd, nproto::connection* c) {
	tcp_ping_ = timeGetTime() - cmd.stampc;
	if (message_status_ == PUserSubcribeMessage::ALL_MESSAGE)
		conn_manager_->GetSessionImp()->SetRtt(GetPing());
	return true;
}

bool FrontConnection::OnUdpKeepAliveRes(XCast<protocol::login::PPlus>& cmd, nproto::connection* c) {
	udp_ping_ =  timeGetTime() - cmd.stampc;
	return true;
}

void FrontConnection::OnExchangeKeyRes(XCast<protocol::login::PExchangeKeyRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		tcp_status_ = FS_TCP_ENCODE;
		std::string rc4;
		conn_manager_->GetSessionImp()->GetRsaKey()->rsaDecodeRc4(cmd.encSessionKey, rc4);
		tcp_socket_->setEncKey((const unsigned char *)rc4.data(), rc4.length());
		conn_manager_->TcpConnectionReady(this);
		tcp_timer_.start(TCP_PING_TIME);
	}else{
		//__handler->onStatus(ISessionHandler::INVALID, this);
	}
}

void FrontConnection::onclose(int err, nproto::connection* c) {
	if (c == tcp_socket_) {
		conn_manager_->TcpConnectionClose(this);
	} else if (c == udp_socket_) {
		conn_manager_->UdpConnectionClose(this);
	}
}

void FrontConnection::oncreate(nproto::connection* c){
	if (c ==tcp_socket_) {
		tcp_status_ = FS_TCP_CONNECT;
		XCast<protocol::login::PExchangeKey> exk;
		conn_manager_->GetSessionImp()->GetRsaKey()->getRSAKeyString(exk.publicKey, exk.e);
		SendByTcp(&exk, exk.uri);
		conn_manager_->GetSessionImp()->SetTime(2);
	} else if (c == udp_socket_) {
		udp_status_ = FS_UDP_CONNECT;
		conn_manager_->UdpConnectionReady(this);
	}
}

void FrontConnection::LoginSuccess() {
	if (tcp_status_ == FS_TCP_ENCODE)
		tcp_status_ = FS_TCP_LOGIN;
	StartUdp();
}

void FrontConnection::RegisterUdpSuccess() {
	if (udp_status_ == FS_UDP_CONNECT)
		udp_status_ = FS_UDP_LOGIN;
}

uint32_t FrontConnection::GetPing() {
	return min(tcp_ping_, udp_ping_);
}

//////////////////////////////////////////////////////////////////////////////////


BEGIN_REQUEST_MAP_EXT(SInfoFront)
//CHECK_CONNECTION(c)
//REQUEST_HANDLER(XCast<protocol::login::PExchangeKeyRes>, OnExchangeKeyRes)
//REQUEST_HANDLER(XCast<protocol::login::PPlus>, OnUdpKeepAliveRes)
//REQUEST_HANDLER(XCast<protocol::session::PTcpPingRes>,OnTcpKeepAliveRes)
CHAIN_REQUEST_MAP_PTR_MEMBER(conn_manager_)
END_REQUEST_MAP()

void SInfoFront::SetManager(FrontConnectionManager* manager) {
	conn_manager_ = manager;
}

SInfoFront::SInfoFront() {
	conn_manager_ = NULL;
}

SInfoFront::~SInfoFront() {
	if (tcp_socket_main_) {
		tcp_socket_main_->close_conn();
		tcp_socket_main_->SetProtoHandler(NULL);
		tcp_socket_main_ = NULL;
	}

	if (tcp_socket_vice_) {
		tcp_socket_vice_->close_conn();
		tcp_socket_vice_->SetProtoHandler(NULL);
		tcp_socket_vice_ = NULL;
	}
}

void SInfoFront::onclose(int err, nproto::connection* c) {
	if (tcp_socket_vice_ == c) {
		tcp_socket_vice_->close_conn();
		tcp_socket_vice_->SetProtoHandler(NULL);
		tcp_socket_vice_ = NULL;
	}
	if (tcp_socket_main_ == c) {
		tcp_socket_main_->close_conn();
		tcp_socket_main_->SetProtoHandler(NULL);
		tcp_socket_main_ = NULL;
	}
	//tcp_socket_main_ = NULL;
}

void SInfoFront::oncreate(nproto::connection* c) {
	//tcp_socket_main_ = c;
}

bool SInfoFront::IsConnAlive() {
	bool ret = false;
	if (tcp_socket_main_) {
		ret = tcp_socket_main_->is_conn_alive();
	}
	if (ret == false) {
		if (tcp_socket_vice_) {
			ret = tcp_socket_vice_->is_conn_alive();
		}
	}
	return ret;
}

bool SInfoFront::IsAllConnAlive() {
	if (tcp_socket_main_ && tcp_socket_vice_) {
			return tcp_socket_main_->is_conn_alive() && tcp_socket_vice_->is_conn_alive();
	}
	return false;
}

void SInfoFront::Start(const std::string& ip, uint16_t port) {
	//if (tcp_socket_main_ && tcp_socket_main_->getp) {
	//}
	if(tcp_socket_main_ == NULL || tcp_socket_vice_ == NULL){
		//tcp_socket_main_->close_conn();
		//tcp_socket_main_->SetProtoHandler(NULL);
		//tcp_socket_main_ = NULL;
		nsox::nu_auto_ptr<CEncryTcpSocket> new_conn = nsox::nu_create_object<CEncryTcpSocket, nsox::nu_multi_thread_model>::create_inst();
		new_conn->Create();
		new_conn->SetProtoHandler(this);
		new_conn->AsynConnect(ip.c_str(), port);

		int size = 100 * 1024;
		new_conn->SetSockOpt(SO_SNDBUF, &size, sizeof(size));
		
		if (tcp_socket_main_ == NULL) {
			tcp_socket_main_ = new_conn;
		} else if (tcp_socket_vice_ == NULL) {
			tcp_socket_vice_ = new_conn;
		}
		new_conn = NULL;
	}
}

bool SInfoFront::Send(nproto::request* c, int URI) {
	if (tcp_socket_main_) {
		nproto::write_request(tcp_socket_main_, c, URI);
	} else if (tcp_socket_vice_) {
		nproto::write_request(tcp_socket_vice_, c, URI);
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////

FrontConnectionManager::FrontConnectionManager(SessionImp* session_imp) {
	InitializeCriticalSection(&__connections_cs);
	last_change_main_ = ::timeGetTime();
	change_main_time_ = 0;
	sequence_ = 0;
	session_imp_ = session_imp;
	backup_timer_.init(this, &FrontConnectionManager::CheckBackup);
	change_ = false;
	sinfo_front_.SetManager(this);
}

FrontConnectionManager::~FrontConnectionManager() {
	EnterCriticalSection(&__connections_cs);
	FrontConnectionList::iterator it = conntions_.begin();
	for (; it != conntions_.end(); ++it) {
		*it = NULL;
	}
	conntions_.clear();
	LeaveCriticalSection(&__connections_cs);
	DeleteCriticalSection(&__connections_cs);
}

void FrontConnectionManager::Start(const char* ip, uint16_t port) {
	if (sinfo_front_.IsAllConnAlive() == false) {
		sinfo_front_.Start(ip, 10351);
	}
	if (GetConnectionSize() >= 4) {
		return;
	}
	if (IsIpConnection(ip) == true) {
		return;
	}
	nsox::nu_auto_ptr<FrontConnection> master =  nsox::nu_create_object<FrontConnection, nsox::nu_multi_thread_model>::create_inst();

	if (master) {
		InsertConnection(master);
		master->SetManager(this);
		master->SetIpPort(ip, port);
		master->StartTcp();
		backup_timer_.start(1000 * 10);
	}
}

SessionImp* FrontConnectionManager::GetSessionImp() {
	assert(session_imp_);
	return session_imp_;
}

bool FrontConnectionManager::CheckHeadSequence(const char* data, int len) {
	// TODO, 先不管这个
	return true;
	sox::Unpack unpack(data,len);
	uint32_t pack_len;
	uint32_t uri;
	uint16_t rescode;
	uint16_t sid;
	uint8_t tag;
	unpack >> pack_len >> uri >> sid >> rescode >> tag;
	if (seq_uris_.find(uri) != seq_uris_.end()) {
		if (sequence_ + 1 == sid) {
			sequence_ = sid;
			return true;
		} else {
			return false;
		}
	}
	return true;
}

void FrontConnectionManager::TcpConnectionReady(FrontConnection* connction) {
	session_imp_->Login(connction);
}

void FrontConnectionManager::UdpConnectionReady(FrontConnection* connection) {
	session_imp_->RegisterUdp(connection);
}

void FrontConnectionManager::TcpConnectionClose(FrontConnection* connection) {
	EraseConnection(connection);

	if (GetConnectionSize() == 0) {
		session_imp_->Relogin();
	} else if(GetConnectionSize() == 1) {
		session_imp_->GetBackUpFronts(1);
	}
	SetSyncMessage();
}

void FrontConnectionManager::UdpConnectionClose(FrontConnection* connction) {
	session_imp_->UdpClose(connction);
}

void FrontConnectionManager::LoginSuccess(nproto::connection* socket_) {
	nsox::nu_auto_ptr<FrontConnection>  connection = GetConnectionBySocket(socket_);
	if (connection) {
		connection->LoginSuccess();
	}
}

void FrontConnectionManager::UdpRegisterSuccess(nproto::connection* socket_) {
	nsox::nu_auto_ptr<FrontConnection>  connection = GetConnectionBySocket(socket_);
	if (connection) {
		connection->RegisterUdpSuccess();
	}
}

void FrontConnectionManager::SetSeqUris(const std::vector<uint32_t>& uris) {
	std::copy(uris.begin(), uris.end(), std::inserter(seq_uris_, seq_uris_.begin()));
}

void FrontConnectionManager::SetSequence(uint16_t seq) {
	sequence_ = seq;
}

void FrontConnectionManager::CheckBackup() {
	if (change_) {
		ChangeFirst();
		change_ = false;
	}
	if (sinfo_front_.IsAllConnAlive() == false && conntions_.size() > 0) {
		FrontConnectionList::iterator it = conntions_.begin();
		size_t size = conntions_.size();
		size = rand() % size;
		for (size_t i = 0; i < size && it != conntions_.end(); ++it) {
			++i;
		}
		if (it != conntions_.end()) {
			sinfo_front_.Start((*it)->ip_, 10351);
		}
	}
			
	if (GetConnectionSize() == 1) {
		session_imp_->GetBackUpFronts(1);
	}
	SetSyncMessage();
}

void FrontConnectionManager::SetConntionMessage(nproto::connection* socket_, int status) {
	nsox::nu_auto_ptr<FrontConnection>  connection = GetConnectionBySocket(socket_);
	if (connection) {
		connection->message_status_ = status;
	}
	SetSyncMessage();
}

void FrontConnectionManager::SetSyncMessage() {
	if (GetConnectionSize() == 0) {
		return;
	}
	nsox::nu_auto_ptr<FrontConnection>  connection = GetFirstConnection();
	if (connection == NULL) {
		SwapFirst();
		return;
	}
	if (connection->tcp_status_ == FS_TCP_LOGIN && connection->message_status_ != PUserSubcribeMessage::ALL_MESSAGE) {
		XCast<PUserSubcribeMessage> cmd7;
		cmd7.message_type_ = PUserSubcribeMessage::ALL_MESSAGE;
		connection->SendByTcp(&cmd7, PUserSubcribeMessage::uri);
		connection->message_status_ = PUserSubcribeMessage::ALL_MESSAGE;
		ChangeOtherAsSlave(connection);
	}
}

int FrontConnectionManager::GetChangeMainTime() {
	if ( ::timeGetTime() - last_change_main_ > 30 * 60 * 1000) {
		change_main_time_ = 0;
	}
	return change_main_time_;
}

bool FrontConnectionManager::ChangeMain() {
	return ChangeFirst();
}

bool FrontConnectionManager::ChangeFirst() {
	if ( ::timeGetTime() - last_change_main_ > 10 * 1000) {
		last_change_main_ = ::timeGetTime();
	} else {
		return false;
	}

	if (GetSyncConnections() <= 2) {
		session_imp_->GetBackUpFronts(1);
		SwapFirst();
		SetSyncMessage();
	} else {
		nsox::nu_auto_ptr<FrontConnection> connection = GetFirstConnection();
		EraseConnection(connection);
		connection->Close();
		change_main_time_++;
		SwapFirst();
		SetSyncMessage();
		return true;
	}
	return false;
}


/////////////////////////////////////////////////
//conntions_
//no_sub_fuction_


nsox::nu_auto_ptr<FrontConnection> FrontConnectionManager::GetFirstConnection() {
	CAutoLock lock(&__connections_cs);
	if (conntions_.empty())
		return NULL;
	nsox::nu_auto_ptr<FrontConnection>  connection = *(conntions_.begin());
	if (connection && connection->tcp_status_ == FS_TCP_LOGIN) {
		return connection;
	}
	return NULL;
}

nsox::nu_auto_ptr<FrontConnection> FrontConnectionManager::GetConnectionBySocket(nproto::connection* socket) {
	CAutoLock lock(&__connections_cs);
	nsox::nu_auto_ptr<FrontConnection>  connection = NULL;
	FrontConnectionList::iterator it  = conntions_.begin();
	for (; it != conntions_.end(); ++it) {
		if ((*it)->tcp_socket_ == socket || (*it)->udp_socket_ == socket) {
			connection = *it;
			break;
		}
	}
	return connection;
}


bool FrontConnectionManager::EraseConnection(nsox::nu_auto_ptr<FrontConnection>  connection) {
	CAutoLock lock(&__connections_cs);
	FrontConnectionList::iterator it  = conntions_.begin();
	for (; it != conntions_.end(); ++it) {
		if ((*it) == connection) {
			conntions_.erase(it);
			break;
		}
	}
	return true;
}


bool FrontConnectionManager::InsertConnection(nsox::nu_auto_ptr<FrontConnection>  connection) {
	CAutoLock lock(&__connections_cs);
	conntions_.push_back(connection);
	return true;
}

bool FrontConnectionManager::IsIpConnection(const char* ip) {
	CAutoLock lock(&__connections_cs);
	FrontConnectionList::iterator it = conntions_.begin();
	for (; it != conntions_.end(); ++it) {
		if ((*it)->ip_ == ip) {
			return true;
		}
	}
	return false;
}

int FrontConnectionManager::GetConnectionSize() {
	CAutoLock lock(&__connections_cs);
	return conntions_.size();
}

bool FrontConnectionManager::ChangeOtherAsSlave(nsox::nu_auto_ptr<FrontConnection>  connection) {
	EnterCriticalSection(&__connections_cs);
	FrontConnectionList conntions_copy = conntions_;
	LeaveCriticalSection(&__connections_cs);

	FrontConnectionList::const_iterator it = conntions_copy.begin();
	for (; it != conntions_copy.end(); ++it) {
		if ((*it) != connection && (*it)->tcp_status_ == FS_TCP_LOGIN && (*it)->message_status_ != PUserSubcribeMessage::SEQUENCE_MESSAGE) {
			XCast<PUserSubcribeMessage> cmd7;
			cmd7.message_type_ = PUserSubcribeMessage::SEQUENCE_MESSAGE;
			(*it)->SendByTcp(&cmd7, PUserSubcribeMessage::uri);
			(*it)->message_status_ = PUserSubcribeMessage::SEQUENCE_MESSAGE;
		}
	}
	return true;
}

bool FrontConnectionManager::HasTcpLoginReady()  {
	CAutoLock lock(&__connections_cs);
	bool ret = false;
	FrontConnectionList::const_iterator it = conntions_.begin();
	for (; it != conntions_.end(); ++it) {
		if ((*it)->tcp_status_ == FS_TCP_LOGIN) {
			ret = true;
			break;
		}
	}
	return ret;
}

uint32_t FrontConnectionManager::GetPing() {
	CAutoLock lock(&__connections_cs);
	uint32_t ping = 10000;
	FrontConnectionList::iterator it = conntions_.begin();
	for (; it != conntions_.end(); ++it) {
		if (ping > (*it)->GetPing()) {
			ping = (*it)->GetPing();
		}
	}
	return ping;
}


bool FrontConnectionManager::SwapFirst() {
	CAutoLock lock(&__connections_cs);
	if (conntions_.size() < 2) 
		return false;

	FrontConnectionList::iterator it  = conntions_.begin();
	FrontConnectionList::iterator it_swap  = conntions_.begin();
	nsox::nu_auto_ptr<FrontConnection>  connection = *it;
	it_swap++;
	for (; it_swap != conntions_.end(); ++it_swap) {
		if ((*it_swap)->tcp_status_ == FS_TCP_LOGIN) {
			break;
		}
	}
	if (it_swap == conntions_.end()) {
		return false;
	}
	nsox::nu_auto_ptr<FrontConnection>  swap = *it;
	*it = (*it_swap);
	*it_swap = swap;
	return true;
}

int FrontConnectionManager::GetSyncConnections() {
	int size = 0;
	CAutoLock lock(&__connections_cs);
	
	FrontConnectionList::iterator it  = conntions_.begin();
	for (; it != conntions_.end(); ++it) {
		if ((*it)->tcp_status_ == FS_TCP_LOGIN) {
			size++;
		}
	}
	return size;
}

void FrontConnectionManager::ChangeFirstNextTime() {
	change_ = true;
}

}
}
