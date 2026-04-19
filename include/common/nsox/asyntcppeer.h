#ifndef	 __asyn_tcp_peer_h
#define  __asyn_tcp_peer_h
#include "sockbuffer.h"

nsox_namespace_begin

class tcp_peer_socket : public tcpsocket
{
public: 
	virtual void set_socket_handler(HSOCKET hSocket)
	{
			tcpsocket::set_socket_handler(hSocket);		
			set_connected(true);
	}
	virtual void create()
	{
			nsox::xlog(NsoxError, "tcp peer socket do not support create function");
	}
};

class asyn_tcp_peer : public asyn_socket<tcp_peer_socket>
{
public:
	virtual void set_socket_handler(HSOCKET hSocket)
	{
			asyn_socket<tcp_peer_socket>::set_socket_handler(hSocket);
			set_connected(true);
	}
};


nsox_namespace_end
#endif

