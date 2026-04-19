#include "tcpsocket.h"
#include "socketutil.h"

nsox_namespace_begin

inline tcpsocket::tcpsocket()
{
		__is_connected		= false;
		__is_read_closed	= false;
		__is_write_closed	= false;
}

inline tcpsocket::~tcpsocket()
{

}
inline void tcpsocket::create()
{
		socket::create();
		socket_result<HSOCKET> res = SocketUtil::create(AF_INET,SOCK_STREAM);
		if(res.error()){
				nsox::xlog_err(NsoxFatal,res.__errno,"create tcp socket faild");
				throw socket_error("create tcp socket faild", res.__errno);
		}
		set_socket_handler((HSOCKET)res);
}
inline nu_size_t	tcpsocket::write(const char* data, nu_size_t len)
{
		return SocketUtil::write(get_socket_handler(), data, len);
}
inline nu_size_t	tcpsocket::read( char* buffer,  nu_size_t len)
{
		return SocketUtil::read(get_socket_handler(),buffer, len);
}
inline address	 tcpsocket::get_local_address()
{
		return SocketUtil::getLocalAddress(get_socket_handler());
}
inline address	tcpsocket::get_peer_address()
{
		return SocketUtil::getPeerAddress(get_socket_handler());
}
inline bool	tcpsocket::is_connected()
{
		return __is_connected;
}
inline void	tcpsocket::set_connected(bool on)
{
		__is_connected = on;
}
inline void	tcpsocket::close_read()
{
		SocketUtil::closeRead(get_socket_handler());
		__is_read_closed = true;
}
inline bool	tcpsocket::is_read_closed()
{
		return __is_read_closed;
}
inline void	tcpsocket::close_write()
{
		SocketUtil::closeWrite(get_socket_handler());
		__is_write_closed = true;
}
inline bool	tcpsocket::is_write_closed()
{
		return __is_write_closed;
}
inline void	tcpsocket::set_recv_timeout(nu_time_t milliseconds)
{
		SocketUtil::setTimeout(get_socket_handler(), NSOX_RECV_TIMEOUT, milliseconds);
}
inline void	tcpsocket::set_send_timeout(nu_time_t milliseconds)
{
		SocketUtil::setTimeout(get_socket_handler(), NSOX_SEND_TIMEOUT, milliseconds);
}
inline nu_time_t	tcpsocket::get_recv_timeout()
{
		return SocketUtil::getTimeout(get_socket_handler(), NSOX_RECV_TIMEOUT);
}
inline nu_time_t	tcpsocket::get_send_timeout()
{
		return SocketUtil::getTimeout(get_socket_handler(), NSOX_SEND_TIMEOUT);
}
inline void	tcpsocket::set_tcp_nodelay( boolean on)
{
		SocketUtil::setTcpNoDelay(get_socket_handler(), on);
}
inline boolean	tcpsocket::is_tcp_nodelay()
{
		return SocketUtil::isTcpNoDelay(get_socket_handler());
}
inline void	tcpsocket::set_solinger(nu_time_t linger)
{
		SocketUtil::setSoLinger(get_socket_handler(), linger);
}
inline nu_time_t	tcpsocket::get_solinger()
{
		return SocketUtil::getSoLinger(get_socket_handler());
}
inline void	tcpsocket::set_sendbuff_size(nu_size_t size)
{
		SocketUtil::setSendBufferSize(get_socket_handler(), size);
}
inline nu_size_t	tcpsocket::get_sendbuff_size()
{	
		return SocketUtil::getSendBufferSize(get_socket_handler());
}
inline void	tcpsocket::set_recvbuff_size(nu_size_t size)
{
		SocketUtil::setRecvBufferSize(get_socket_handler(),size);
}
inline nu_size_t	tcpsocket::get_recvbuff_size()
{
		return SocketUtil::getRecvBufferSize(get_socket_handler());
}

nsox_namespace_end


