#ifndef nsox_socket_helper_h
#define nsox_socket_helper_h
#include "sockcomm.h"

nsox_namespace_begin

class SocketUtil
{
public:
	static socket_result<void>		setSendBufferSize(HSOCKET hSocket, nu_size_t size);
	static socket_result<void>		setRecvBufferSize(HSOCKET hSocket, nu_size_t size);

	static socket_result<nu_size_t>	getSendBufferSize(HSOCKET hSocket);
	static socket_result<nu_size_t>	getRecvBufferSize(HSOCKET hSocket);

	static socket_result<void>		setTcpNoDelay(HSOCKET hSocket, boolean on);	
	static socket_result<address>	getLocalAddress(HSOCKET hSocket);
	static socket_result<address>	getPeerAddress(HSOCKET hSocket);
	static socket_result<void>		close(HSOCKET hSocket);
	static socket_result<void>		closeRead(HSOCKET hSocket);
	static socket_result<void>		closeWrite(HSOCKET hSocket);
	static socket_result<void>		setTimeout(HSOCKET hSocket, int side, nu_time_t timeout);
	static socket_result<void>		setSoLinger(HSOCKET hSocket, nu_time_t millseconds);
	static socket_result<void>		bind(HSOCKET hSocket, const address& addr);
	static socket_result<void>		setBlocking(HSOCKET hSocket, boolean on = true);


	static socket_result<nu_time_t>	getTimeout(HSOCKET hSocket, int side );
	static socket_result<nu_time_t>	getSoLinger(HSOCKET hSocket);
	static socket_result<boolean>	isTcpNoDelay(HSOCKET hSocket);
	static socket_result<boolean>	isBlocking(HSOCKET hSocket);
	
	static socket_result<nu_size_t>	getAvailBytes(HSOCKET hSocket);

	static socket_result<nu_size_t>	read(HSOCKET hSocket, void* buf, nu_size_t len, int flag = 0);
	static socket_result<nu_size_t>	write(HSOCKET hSocket, const void* buf, nu_size_t len, int flag = 0); 
	static socket_result<nu_size_t>	recvfrom(HSOCKET hSocket, void* buf, nu_size_t len, address& addr, int flag = 0);
	static socket_result<nu_size_t>	sendto(HSOCKET hSocket, const void* buf, nu_size_t len, const address& addr, int flag = 0);


	static socket_result<void>		connect(HSOCKET hSocket, const address& addr);
	static socket_result<HSOCKET>	accept(HSOCKET hSocket, address& addr);
	static socket_result<void>		listen(HSOCKET hSocket);

	static socket_result<HSOCKET>	create(int addressFamily, int type, int protocol = 0);

	static socket_result<boolean>	isReuseAddress(HSOCKET hSocket);
	static socket_result<void>		setReuseAddress(HSOCKET hSocket, boolean on);

	static	socket_result<error_t>	get_socket_error(HSOCKET hSocket);

	static socket_result<int>		waitevent(HSOCKET hSocket, int event, uint32_t timeout);


	static socket_result<boolean>	getsockopt(HSOCKET hSocket, int level, int optname, void* optval, socklen_t* optlen);
	static socket_result<boolean>	setsockopt(HSOCKET hSocket, int level, int optname, const void *optval, socklen_t optlen);

	static void						format_error(error_t err, std::string& str);
	static nu_time_t				get_now_time();

	static void						sleep_us(uint32_t nusecs);
	static void						sleep_ms(uint32_t nusecs);

};
nsox_namespace_end

#include "socketutil.hpp"

#endif// nsox_socket_helper_h

