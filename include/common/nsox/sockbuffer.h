
#ifndef __SNOX_SOCKBUFFER_H_INCLUDE__
#define __SNOX_SOCKBUFFER_H_INCLUDE__

#include <stdexcept>
#include "blockbuffer.h"
#include "socketutil.h"
#include "nu_types.h"
#include "sockcomm.h"

namespace nsox
{

class buffer_overflow : public std::runtime_error
{
public:
	buffer_overflow(const std::string& _w): std::runtime_error(_w) {}
};

class buffer_overflow_all : public buffer_overflow
{
public:
	buffer_overflow_all(const std::string& _w): buffer_overflow(_w) {}
};

class FilterDefault
{
protected:
	void filterRead(char *, size_t)  {}
	void filterWrite(char *, size_t) {}
};

template < class BlockBufferClass, class FilterClass = FilterDefault >
struct SockBuffer : public BlockBufferClass, public FilterClass
{
	using BlockBufferClass::npos;
	using BlockBufferClass::freespace;
	using BlockBufferClass::blocksize;
	using BlockBufferClass::block;
	using BlockBufferClass::max_blocks;
	using BlockBufferClass::tail;
	using BlockBufferClass::size;
	using BlockBufferClass::increase_capacity;
	using BlockBufferClass::append;
	using BlockBufferClass::empty;
	using BlockBufferClass::data;
	using BlockBufferClass::erase;

	using FilterClass::filterWrite;
	
	typedef FilterClass Filter;
	// 0 : normal close ; >0 : current pump bytes
	socket_result<nu_size_t> pump(HSOCKET  hSocket, size_t n = npos)
	{
		if (freespace() < (blocksize() >> 1) 
				&& block() < max_blocks)
			// ignore increase_capacity result.
			increase_capacity(blocksize());

		size_t nrecv = freespace();
		if (nrecv == 0) throw buffer_overflow("input buffer overflow");
		if (n < nrecv) nrecv = n; // min(n, freespace());

		socket_result<nu_size_t> ret = SocketUtil::read(hSocket, tail(), nrecv);
		if (ret)
		{
				filterRead(tail(), ret);
				size(size() + ret);
		}
		return ret;
	}

	////////////////////////////////////////////////////////////////////
	// append into buffer only
	void write(char * msg, size_t size)
	{
		if (size == 0) return;

		filterWrite(msg, size);
		if (!append(msg, size))
			throw buffer_overflow_all("output buffer overflow [all]");
	}

	void write(HSOCKET hSocket, SockBuffer & buf)
	{
		write(hSocket, buf.data(), buf.size());
		buf.erase();
	}

	socket_result<nu_size_t> write(HSOCKET hSocket, char * msg, size_t size)
	{	// write all / buffered
		if (size == 0) return socket_result<nu_size_t>(0);

		filterWrite(msg, size);

		socket_result<nu_size_t> nsent(0);
		if (empty()){
				nsent = SocketUtil::write(hSocket, msg, size);
		}
		if ((!nsent.error())&& !append(msg + nsent, size - nsent))
		{
			// all or part append error .
			if (nsent > 0) throw buffer_overflow("output buffer overflow");
			else throw buffer_overflow_all("output buffer overflow [all]");
		}
		return nsent;
	}

	socket_result<nu_size_t> flush(HSOCKET hSocket, size_t n = npos)
	{
		size_t nflush = size(); if (n < nflush) nflush = n; // std::min(n, size());
		socket_result<nu_size_t> ret = SocketUtil::write(hSocket, data(), nflush);
		if(!ret.error())
				erase( 0, ret ); // if call flush in loop, maybe memmove here
		return ret;
	}
};

} // namespace sox

#endif // __SNOX_SOCKBUFFER_H_INCLUDE__
