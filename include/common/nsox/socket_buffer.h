#ifndef __NSOX_SOCKET_BUFFER_H__
#define __NSOX_SOCKET_BUFFER_H__

#include "block_buffer.h"


namespace nsox{

template<size_t BLOCKBITS, class Alloc = nu_buffer_alloc<BLOCKBITS> >
struct socket_buffer_item : public nu_buffer_item<BLOCKBITS, Alloc>
{
		typedef nu_buffer_item<BLOCKBITS, Alloc> parent;
public:
		inline size_t freespace()
		{
				return parent::block_size - parent::_len - parent::_pos;
		}
		inline char* tail()
		{
				return parent::_data + parent::_pos;
		}
		inline socket_result<nu_size_t> read(HSOCKET hSocket){
				if(freespace() == 0) throw buff_over_flow("buffer overflow");

				socket_result<nu_size_t> ret = SocketUtil::read(hSocket,tail(),freespace());
				if(ret){
					parent::_len += ret;
				}
				return ret;
		}
};

template<size_t BLOCKBITS, class buffer_item = socket_buffer_item<BLOCKBITS> >
class socket_buffer : public  block_buffer<BLOCKBITS,buffer_item>
{
		typedef block_buffer<BLOCKBITS,buffer_item> parent;
public:
		inline size_t freespace()
		{
				if(parent::empty()) return 0;
				return parent::__items.back()->freespace();
		}

		inline void increase_capacity()
		{
				parent::create_new_item();
		}
		inline	void  append(const char* data, size_t len)
		{
				parent::write(data, len);
		}
		inline socket_result<nu_size_t> pump(HSOCKET  hSocket, size_t n = parent::npos)		
		{		
				while(true){
						if(!freespace()) increase_capacity();

						socket_result<nu_size_t> ret = parent::__items.back()->read(hSocket);
						if(ret){
								parent::__size += ret;
						}else{
								return ret;
						}						
					}
		
}
		
		inline socket_result<nu_size_t> write(HSOCKET hSocket, char * msg, size_t size)
		{	// write all / buffered
				if (size == 0) return socket_result<nu_size_t>(0);

				socket_result<nu_size_t> nsent(0);
				if (parent::empty()){
						nsent = SocketUtil::write(hSocket, msg, size);
				}
				if(!nsent.error())
				{
						append(msg + nsent, size - nsent);
				}
				return nsent;
		}

		socket_result<nu_size_t> flush(HSOCKET hSocket, size_t n = parent::npos)
		{
				size_t nflush = parent::size(); if (n < nflush) nflush = n; // std::min(n, size());

				size_t total = 0;

				socket_result<nu_size_t> ret(0);
				for(int i=0; i<parent::__items.size(); ++ i)
				{
						ret = SocketUtil::write(hSocket, 
												parent::__items[i]->data(), parent::__items[i]->size());
						if(ret.error()) break;
						total += ret;
				}
				if(total) parent::erase_front(total);

				return ret;
		}

};

typedef socket_buffer_item<BLOCK_BITS_4K>							def_socket_buf_item;
typedef socket_buffer<BLOCK_BITS_4K>								def_socket_buffer;


}
#endif//__NSOX_SOCKET_BUFFER_H__