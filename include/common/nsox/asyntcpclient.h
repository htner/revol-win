#ifndef __asyn_tcp_client_h
#define  __asyn_tcp_client_h
#include "asynsocket.h"

nsox_namespace_begin
class asyn_tcp_client : public asyn_socket<tcpclient>
{
public:
		asyn_tcp_client(){
				__timer_id = -1;
		}
		virtual	~asyn_tcp_client()
		{		
		}
		virtual void	connect(const address& addr, const nu_time_t& timeout = INFINITY_TIME)
		{
				__tmpaddr = addr;

				socket_result<void> res = SocketUtil::connect(get_socket_handler(), addr);
				if(!res.error()){
						set_connected(true);
						__inner_onconnected(); //³É¹¦
						return;
				}
				int socket_error = res.__errno;
				if (!(socket_error == NU_EINPROGRESS || socket_error == NU_EWOULDBLOCK)){
						__inner_connect_error(socket_error);
						return;
				}
				if(timeout != INFINITY_TIME)
				{
						__timer_id = asyn_socket<tcpclient>::select_timeout(timeout);
				}
				asyn_socket<tcpclient>::select(EVENT_READ|EVENT_WRITE|EVENT_EXCPTION, true);
				__is_connecting = true;
		}
		virtual void timeout()
		{		
				__timer_id = -1;
				__inner_time_out();
		}
		virtual void onevent(int ev){
				if(__is_connecting)//socket is on connectiong
				{
						__is_connecting	 = false;
						asyn_socket<tcpclient>::select(EVENT_WRITE|EVENT_READ|EVENT_EXCPTION, false);

						if((ev & EVENT_READ) || (ev & EVENT_WRITE) || (ev & EVENT_EXCPTION)) //windows connect error leads to exception event
						{
								error_t	error = SocketUtil::get_socket_error(get_socket_handler());					
								if (error){									
										__inner_connect_error(error);				
								}else{
										set_connected(true);
										__inner_onconnected();
								}
						}else{
								nsox::xlog(NsoxError, "onevent : code arrrive an uncorrected place");
						}
						if(__timer_id !=  -1) remove_timeout(__timer_id);
						
				}else{
						asyn_socket<tcpclient>::onevent(ev);			
				}
		}
		virtual	void on_connect_success(){}
		virtual	void on_connect_timeout(){}
		virtual void on_connect_error(error_t error){}

		void __inner_time_out()
		{
				asyn_socket<tcpclient>::select(EVENT_READ|EVENT_WRITE|EVENT_EXCPTION, false);
				nsox::xlog(NsoxError, "connect timeout to %s:%d", __tmpaddr.get_ip_addr().c_str(), __tmpaddr.port);

				__is_connecting = false;
				on_connect_timeout();	
		}
		void __inner_connect_error(int error)
		{
				asyn_socket<tcpclient>::select(EVENT_READ|EVENT_WRITE|EVENT_EXCPTION, false);
				nsox::xlog_err(NsoxError, error,  "connect error to %s:%d", __tmpaddr.get_ip_addr().c_str(), __tmpaddr.port);

				__is_connecting = false;
				on_connect_error(error);
		}
		void __inner_onconnected()//onconnected, begin to listen read event
		{
				nsox::xlog(NsoxDebug, "connect successfully to %s:%d", __tmpaddr.get_ip_addr().c_str(), __tmpaddr.port);

				asyn_socket<tcpclient>::select(EVENT_READ|EVENT_WRITE|EVENT_EXCPTION, false);
				select(EVENT_READ, true);
				on_connect_success();
		}
protected:
		bool __is_connecting;
		timer_id  __timer_id;
		address __tmpaddr;
};

nsox_namespace_end
#endif

