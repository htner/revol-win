#ifndef __nsox_http_server_h
#define __nsox_http_server_h

class asyntcpserver : public asyn_server_handler
{
public:
	asyntcpserver(){
			__server_socket = nu_create_object<asyn_server_socket>::create_inst();
			__server_socket.set_handler(this);
	}
	virtual void onsocket(HSOCKET hsocket, const address& addr)
	{			
	}
protected:
	nu_auto_ptr<asyn_server_socket> __server_socket;
};

class httpconn : public asyn_tcp_socket<asyn_tcp_peer>
{
public:
	httpconn
};

class httpserver : public asyntcpserver
{
public:
	httpserver(void);
	virtual ~httpserver(void);
	virtual void onsocket(HSOCKET hsocket, const address& addr);
};

#endif