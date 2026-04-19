

inline socket_result<void>		SocketUtil::close(HSOCKET hSocket)
{
		if(NSOX_SOCKET_ERROR == ::close(hSocket))
		{
				return socket_result<void>(nsox_last_error);
		}
		return socket_result<void>();
}	
inline socket_result<void>		SocketUtil::closeRead(HSOCKET hSocket)
{
		if(NSOX_SOCKET_ERROR == ::shutdown(hSocket, SHUT_RD))
		{
					return socket_result<void>(nsox_last_error);
		}
		return socket_result<void>();
}
inline socket_result<void>		SocketUtil::closeWrite(HSOCKET hSocket)
{
		if(NSOX_SOCKET_ERROR == ::shutdown(hSocket, SHUT_WR))
		{
					return socket_result<void>(nsox_last_error);
		}
		return socket_result<void>();
}

inline socket_result<void> SocketUtil::setBlocking(HSOCKET hSocket, bool blocking)
{
		int fflags = ::fcntl(hSocket, F_GETFL);
		if (NSOX_SOCKET_ERROR == fflags)
						return socket_result<void>(nsox_last_error);

		if (blocking) 
					fflags &= ~O_NONBLOCK;
		else        
					fflags |= O_NONBLOCK;

		if (NSOX_SOCKET_ERROR == ::fcntl(hSocket, F_SETFL, fflags))
		{
				return socket_result<void>(nsox_last_error);
		}
		return socket_result<void>();
}
inline socket_result<boolean> SocketUtil::isBlocking(HSOCKET hSocket)
{
		int fflags = ::fcntl(hSocket, F_GETFL);
		if (NSOX_SOCKET_ERROR == fflags)
				return socket_result<boolean>(false,nsox_last_error);

		return ! (fflags & O_NONBLOCK);
}

inline socket_result<int> SocketUtil::waitevent(HSOCKET hSocket, int event, uint32_t timeout)
{
		struct pollfd fds[1];
		fds[0].fd		= hSocket;
		fds[0].events	= 0;

		if (event & EVENT_READ) fds[0].events |= POLLIN;
		if (event & EVENT_WRITE) fds[0].events |= POLLOUT;

		int ret = ::poll(fds, 1, timeout);
		if(ret > 0){
				int event = 0;
				if (fds[0].revents & POLLIN) event |= EVENT_READ;
				if (fds[0].revents & POLLOUT) event |= EVENT_WRITE;
				return socket_result<int>(event);
		}else if(ret == 0){
				return socket_result<int>(ret, NU_ETIMEDOUT);
		}else{
				return socket_result<int>(ret, nsox_last_error);
		}

}


inline void	SocketUtil::format_error(error_t err, std::string& str)
{
		str = strerror(err);
}
inline nu_time_t	SocketUtil::get_now_time()
{
		struct timeval stv;
		struct timezone stz;
		gettimeofday(&stv,&stz);

		return  stv.tv_sec * 1000 + stv.tv_usec/1000;
}

