#pragma once
#include <common/nsox/nu_request.h>
#include <common/nsox/proto_conn.h>
#include "AsyncSocketEx.h"


class CWsUdpSocket : public CAsyncSocketEx
				   , public  nproto::connection
{
public:
	CWsUdpSocket(void);
	virtual ~CWsUdpSocket(void);

public: //override nproto::connection interface
	virtual bool write_data(const char* p, nsox::nu_size_t len);
	virtual void close_conn();
	virtual bool is_conn_close();
	virtual bool is_conn_alive();


	void create();
	void connect(const char* ip, unsigned int port);
	void send(const char* data, unsigned int len);
	void close();
	void setProtoHandler(nproto::proto_handler* handler);
protected:
	void OnReceive(int err);
private:
	SOCKADDR_IN				m_addr;
	nproto::proto_handler*	__protoHandler;
	bool					__closed;
	bool					__connected;
};
