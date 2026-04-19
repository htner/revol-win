#ifndef	__asyn_tcp_socket
#define __asyn_tcp_socket
#include "sockbuffer.h"

nsox_namespace_begin

template <typename t_asyn_socket>
class asyn_tcp_socket : public t_asyn_socket
{
public:
		typedef sox::BlockBuffer<sox::def_block_alloc_8k, 10240> Buffer8x10M; // 2M WOW!

		typedef SockBuffer<Buffer8x10M> InputBuffer;
		typedef SockBuffer<Buffer8x10M> OutputBuffer;

		virtual ~asyn_tcp_socket()
		{		
		}
		bool sendstream(const char* data, nu_size_t len)
		{
				try
				{
						char* bin = const_cast<char*>(data);
						if(t_asyn_socket::is_connected() && (!t_asyn_socket::is_closed()))
						{
								socket_result<nu_size_t> ret = __output_buffer.write(t_asyn_socket::get_socket_handler(), bin, len);

								if(ret.error() && ret.__errno != NU_EWOULDBLOCK){
										nsox::xlog_err(NsoxError, nsox_last_error, "error occured on write to socket : %s, close it",
												t_asyn_socket::get_peer_address().get_ip_addr().c_str());								
										close();
										onclose(ret.__errno);
										return false;
								}else{
										if(!__output_buffer.empty()){ //have not flush all data, should listen on write event
												t_asyn_socket::select(EVENT_WRITE, true);
										}
										return true;										
								}
						}else{
								nsox::xlog(NsoxError, "try to write on an unconnected socket");	
								return false;
						}			
				}catch(nsox::buffer_overflow& e) {
						nsox::xlog(NsoxDebug, "write to socket lead to buffer overflow %s", e.what());
						return false;
				}
				
		}
		virtual void close()
		{
				__input_buffer.erase();
				__output_buffer.erase();

				t_asyn_socket::select(EVENT_ALL, false);
				t_asyn_socket::close();		
		}

		// tell the data has arrived, return how many data has been fetched
		virtual int ondata(const char* data, nu_size_t len){return 0;}

		// tell tcp peer has closed the connection
		virtual void onclose(error_t err){};
protected:
		virtual void onread()
		{
				try{
						socket_result<nu_size_t> ret = __input_buffer.pump(t_asyn_socket::get_socket_handler());

						if(ret.error())
						{
								if(ret.__errno != NU_EWOULDBLOCK)
								{
										nsox::xlog_err(NsoxError, nsox_last_error, "error occured on reading from socket : %s, close it",
												t_asyn_socket::get_peer_address().get_ip_addr().c_str());								
										close(); // error, close socket
										onclose(ret.__errno);
								}	
						}else
						{
								if(ret == 0){		//peer close the connection						
										nsox::xlog(NsoxDebug, "asyn_tcp_socket socket been closed from %s",
												t_asyn_socket::get_peer_address().get_ip_addr().c_str());					
										close(); // close socket
										onclose(0); // occur event
								}else{
										__input_buffer.erase(0, ondata(__input_buffer.data(), __input_buffer.size()));	
								}
						}
				}catch (nsox::buffer_overflow& e) {
						nsox::xlog(NsoxDebug, "read from socket lead to buffer overflow %s", e.what());
				}
				
		}
		virtual void onwrite()
		{
				if(t_asyn_socket::is_connected() && (!t_asyn_socket::is_closed()))
				{
						socket_result<nu_size_t> ret = __output_buffer.flush(t_asyn_socket::get_socket_handler());
						if(ret.error() && ret.__errno != NU_EWOULDBLOCK){
								nsox::xlog_err(NsoxError, nsox_last_error, "error occured on reading from socket : %s, close it",
																		t_asyn_socket::get_peer_address().get_ip_addr().c_str());								
								close();
								onclose(ret.__errno);
						}else{
								if (__output_buffer.empty()){ //all data are out, do not need write event
										t_asyn_socket::select(EVENT_WRITE, false);
								}
						}					
				}
				else{
						nsox::xlog(NsoxError, "try to write on an unconnected socket");	
				}			
		}
protected:
		InputBuffer		__input_buffer;
		OutputBuffer	__output_buffer;
};

nsox_namespace_end
#endif


