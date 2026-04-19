#ifndef nsox_clientsocket_h
#define nsox_clientsocket_h
#include "nu_except.h"
#include "tcpsocket.h"

nsox_namespace_begin

class tcpclient :  public tcpsocket
{
public:
	tcpclient()
	{			
	}
    /*
     *  connect to server, if connect timeout, throw timeouterr, other
     *  error will throw sockerror. this is an sysn operation
     */
	void connect(const address& addr, nu_time_t timeout = INFINITY_TIME)
	{
		bool isblockabe = is_block_mode();
		set_block_mode(false);

		error_t error_no = 0;

		socket_result<void> connres =  SocketUtil::connect(get_socket_handler(), addr);
		if(connres.error())
		{
				error_no = connres.__errno;
				if (!(error_no == NU_EINPROGRESS || error_no == NU_EWOULDBLOCK)) // connecting in background
				{
						// should log the error?
						throw connect_error("nonblocking connect error");
				}		
				bool again = false; 
				do{
								SET_FALSE(again);
								socket_result<int> waitres = SocketUtil::waitevent(
															 get_socket_handler(),
															 EVENT_WRITE|EVENT_READ|EVENT_EXCPTION, timeout);
								error_no = waitres.error() ? waitres.__errno : 
										   (error_t)SocketUtil::get_socket_error(get_socket_handler());

								if(error_no == 0) break; //success

								if(error_no == EINTR){
										again = true;		
								}else if(error_no == NU_ETIMEDOUT){
										set_block_mode(isblockabe);

										nsox::xlog(NsoxError,
												"tcpclient::connect() to server %s:%d timeout ", 
												addr.get_ip_addr().c_str(), addr.port);

										throw connect_timeout();
								}else{ 
										set_block_mode(isblockabe);

										nsox::xlog_err(NsoxError, error_no,
											         "tcpclient::connect() to server %s:%d  ",
													  addr.get_ip_addr().c_str(), addr.port);

										throw connect_error(error_no);
								}
				}while(again);
		}
		set_block_mode(isblockabe);
		__onconnected(addr); // connect successfully;
	}

	virtual void onconnected(){}
private:
	void __onconnected(const address& addr) //internal function, don't call outerside
	{
			nsox::xlog(NsoxDebug, "tcp client sucessfully connected to server %s:%d ", addr.get_ip_addr().c_str(), addr.port);
			set_connected(true);
			onconnected();
	}
};    

nsox_namespace_end

#endif


