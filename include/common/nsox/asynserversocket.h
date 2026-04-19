#ifndef	 __asyn_server_socket_h
#define  __asyn_server_socket_h

#include "asynsocket.h"
#include "listensocket.h"

nsox_namespace_begin


class asyn_server_socket : public asyn_socket<serversocket>
{
public:
		asyn_server_socket(){
				__handler_ptr = NULL;
		}
		~asyn_server_socket(){}
		void	set_handler(asyn_server_handler* handler_ptr){
				__handler_ptr = handler_ptr;
		}
		virtual void onread(){
				//while(1){
				address inetaddr;  int times = 50;
				do{
                     socket_result<HSOCKET> ret = serversocket::accept(inetaddr); //³É¹¦
						if(ret.error())
						{
								if(ret.__errno == NU_EWOULDBLOCK ||ret.__errno  == NU_EAGAIN)
								{
										break;
								}
								nsox::xlog_err(NsoxFatal, ret.__errno, "accept error");
								break;
						}else{
								onsocket(ret.__result, inetaddr);
						}
                                
                 }while( times -- );
		}
		virtual void onsocket(HSOCKET hsocket,const address& addr)
		{
				if(__handler_ptr){
						__handler_ptr->onsocket(hsocket, addr);
				}
		}

		void	listen(const address& address)
		{
				serversocket::listen(address);
				asyn_socket<serversocket>::select(EVENT_READ, true);
		}
		void	close()
		{
				asyn_socket<serversocket>::select(EVENT_ALL, false);
				serversocket::close();
		}
protected:
		asyn_server_handler * __handler_ptr;
};

nsox_namespace_end

#endif

