#pragma once
#include "proto/isession.h"
#include "proto/ismanager.h"
#include "protocol/psessionbase.h"
#include "protocol/plogin.h"
#include "common/iproperty.h"
#include "common/core/ibase.h"
#include "common/core/iclient.h"
#include "protocol/psession.h"
#include "timer/TimerWrap.h"
#include "common/nsox/nuautoptr.h"
#include "common/nsox/proto_cast.h"
#include "common/nsox/nu_request.h"
#include "common/nsox/default_proto_handler.h"
#include "helper/RSAKey.h"
#include "sitem.h"
#include "WsTcpSocket.h"
#include "WsUdpSocket.h"

#ifdef WIN32
#include "atlwin.h"
#endif

#include <set>
#include <list>
#include <fstream>

#include "memMisc.h"

#define QUEUE_SIZE 60

namespace protocol {
namespace session {

extern bool g_is_udp;

class SessionImp;

class FrontConnectionManager;

enum FrontConnectionStatus {
	FS_INIT = 0,
	FS_TCP_INIT = 1,
	FS_TCP_CONNECT = 2,
	FS_TCP_ENCODE = 3,
	FS_TCP_LOGIN = 4,
	FS_UDP_INIT = 11,
	FS_UDP_CONNECT = 12,
	FS_UDP_LOGIN = 13
};


class FrontConnection : public nproto::default_proto_handler, public nsox::IAutoPtr {
public:
	DECLARE_REQUEST_MAP

	FrontConnection();
	virtual ~FrontConnection();

	void SetManager(FrontConnectionManager* manager);
	void SetIpPort(const char* ip, uint16_t port);

	void StartTcp();
	void StartUdp();
	void Close();

	void OnTcpPing(uint32_t ms);
	void OnUdpPing(uint32_t ms);
	virtual void onclose(int err, nproto::connection* c);
	virtual void oncreate(nproto::connection* c);

	void TcpKeepAlive();
	void UdpKeepAlive();

	bool OnTcpKeepAliveRes(XCast<protocol::session::PTcpPingRes>& cmd, nproto::connection* c);
	bool OnUdpKeepAliveRes(XCast<protocol::login::PPlus>& cmd, nproto::connection* c);
	
	bool CheckConnection(nproto::connection* c);
	void OnExchangeKeyRes(XCast<protocol::login::PExchangeKeyRes>& cmd, nproto::connection* c);

	bool Send(nproto::request* c, int URI);
	bool SendByUdp(nproto::request* c, int URI);
	bool SendByTcp(nproto::request* c, int URI);

	void LoginSuccess();
	void RegisterUdpSuccess();
	uint32_t GetPing();


private:
	TimerHandler<FrontConnection>	udp_timer_; 
	TimerHandler<FrontConnection>	tcp_timer_;
	
	FrontConnectionStatus tcp_status_;
	FrontConnectionStatus udp_status_;

	int message_status_;
	uint32_t tcp_ping_;
	uint32_t udp_ping_;
	uint32_t last_tcp_time_;
	uint32_t last_udp_time_;
	std::string	ip_;
	uint16_t port_;
	nsox::nu_auto_ptr<CEncryTcpSocket> tcp_socket_;
	nsox::nu_auto_ptr<CWsUdpSocket>	udp_socket_;
	FrontConnectionManager* conn_manager_;
	
	friend class FrontConnectionManager;
};


class SInfoFront : public nproto::default_proto_handler {
public:
	SInfoFront();
	virtual ~SInfoFront();

	DECLARE_REQUEST_MAP

public:
	virtual void onclose(int err, nproto::connection* c);
	virtual void oncreate(nproto::connection* c);
	
public:
	void SetManager(FrontConnectionManager* manager);
	bool IsConnAlive();
	bool IsAllConnAlive();
	void Start(const std::string& ip, uint16_t);
	bool Send(nproto::request* c, int URI);

private:
	nsox::nu_auto_ptr<CEncryTcpSocket> tcp_socket_main_;
	nsox::nu_auto_ptr<CEncryTcpSocket> tcp_socket_vice_;
	FrontConnectionManager* conn_manager_;
	friend class FrontConnectionManager;
};



typedef std::vector<nsox::nu_auto_ptr<FrontConnection> >   FrontConnectionList;

class FrontConnectionManager  {
public:
	DECLARE_REQUEST_MAP
	FrontConnectionManager(SessionImp*);
	~FrontConnectionManager();

	void Start(const char* ip, uint16_t port);

	template<typename T>
	void Send(T& req) {
		SendRequest(req, T::uri);
	}

