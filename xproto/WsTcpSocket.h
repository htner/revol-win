#pragma once

#include "AsyncSocketEx.h"
#include <common/nsox/blockbuffer.h>
#include <common/nsox/nu_request.h>
#include <common/nsox/proto_conn.h>
#include <openssl/rc4.h>



class CWsTcpSocket :  public  CAsyncSocketEx
				   ,  public  nproto::connection
{
public:
	CWsTcpSocket(void);
	virtual ~CWsTcpSocket(void);

	virtual void	AsynSend(const char* data, size_t len);
	void	SetProtoHandler(nproto::proto_handler* handler);
	void	AsynConnect(const char* host, UINT port);

public: //override nproto::connection interface
	virtual bool write_data(const char* p, nsox::nu_size_t len);
	virtual void close_conn();
	virtual bool is_conn_close();
	virtual bool is_conn_alive();

protected:
	virtual void OnClose(int err);
	virtual void OnConnect(int err);
	virtual void OnReceive(int err);
	virtual void OnSend(int err);

	typedef sox::BlockBuffer<sox::def_block_alloc_16k, 256> buffer_type;

protected:
	nproto::proto_handler*	__protoHandler;
	buffer_type				__input;
	buffer_type				__output;
	
	bool					__connected;
	bool					__closed;
	CRITICAL_SECTION		__output_cs;
};



class CEncryTcpSocket : public CWsTcpSocket
{
public:
	CEncryTcpSocket();
	virtual ~CEncryTcpSocket(){}
	virtual void	AsynSend(const char* data, size_t len);
	void	setEncKey(const unsigned char *k, size_t len);
	bool    isEncrypto();
	void	SendBuffer();
public:
	virtual bool write_data(const char* p, nsox::nu_size_t len);

protected:
	virtual void OnReceive(int err);
protected:
	RC4_KEY __sendKey;
	RC4_KEY __recvKey;
	bool	__bEncypt;
};