#include "socketutil.h"
#include <errno.h>

nsox_namespace_begin

typedef struct sockaddr_in ipaddr_type;

inline socket_result<boolean>		SocketUtil::getsockopt(HSOCKET hSocket, int level, int optname,
									 void* optval, socklen_t* optlen)
{
		if(NSOX_SOCKET_ERROR == ::getsockopt(hSocket, level, optname,
											(char *)optval, optlen))
		{
				return socket_result<boolean>(false, nsox_last_error);
		}
		return socket_result<boolean>(true);
}
inline socket_result<boolean>		SocketUtil::setsockopt(HSOCKET hSocket, int level, int optname,
									 const void *optval, socklen_t optlen)
{
		if(NSOX_SOCKET_ERROR == ::setsockopt(hSocket, level, optname,
											 (const char *)optval, optlen))
		{
				return socket_result<boolean>(false, nsox_last_error);
		}
		return socket_result<boolean>(true);
}
inline socket_result<void>	SocketUtil::setSendBufferSize(HSOCKET hSocket, nu_size_t size)
{
		socket_result<boolean> res = setsockopt(hSocket,SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
		return socket_result<void>(res.__errno);
}
inline socket_result<void>	SocketUtil::setRecvBufferSize(HSOCKET hSocket, nu_size_t size)
{
		socket_result<boolean> res = setsockopt(hSocket,SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
		return socket_result<void>(res.__errno);
}
inline socket_result<nu_size_t>	SocketUtil::getSendBufferSize(HSOCKET hSocket)
{
		nu_size_t size;
		socklen_t len = sizeof(size);

		socket_result<boolean> res = getsockopt(hSocket,SOL_SOCKET, SO_SNDBUF, &size, &len);
		return socket_result<nu_size_t>(size, res.__errno);
}
inline socket_result<nu_size_t>	SocketUtil::getRecvBufferSize(HSOCKET hSocket)
{
		nu_size_t size;
		socklen_t len = sizeof(size);

		socket_result<boolean> res = getsockopt(hSocket,SOL_SOCKET, SO_RCVBUF, &size, &len);
		return socket_result<nu_size_t>(size, res.__errno);
}


inline socket_result<address>	SocketUtil::getLocalAddress(HSOCKET hSocket)
{
		address netaddr;
		ipaddr_type sa;
		memset(&sa, 0, sizeof(sa));
		socklen_t len = sizeof(sa);

		if (NSOX_SOCKET_ERROR == ::getsockname(hSocket, (struct sockaddr *)&sa, &len))
		{
				return socket_result<address>(netaddr, nsox_last_error);
		}
		netaddr.ip		= sa.sin_addr.s_addr;
		netaddr.port	= ntohs(sa.sin_port);
		return socket_result<address>(netaddr);
}
inline socket_result<address>	SocketUtil::getPeerAddress(HSOCKET hSocket)
{
		address netaddr;
		ipaddr_type sa;
		memset(&sa, 0, sizeof(sa));
		socklen_t len = sizeof(sa);

		if (NSOX_SOCKET_ERROR == ::getpeername(hSocket, (struct sockaddr *)&sa, &len))
		{
				return socket_result<address>(netaddr, nsox_last_error);
		}
		netaddr.ip		= sa.sin_addr.s_addr;
		netaddr.port	= ntohs(sa.sin_port);
		return socket_result<address>(netaddr);
}

inline socket_result<void>		SocketUtil::setTimeout(HSOCKET hSocket, int side , nu_time_t timeout)
{
		timeval  tv;
		tv.tv_sec  =  timeout/1000;
		tv.tv_usec =  1000 * (timeout%1000);
		socket_result<boolean> res = setsockopt(hSocket, SOL_SOCKET, side, &tv, sizeof(tv));
		return socket_result<void>(res.__errno);
}
inline socket_result<void>		SocketUtil::setSoLinger(HSOCKET hSocket, nu_time_t millseconds)
{
		linger lg;
		lg.l_onoff	= millseconds;	// ==0 will disable
		lg.l_linger	= millseconds;
		socket_result<boolean> res =  setsockopt(hSocket, SOL_SOCKET, SO_LINGER, &lg, sizeof(lg));
		return socket_result<void>(res.__errno);
}


inline socket_result<nu_time_t>	SocketUtil::getTimeout(HSOCKET hSocket, int side)
{
		timeval  tv;
		memzero(tv);
		socklen_t len = sizeof(tv);
		socket_result<boolean> res =  getsockopt(hSocket, SOL_SOCKET, side, &tv, &len);
		return socket_result<nu_time_t>(tv.tv_sec * 1000000 + tv.tv_usec, res.__errno);
}
inline socket_result<nu_time_t>	SocketUtil::getSoLinger(HSOCKET hSocket)
{
		linger lg;
		memzero(lg);
		socklen_t len = sizeof(lg);
		socket_result<boolean> res =  getsockopt(hSocket, SOL_SOCKET, SO_LINGER, &lg, &len);
		return socket_result<nu_time_t>(lg.l_onoff == 0 ? 0 : lg.l_linger, res.__errno);

}
inline socket_result<boolean>	SocketUtil::isTcpNoDelay(HSOCKET hSocket)
{
		int option = 0;
		socklen_t len = sizeof(option);
		socket_result<boolean> res = getsockopt(hSocket, SOL_SOCKET, TCP_NODELAY, &option, &len);
		return socket_result<boolean>(option, res.__errno);
}
inline socket_result<void>	SocketUtil::setTcpNoDelay(HSOCKET hSocket, boolean on)
{
		int option = on ? 1 : 0;
		socket_result<boolean> res = setsockopt(hSocket, SOL_SOCKET, TCP_NODELAY, &option, sizeof(option));
		return socket_result<void>(res.__errno);
}

inline socket_result<nu_size_t>	SocketUtil::getAvailBytes(HSOCKET hSocket)
{
		return 0;
}

inline socket_result<nu_size_t>	SocketUtil::read(HSOCKET hSocket, void* buf, nu_size_t len, int flag)
{
again:
		int32_t res = ::recv(hSocket, (char*)buf, len, flag);
		if(res < 0){
				if(nsox_last_error == NU_EINTR) goto again;
				return socket_result<nu_size_t>(0, nsox_last_error);
		}
		return socket_result<nu_size_t>(res);
}
inline socket_result<nu_size_t>	SocketUtil::write(HSOCKET hSocket, const void* buf, nu_size_t len, int flag)
{
again:
		int32_t res = ::send(hSocket, (const char*)buf, len, flag);
		if(res < 0){
				if(nsox_last_error == NU_EINTR) goto again;
				return socket_result<nu_size_t>(0, nsox_last_error);
		}
		return socket_result<nu_size_t>(res);
}
inline socket_result<nu_size_t>	SocketUtil::recvfrom(HSOCKET hSocket, void* buf, nu_size_t len, address& netaddr, int flag)
{
		ipaddr_type fromsa;
		memzero(fromsa);
		socklen_t salen = sizeof(fromsa);
again:
		int32_t ret = ::recvfrom(hSocket, (char *)buf, len, flag, (struct sockaddr*)&fromsa, &salen);
		if (ret < 0){
				if(nsox_last_error == NU_EINTR) goto again;
				return socket_result<nu_size_t>(0, nsox_last_error);
		}
		netaddr.ip				= fromsa.sin_addr.s_addr;
		netaddr.port			= ntohs(fromsa.sin_port);
		return socket_result<nu_size_t>(ret);
}
inline socket_result<nu_size_t>	SocketUtil::sendto(HSOCKET hSocket, const void* buf, nu_size_t len, const address& netaddr, int flag)
{
		ipaddr_type tosa;
		tosa.sin_family			= AF_INET;
		tosa.sin_addr.s_addr	= netaddr.ip;
		tosa.sin_port			= htons(netaddr.port);
again:
		int32_t ret = ::sendto(hSocket, (const char *)buf, len, flag, (struct sockaddr*)&tosa, sizeof(tosa));
		if (ret < 0){
				if(nsox_last_error == NU_EINTR) goto again;
				return socket_result<nu_size_t>(0, nsox_last_error);
		}
		return socket_result<nu_size_t>(ret);
}

inline socket_result<void>		SocketUtil::bind(HSOCKET hSocket, const address& netaddr)
{
		ipaddr_type sa;
		memset(&sa, 0, sizeof(sa));

		sa.sin_family		= AF_INET;
		sa.sin_addr.s_addr	= netaddr.ip;
		sa.sin_port			= htons((u_short)netaddr.port);

		if (NSOX_SOCKET_ERROR == ::bind(hSocket, (struct sockaddr*)&sa, sizeof(sa)))
		{
				return socket_result<void>(nsox_last_error);
		}
		return socket_result<void>();
}
inline socket_result<void>	SocketUtil::connect(HSOCKET hSocket, const address& netaddr)
{
		ipaddr_type sa;
		memzero(sa);

		sa.sin_family		= AF_INET;
		sa.sin_addr.s_addr	= netaddr.ip;
		sa.sin_port			= htons(netaddr.port);

		if(NSOX_SOCKET_ERROR == ::connect(hSocket, (struct sockaddr*)&sa, sizeof(sa)))
		{
				return socket_result<void>(nsox_last_error);
		}
		return socket_result<void>();
}
inline socket_result<HSOCKET>	SocketUtil::accept(HSOCKET hSocket, address& netaddr)
{
		ipaddr_type sa;
		socklen_t len = sizeof(sa);
		memzero(sa);
		bool again = false;
		do{
				SET_FALSE(again);
				HSOCKET hNewSocket = ::accept(hSocket, (struct sockaddr*)&sa, &len);
				if (!VALID_SOCKET(hNewSocket)){
						int m_errno = nsox_last_error;
						if(m_errno == NU_EINTR || m_errno == NU_ECONNABORTED){
									again = true;
						}else{
									return socket_result<HSOCKET>(NSOX_INVALID_SOCKET, m_errno);
						}
				}else{
						netaddr.ip	 = sa.sin_addr.s_addr;
						netaddr.port = ntohs(sa.sin_port);
						return socket_result<HSOCKET>(hNewSocket);
				}
		}while(again);
}
inline socket_result<void>		SocketUtil::listen(HSOCKET hSocket)
{
		if (NSOX_SOCKET_ERROR == ::listen(hSocket, SOMAXCONN))
		{
				return socket_result<void>(nsox_last_error);
		}
		return socket_result<void>();
}
inline socket_result<HSOCKET>	SocketUtil::create(int netaddrFamily, int type, int protocol)
{
		HSOCKET hSocket = (int)::socket(netaddrFamily, type, protocol);
		if (!VALID_SOCKET(hSocket))
		{
				return socket_result<HSOCKET>(NSOX_INVALID_SOCKET, nsox_last_error);
		}
		return hSocket;
}
inline socket_result<boolean>	SocketUtil::isReuseAddress(HSOCKET hSocket)
{
		int option		= 0;
		socklen_t len	= sizeof(option);
		socket_result<boolean>	res = getsockopt(hSocket, SOL_SOCKET, SO_REUSEADDR, &option, &len);
		return socket_result<boolean>(option, res.__errno);
}

inline socket_result<void>	SocketUtil::setReuseAddress(HSOCKET hSocket, boolean on)
{
		int option = on ? 1 : 0;
		socket_result<boolean>	res = setsockopt(hSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
		return socket_result<void>(res.__errno);
}
inline socket_result<error_t>		SocketUtil::get_socket_error(HSOCKET hSocket)
{
		int error; socklen_t len = sizeof(error);
		socket_result<boolean>	res = getsockopt(hSocket, SOL_SOCKET, SO_ERROR, &error, &len);
		return socket_result<int>(error, res.__errno);
}


inline void SocketUtil::sleep_ms(uint32_t msecs)
{
		return sleep_us(msecs*1000);
}

inline void SocketUtil::sleep_us(uint32_t nusecs)
{
	struct timeval tval;
	for ( ; ; ) {
		tval.tv_sec = nusecs / 1000000;
		tval.tv_usec = nusecs % 1000000;
		if (select(0, NULL, NULL, NULL, &tval) == 0)
			break;/* all OK */
		if (errno == EINTR) 
				continue;
	}
}


#ifdef NSOX_WIN32
	#include "sh_win.inl"
#else
	#include "sh_unix.inl"
#endif

nsox_namespace_end


