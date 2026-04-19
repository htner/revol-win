

nsox_namespace_begin

inline serversocket::serversocket()
{
	
}
inline serversocket::~serversocket()
{
}
inline void serversocket::create()
{
		socket::create();

		socket_result<HSOCKET> res = SocketUtil::create(AF_INET,SOCK_STREAM);
		if(res.error()){
				nsox::xlog_err(NsoxFatal,res.__errno,"create tcp socket faild");
				throw socket_error("create tcp socket faild", res.__errno);
		}
		set_socket_handler((HSOCKET)res);
}
inline socket_result<HSOCKET>	serversocket::accept(address& addr)
{
		return SocketUtil::accept(get_socket_handler(), addr);	
}
inline void	serversocket::listen(const address& netaddr)
{
		socket_result<void> bindres = SocketUtil::bind(get_socket_handler(), netaddr);
		if(bindres.error()){
				nsox::xlog_err(NsoxFatal,bindres.__errno,"socket bind error on %s:%d", netaddr.get_ip_addr().c_str(), netaddr.port);
				throw nsox::socket_error("socket bind error", bindres.__errno);
				return;
		}
		socket_result<void> lisres = SocketUtil::listen(get_socket_handler());
		if(lisres.error()){
				nsox::xlog_err(NsoxFatal,lisres.__errno,"socket listern error on %s:%d", netaddr.get_ip_addr().c_str(), netaddr.port);
				throw nsox::socket_error("socket listen error", lisres.__errno);
		}
}
inline address serversocket::get_bind_addr()
{
		return SocketUtil::getLocalAddress(get_socket_handler());
}
inline bool	serversocket::is_reuse_addr()
{
		return SocketUtil::isReuseAddress(get_socket_handler());
}
inline void	serversocket::set_reuse_addr(boolean on)
{
		SocketUtil::setReuseAddress(get_socket_handler(), on);
}

nsox_namespace_end

