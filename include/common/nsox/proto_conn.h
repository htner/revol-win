#pragma once
#ifndef __CACHED_HANDLER_H
#define __CACHED_HANDLER_H

#include "nsoxall.h"
#include "nu_request.h"


namespace nproto
{		
				class default_proto_conn : public nproto::connection
								  		 , public nsox::asyn_tcp_socket<nsox::asyn_tcp_peer>
				{
				public:
						default_proto_conn()
						{
								__proto_handler = NULL;
						}
						virtual ~default_proto_conn()
						{
						}
						void set_proto_handler(nproto::proto_handler* handler)
						{
								__proto_handler = handler;
						}
						bool write_data(const char* p, nsox::nu_size_t len)
						{
								return nsox::asyn_tcp_socket<nsox::asyn_tcp_peer>::sendstream(p, len);
						}
						void close_conn()
						{
								nsox::asyn_tcp_socket<nsox::asyn_tcp_peer>::close();
						}
						bool is_conn_close()
						{
								return nsox::asyn_tcp_socket<nsox::asyn_tcp_peer>::is_closed();
						}
						bool is_conn_alive()
						{
								return nsox::asyn_tcp_socket<nsox::asyn_tcp_peer>::is_connected();
						}
						virtual int	ondata(const char* p, nsox::nu_size_t len)
						{
								return __proto_handler->ondata(p,len,this);
						}
						virtual void onclose(nsox::error_t err)
						{
								__proto_handler->onclose(err,this);
						}					
				protected:	
						nproto::proto_handler*	__proto_handler;
				};

				class default_proto_client : public nproto::connection
						, public nsox::asyn_tcp_socket<nsox::asyn_tcp_client>
				{
				public:
						default_proto_client()
						{
								__proto_handler = NULL;
						}
						virtual ~default_proto_client()
						{
						}
						void set_proto_handler(nproto::proto_handler* handler)
						{
								__proto_handler = handler;
						}
						bool write_data(const char* p, nsox::nu_size_t len)
						{
								return nsox::asyn_tcp_socket<nsox::asyn_tcp_client>::sendstream(p, len);
						}
						void close_conn()
						{
								nsox::asyn_tcp_socket<nsox::asyn_tcp_client>::close();
						}
						bool is_conn_close()
						{
								return nsox::asyn_tcp_socket<nsox::asyn_tcp_client>::is_closed();
						}
						bool is_conn_alive()
						{
								return nsox::asyn_tcp_socket<nsox::asyn_tcp_client>::is_connected();
						}
						virtual int	ondata(const char* p, nsox::nu_size_t len)
						{
								return __proto_handler->ondata(p,len,this);
						}
						virtual void onclose(nsox::error_t err)
						{
								__proto_handler->onclose(err,this);
						}	

						virtual void on_connect_success()
						{
								__proto_handler->oncreate(this);
						}
				protected:	
						nproto::proto_handler*	__proto_handler;
				};


				class sync_proto_client : public nsox::tcpclient
				{
				private:
						typedef sox::BlockBuffer<sox::def_block_alloc_4k, 1024> Buffer4M; // 1M WOW!
						typedef nsox::SockBuffer<Buffer4M> InputBuffer;
						
						InputBuffer __input_buffer;
				public:
						sync_proto_client(){};
						virtual ~sync_proto_client(){};

						template<class TReq>
						void write_request( TReq& req)
						{
								sox::PackBuffer pb;
								sox::Pack pk(pb);
								req.URI = TReq::uri;
								req.pack(pk);
								pk.replace_uint32(0, pk.size());
								write(pb.data(),pb.size());
						}
						template<class TReq>
						bool read_request( TReq& req)
						{	
								while(true)
								{
										nsox::socket_result<nsox::nu_size_t> xx = __input_buffer.pump(get_socket_handler());
										if(xx.error() || xx.__result == 0)
										{
												close();
												return false;
										}

										uint32_t len			= __input_buffer.size();
										char* data		= __input_buffer.data();

										if((len<sizeof(uint32_t)) || (nproto::request::peek_len(data)>len))
										{
												continue;
										}

										bool  success = true;
										int		 plen = nproto::request::peek_len(data);
										uint32_t uri  = nproto::request::peek_uri(data);
										if(uri != TReq::uri){
												success = false;
										}else{
												sox::Unpack upk(data,len);
												try{
														req.unpack(upk);
												}catch(sox::UnpackError& err){
														nsox::xlog(NsoxDebug, "unpack request error , uri:%d", uri);		
														success = false;
												}
										}
										__input_buffer.erase(0, plen);
										return success;
								}
						}
				};
}



#endif //__CACHED_HANDLER_H
