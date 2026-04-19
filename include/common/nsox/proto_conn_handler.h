#pragma once
#ifndef __CACHED_HANDLER_H
#define __CACHED_HANDLER_H

#include "nsox/nsoxall.h"
#include "nrequest.h"

namespace nproto
{		
				class default_proto_conn : public nproto::connection
								  , public nsox::asyn_tcp_socket<nsox::asyn_tcp_peer>
				{
				public:
						default_proto_conn::default_proto_conn()
						{
								__proto_handler = NULL;
						}
						virtual ~default_proto_conn(){}

						void set_proto_handler(nproto::proto_handler* handler)
						{
								__proto_handler = handler;
						}

						void write_data(const char* p, nsox::nu_size_t len)
						{
								nsox::asyn_tcp_socket<nsox::asyn_tcp_peer>::sendstream(p, len);
						}
						void close_conn()
						{
								nsox::asyn_tcp_socket<nsox::asyn_tcp_peer>::close();
						}
						bool is_conn_close()
						{
								return nsox::asyn_tcp_socket<nsox::asyn_tcp_peer>::is_closed();
						}
						int	ondata(const char* p, nsox::nu_size_t len)
						{
								return __proto_handler->ondata(p,len,this);
						}
						void onclose(nsox::error_t err)
						{
								__proto_handler->onclose(err,this);
						}					

				protected:	
						nproto::proto_handler*	__proto_handler;
				};

}



#endif //__CACHED_HANDLER_H