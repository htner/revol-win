#ifndef __nsox_udpsocket_h
#define __nsox_udpsocket_h
#include "socket.h"
nsox_namespace_begin

class udpsocket : public socket
{
public:
	udpsocket():__is_connected(false){
			
	}
	virtual ~udpsocket()
	{
	}
	virtual void create()
	{
			socket::create();
			socket_result<HSOCKET> res =  SocketUtil::create(AF_INET, SOCK_DGRAM, 0);
			if(res.error()){
					nsox::xlog_err(NsoxError, res.__errno,"create tcp socket faild");
					throw socket_error("create udp socket faild", res.__errno);
			}
			set_socket_handler((HSOCKET)res);
	}
	/*
	* connect server, in udp mode, it don't really contact server 
	*/
	void connect(const address& addr)
	{
			socket_result<void> res = SocketUtil::connect(get_socket_handler(), addr);
			if(res.error()){
					nsox::xlog_err(NsoxError, res.__errno, "udpsocket::connect");
					throw socket_error( "udpsocket::connect", res.__errno);
			}else{
					__is_connected = true;
			}
	}
	boolean is_connected()
	{
			return __is_connected;
	}
	/*
	* disconnect from server, the binded local address still valid
	*/
	void disconnect(){
			throw "unimplement exception";
	}
	/*
	* bind a local address to this socket, if error , throw nsoxerror
	*/
	void bind(const address& addr){
			socket_result<void> res = SocketUtil::bind(get_socket_handler(), addr);
			if(res.error()){
						nsox::xlog_err(NsoxError, res.__errno, "udp bind socket error on %s:%d", addr.get_ip_addr().c_str(), addr.port);
						throw socket_error("udpsocket bind fail", res.__errno);
			}
			__address = addr;
	}
	address get_bind_address()
	{
			return __address;
	}
	void sendstream(const char* data, nu_size_t len)
	{
			if(is_connected())
			{
					socket_result<nu_size_t> res = SocketUtil::write(get_socket_handler(), data, len);
					if(res.error()) nsox::xlog_err(NsoxDebug, res.__errno, "sendstream error");
			}else{
					nsox::xlog(NsoxError, "you should provided remote address on an unconnected udp socket!");
			}		
	}
	void sendstream(const char* data, nu_size_t len, const address& addr)
	{
			if(!is_connected()){
					socket_result<nu_size_t> res = SocketUtil::sendto(get_socket_handler(), data, len, addr);

					if(res.error()) nsox::xlog_err(NsoxDebug, res.__errno, "sendstream error");
			}else{
					nsox::xlog(NsoxError, "you should not provided remote address on an connected udp socket!");
					sendstream(data, len);
			}
	}
	int readstream(char* buf, nu_size_t len, address& addr)
	{
			socket_result<nu_size_t> res = SocketUtil::recvfrom(get_socket_handler(), buf, len, addr);
			if(res.error()) nsox::xlog_err(NsoxDebug, res.__errno, "sendstream error");
			return res;
	}
	int read(char* buf, nu_size_t len)
	{
			if(is_connected()){
					socket_result<nu_size_t> res = SocketUtil::read(get_socket_handler(), buf, len);
					if(res.error()) nsox::xlog_err(NsoxDebug, res.__errno, "sendstream error");
					return res;
			}else{
					nsox::xlog(NsoxError, "should not read from an unconnected udp socket");
					return 0;
			}
	}
private:
	bool	__is_connected;
	address	__address;
};

nsox_namespace_end
#endif


