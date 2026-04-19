#pragma once

#ifndef __CACHED_SERVER_H
#define  __CACHED_SERVER_H

#include "nu_request.h"
namespace nproto
{
				template<class TCONN>
				class default_app_server : public nsox::nsox_tcp_server<TCONN>
				{
				public:
						default_app_server(void){
								__proto_handler = NULL;
						}

						default_app_server(int port, const char* addr = NULL) :
									nsox::nsox_tcp_server<TCONN>(port, addr)
						{
								__proto_handler = NULL;
						}
						virtual ~default_app_server(void){

						}
						void set_proto_handler(nproto::proto_handler* handler)
						{
								__proto_handler = handler;
						}
						virtual void onsocket(TCONN* conn)
						{				
								conn->set_proto_handler(__proto_handler);
								nsox::nsox_tcp_server<TCONN>::onsocket(conn);
								__proto_handler->oncreate(conn);
						}
				protected:
						nproto::proto_handler* __proto_handler;
				};
}


#endif //__CACHED_SERVER_H

