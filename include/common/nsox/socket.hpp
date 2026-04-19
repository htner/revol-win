

nsox_namespace_begin

inline socket::socket():__socket_fd(NSOX_INVALID_SOCKET)
{
		__is_blocking = true;
		__is_closed	  = false;	
}
inline socket::socket(HSOCKET hSocket)
{
		__socket_fd = hSocket;
}
inline socket::~socket()
{
		if(VALID_SOCKET(__socket_fd)){
				nsox::xlog(NsoxDebug, "socket is not correctly closed");
		}
}

inline bool	socket::is_block_mode()
{	
		return __is_blocking;
}                          
inline void	socket::set_block_mode(bool on)
{		
		SocketUtil::setBlocking(__socket_fd, on);
		__is_blocking = on;
}       
inline void	socket::set_socket_handler(HSOCKET hSocket)
{
		__socket_fd = hSocket;
}
inline void	socket::close()
{
		if(__socket_fd != NSOX_INVALID_SOCKET){
				SocketUtil::close(__socket_fd);
				__socket_fd = NSOX_INVALID_SOCKET;
				__is_closed = true;
		}
}
inline void	socket::onclose(error_t err)
{
}
inline void	socket::create()
{
		__is_closed	  = false;	
}
inline bool	socket::is_closed()
{
		return __is_closed;
}
nsox_namespace_end