	void SendRequest(nproto::request& req, int uri) {
		nsox::nu_auto_ptr<FrontConnection> connection = GetFirstConnection();
		if (connection) {
			switch(uri) {
				case PUdpLogin::uri:
				case PUdpPing::uri:
					connection->SendByUdp(&req, uri);
					break;
				case PStopVoice::uri:
				case PChatVoice::uri:
				case PReSendVoice::uri:
					connection->Send(&req, uri);
					break;
				case PGetUserInfo::uri:
				case PGetUserSessionCredit::uri:
				case PGetSessionMemberCredit::uri:
					if (sinfo_front_.IsConnAlive()) {
						sinfo_front_.Send(&req, uri);
					} else {
						connection->SendByTcp(&req, uri);
					}
					break;
				default:
					connection->SendByTcp(&req, uri);
				break;
			}
		}
	}

	template<typename T>
	void SendByFirstBackup(T& req) {
		SendRequestByFirstBackup(req, T::uri);
	}

	/*
	void SendRequestByFirstBackup(nproto::request& req, int uri) {

		if (conntions_.size() >= 2) {
			std::list<FrontConnection*>::iterator it = conntions_.begin();
			++it;
			switch(uri) {
				case PUdpLogin::uri:
				case PUdpPing::uri:
					(*it)->SendByUdp(&req, uri);
					break;
				case PStopVoice::uri:
				case PChatVoice::uri:
				case PReSendVoice::uri:
					(*it)->Send(&req, uri);
					break;
				default:
					(*it)->SendByTcp(&req, uri);
				break;
			}
		} else {
			SendRequest(req, uri);
		}
	}
	*/

	template<typename T>
	void SendByTcp(T& req){
		nsox::nu_auto_ptr<FrontConnection> connection = GetFirstConnection();
		if (connection) {
			connection->SendByTcp(seq, T::uri);
		}
	};

	template<typename T>
	void SendByUdp(T& req){
		nsox::nu_auto_ptr<FrontConnection> connection = GetFirstConnection();
		if (connection) {
			connection->SendByUdp(seq, T::uri);
		}
		
	};

	template<typename T>
	void SendByConnection(nsox::nu_auto_ptr<FrontConnection> connection, T& req) {
		if (connection) {
			switch(T::uri) {
				case PUdpLogin::uri:
				case PUdpPing::uri:
					connection->SendByUdp(&req, T::uri);
					break;
				case PStopVoice::uri:
				case PChatVoice::uri:
				case PReSendVoice::uri:
					connection->Send(&req, T::uri);
					break;
				case PGetUserInfo::uri:
				case PGetUserSessionCredit::uri:
				case PGetSessionMemberCredit::uri:
					if (sinfo_front_.IsConnAlive()) {
						sinfo_front_.Send(&req, T::uri);
					} else {
						connection->SendByTcp(&req, T::uri);
					}
					break;
				default:
					connection->SendByTcp(&req, T::uri);
				break;
			}
		}
	}

	SessionImp* GetSessionImp();
	bool CheckHeadSequence(const char* data, int len);

	
	void TcpConnectionReady(FrontConnection*);
	void UdpConnectionReady(FrontConnection*);
	void TcpConnectionClose(FrontConnection*);
	void UdpConnectionClose(FrontConnection*);
	void LoginSuccess(nproto::connection* socket_);
	void UdpRegisterSuccess(nproto::connection* socket_);


	void SetSeqUris(const std::vector<uint32_t>&);
	void SetSequence(uint16_t);
	void CheckBackup();
	void SetConntionMessage(nproto::connection* socket_, int);
	void SetSyncMessage();
	
	bool ChangeMain();
	bool ChangeFirst();
	int GetChangeMainTime();


	nsox::nu_auto_ptr<FrontConnection> GetFirstConnection();
	nsox::nu_auto_ptr<FrontConnection> GetConnectionBySocket(nproto::connection* socket_);
	bool EraseConnection(nsox::nu_auto_ptr<FrontConnection>  connection);
	bool InsertConnection(nsox::nu_auto_ptr<FrontConnection>  connection);
	bool IsIpConnection(const char* ip);
	int GetConnectionSize();
	bool ChangeOtherAsSlave(nsox::nu_auto_ptr<FrontConnection>  connection);
	bool HasTcpLoginReady();
	uint32_t GetPing();
	bool SwapFirst();
	int GetSyncConnections();
	void ChangeFirstNextTime();

private:
	bool change_;
	unsigned long last_change_main_;	// 防止过多更换主链接导致不稳定
	int	change_main_time_;				// 总次数
	uint32_t last_ping_time_;
	FrontConnectionList conntions_;     //功能
	//nsox::nu_auto_ptr<FrontConnection> info_conntion_;  //资源下载
	std::set<uint32_t> seq_uris_;
	uint16_t sequence_;
	SessionImp* session_imp_;
	TimerHandler<FrontConnectionManager>	backup_timer_;
	CRITICAL_SECTION __connections_cs;					// 最小粒度控制conntions_,需要优化
	SInfoFront			sinfo_front_;

};

}
}