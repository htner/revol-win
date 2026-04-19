#ifndef __nsox_app_h
#define __nsox_app_h

#include "nsox.h"
#include "asynserversocket.h"
#include "nuautoptr.h"
#include "nsoxenv.h"



nsox_namespace_begin

template <typename t_connection, typename t_server_socket = asyn_server_socket>
class nsox_tcp_server  : public nsox_app_handler 
					   , public asyn_server_handler
{
public:
	nsox_tcp_server(int port, const char* addr = NULL)
	{
			__server_socket = nu_create_object<t_server_socket>::create_inst();
			__server_socket->set_handler(this);
			__address = address(port, addr);
			
	}
	virtual ~nsox_tcp_server(){}
	void start()
	{
			__server_socket->create();
			__server_socket->set_reuse_addr(true);
			__server_socket->listen(__address);
			nsox::xlog(NsoxDebug, "tcp server listend on %s:%d", __address.get_ip_addr().c_str(), __address.port);
	}
	void stop()
	{
			__server_socket->close();
	}

	void set_address(address& addr)
	{
			__address = addr;
	}
	virtual void onsocket(HSOCKET hsocket, const address& addr)
	{
			nsox::xlog(NsoxDebug, "accept client from %s:%d", addr.get_ip_addr().c_str(), addr.port);
			nu_auto_ptr<t_connection> conn = nu_create_object<t_connection>::create_inst();
			conn->set_socket_handler(hsocket);
			onsocket(conn);			
	}
	// if you override this function, you should call this default function
	virtual void onsocket(t_connection* conn)
	{
			conn->select(EVENT_READ, true);			
	}

protected:
	nu_auto_ptr<t_server_socket> __server_socket;
	address						 __address;
};

template<typename udp_server_socket = asyn_udp_socket>
class nsox_udp_server : public nsox_app_handler
{
public:
	class udp_socket_wrapper : public udp_server_socket
	{
	public:
		virtual void ondata(const char* p, int len, address& addr)
		{
					__xcallback->ondata(p, len, addr);
		}
		nsox_udp_server* __xcallback; 
	};
public:
	nsox_udp_server(int port, const char* addr = NULL)
	{
			__server_socket = nu_create_object<udp_socket_wrapper>::create_inst();
			__server_socket->__xcallback = this;
			__server_socket->create();
			__server_socket->bind(address(port, addr));
	}
	virtual ~nsox_udp_server(){}
	void start()
	{		
			__server_socket->select(EVENT_READ, true);
			address udpaddr = __server_socket->get_bind_address();
			nsox::xlog(NsoxDebug, "udp server listend on %s:%d", udpaddr.get_ip_addr().c_str(), udpaddr.port);
	}
	void stop()
	{
			__server_socket->close();
	}
	// for override
	virtual void ondata(const char* p, int len,  address& addr)
	{
			std::cout << std::string(p, len) << std::endl;
	}
protected:
	nu_auto_ptr<udp_socket_wrapper> __server_socket;
};

/************************************************************************/
/* Nosx Application Template, To simplify create nsox application       */
/************************************************************************/


class nsoxapp 
{
public:
	nsoxapp()
	{
			//__selector = nu_create_object<selector_select>::create_inst();
	}
	virtual ~nsoxapp(){	
			
	}
	void run() // start application for server
	{			
			env::inst()->selector()->run(500);
	}
	void stop()
	{
			env::inst()->selector()->stop();
	}
	void add_server(nsox_app_handler* server)
	{
			if(server == NULL) return;
			server->start();
	}
};


nsox_namespace_end

#endif

