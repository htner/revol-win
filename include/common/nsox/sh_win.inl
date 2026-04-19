
#pragma comment(lib,"ws2_32")


inline socket_result<void>	SocketUtil::close(HSOCKET hSocket)
{
		if(::closesocket(hSocket) == NSOX_SOCKET_ERROR)
		{
				return socket_result<void>(nsox_last_error);
		}
		return socket_result<void>();
}	
inline socket_result<void>	SocketUtil::closeRead(HSOCKET hSocket)
{
		if(::shutdown(hSocket,SD_RECEIVE) == NSOX_SOCKET_ERROR)
		{
				return socket_result<void>(nsox_last_error);
		}
		return socket_result<void>();
}
inline socket_result<void>	SocketUtil::closeWrite(HSOCKET hSocket)
{
		if(::shutdown(hSocket,SD_SEND) == NSOX_SOCKET_ERROR)
		{
				return socket_result<void>(nsox_last_error);
		}
		return socket_result<void>();
}

inline socket_result<void> SocketUtil::setBlocking(HSOCKET hSocket, bool blocking)
{
		unsigned long op = (blocking ? 0 : 1);
		if (NSOX_SOCKET_ERROR == ::ioctlsocket(hSocket, FIONBIO, &op))
		{
					return socket_result<void>(nsox_last_error);
		}
		return socket_result<void>();
}
inline socket_result<boolean> SocketUtil::isBlocking(HSOCKET hSocket)
{
			throw "you should nerver been here";
			return false;
}


inline void	SocketUtil::format_error(error_t err, std::string& str)
{

//	USES_CONVERSION;
	LPSTR lpBuffer = NULL;
	int count = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,  err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
		(LPSTR) &lpBuffer, 0, NULL);

	if (!count)
	{
		// Is it a network-related error?
			HMODULE hDll = LoadLibraryEx(TEXT("netmsg.dll"), NULL, DONT_RESOLVE_DLL_REFERENCES);
			if (hDll != NULL)
			{
				FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM,
					hDll, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
					(LPSTR) &lpBuffer, 0, NULL);
				FreeLibrary(hDll);
			}
	}	
	if(lpBuffer) {
			str = lpBuffer;
			LocalFree(lpBuffer);
	}
}


inline socket_result<int> SocketUtil::waitevent(HSOCKET hSocket, int event, uint32_t timeout)
{
	fd_set rs,ws,es;
	FD_ZERO(&rs); FD_ZERO(&ws); FD_ZERO(&es);
	if (event & EVENT_READ)		FD_SET(hSocket, &rs);
	if (event & EVENT_WRITE)	FD_SET(hSocket, &ws);
	if (event & EVENT_EXCPTION)	FD_SET(hSocket, &es);

	struct timeval tv;
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = 1000 * (timeout % 1000);
	int ret = ::select(hSocket + 1, &rs, &ws, &es, timeout?&tv:NULL);

	if(ret > 0){
			int event = 0;
			if (FD_ISSET(hSocket, &rs)) event |= EVENT_READ;
			if (FD_ISSET(hSocket, &ws)) event |= EVENT_WRITE;
			if (FD_ISSET(hSocket, &es)) event |= EVENT_EXCPTION;
			return socket_result<int>(event);
	}else if(ret == 0){
			return socket_result<int>(ret, NU_ETIMEDOUT);
	}else{
			return socket_result<int>(ret, nsox_last_error);
	}
}

inline nu_time_t	SocketUtil::get_now_time()
{
		return GetTickCount();
}

