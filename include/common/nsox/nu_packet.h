#ifndef  __NSOX_PACKET_H__
#define  __NSOX_PACKET_H__

#include "block_buffer.h"

namespace nsox
{

struct PacketError : public std::runtime_error {
		PacketError(const std::string & w) :
std::runtime_error(w) {
}
};

struct PackError : public PacketError {
		PackError(const std::string & w) :
PacketError(w) {
}
};

struct UnpackError : public PacketError {
		UnpackError(const std::string & w) :
PacketError(w) {
}
};

template<size_t BLOCKBITS = BLOCK_BITS_4K, class BLOCKBUFF = block_buffer<BLOCKBITS> >
class pack_buffer {
public:
		size_t size() const {
				return bb.size();
		}
		void append(const char * data, size_t size) {
				bb.write(data,size);
		}
		void append(const char * data) {
				append(data,:: strlen(data));}
		void replace(size_t pos, const char * rep, size_t n) {
				if (bb.replace(pos, rep, n)) return;
				throw PackError("replace buffer overflow");
		}

public:
		// use big-block. more BIG? MAX 64K*16k = 1G
		typedef BLOCKBUFF BB;
		BB bb;
};

#if defined(__i386__)||defined(WIN32)

#define XHTONS
#define XHTONL
#define XHTONLL

#else /* big end */

inline uint16_t XHTONS(uint16_t i16) {
		return ((i16 << 8) | (i16 >> 8));
}
inline uint32_t XHTONL(uint32_t i32) {
		return ((uint32_t(XHTONS(i32)) << 16) | XHTONS(i32>>16));
}
inline uint64_t XHTONLL(uint64_t i64) {
		return ((uint64_t(XHTONL((uint32_t)i64)) << 32) |XHTONL((uint32_t(i64>>32))));
}

#endif /* __i386__ */

#define XNTOHS XHTONS
#define XNTOHL XHTONL
#define XNTOHLL XHTONLL


template<size_t BLOCKBITS = BLOCK_BITS_4K, class BlockBuff = block_buffer<BLOCKBITS> >
class pack
{
private:
		pack (const pack & o);
		pack & operator = (const pack& o);
public:
		static uint16_t xhtons(uint16_t i16) { return XHTONS(i16); }
		static uint32_t xhtonl(uint32_t i32) { return XHTONL(i32); }
		static uint64_t xhtonll(uint64_t i64) { return XHTONLL(i64); }

		// IMPORTANT remember the buffer-size before pack. see data(), size()
		// reserve a space to replace packet header after pack parameter
		// sample below: OffPack. see data(), size()
		pack(pack_buffer<BLOCKBITS> & pb) : m_buffer(pb)
		{			
		}


		pack & push(const void * s, size_t n) { m_buffer.append((const char *)s, n); return *this; }
		pack & push(const void * s)           { m_buffer.append((const char *)s); return *this; }

		pack & push_uint8(uint8_t u8)	 { return push(&u8, 1); }
		pack & push_uint16(uint16_t u16) { u16 = xhtons(u16); return push(&u16, 2); }
		pack & push_uint32(uint32_t u32) { u32 = xhtonl(u32); return push(&u32, 4); }
		pack & push_uint64(uint64_t u64) { u64 = xhtonll(u64); return push(&u64, 8); }

		//Pack & push_varstr(const Varstr & vs)     { return push_varstr(vs.data(), vs.size()); }
		pack & push_varstr(const void * s)        { return push_varstr32(s, strlen((const char *)s)); }
		pack & push_varstr(const std::string & s) { return push_varstr32(s.data(), s.size()); }
		pack & push_varstr32(const void * s, size_t len)
		{
				if (len > 0xFFFFFFFF) throw PackError("push_varstr32: varstr too big");
				return push_uint32(uint32_t(len)).push(s, len);
		}
		pack & push_varstr32(const std::string& s)
		{
				if (len > 0xFFFFFFFF) throw PackError("push_varstr32: varstr too big");
				return push_uint32(uint32_t(len)).push(s.data(), s.size());
		}

		virtual ~pack() {}
public:
		// replace. pos is the buffer offset, not this Pack m_offset
		size_t replace(size_t pos, const void * data, size_t rplen) {
				m_buffer.replace(pos, (const char*)data, rplen);
				return pos + rplen;
		}
		size_t replace_uint8(size_t pos, uint8_t u8)    { return replace(pos, &u8, 1); }
		size_t replace_uint16(size_t pos, uint16_t u16) {
				u16 = xhtons(u16);
				return replace(pos, &u16, 2);
		}
		size_t replace_uint32(size_t pos, uint32_t u32) {
				u32 = xhtonl(u32);
				return replace(pos, &u32, 4);
		}
protected:
		pack_buffer<BLOCKBITS, BlockBuff>& m_buffer;
		size_t m_offset;
};


template<size_t BLOCKBITS= BLOCK_BITS_4K,  class BlockBuff = block_buffer<BLOCKBITS> >
class unpack_buffer
{
public:
		enum{npos = (size_t)-1};

		unpack_buffer(BlockBuff& lb, size_t off=0, size_t len = npos):xlb(lb)
		{
				size	= min(lb.size() - off, len);
				pos		= xlb.find(off);
		}

		static uint16_t xhton(uint16_t i16) { return XHTONS(i16); }
		static uint32_t xhton(uint32_t i32) { return XHTONL(i32); }
		static uint64_t xhton(uint64_t i64) { return XHTONLL(i64); }

		template <typename T>
		T pop_int() //in order to read faster
		{
				if(size < sizeof(T)) throw UnpackError("pop_int: not enough data");

				T var;
				uint8_t c = xlb.item(pos.index)->read(pos.offset, var);
				if(c < sizeof(T)){	
							char buff[sizeof(T)];
							xlb.item(pos.index)->read(pos.offset, buff, c);
							xlb.item(++pos.index)->read(0, buff+c, sizeof(T) - c);
							var = *(T*)buff;
							pos.offset = sizeof(T) -c;
				}else{
							pos.offset += c;
				}
				size -= sizeof(T);
				return var;
		}
		void pop_str(std::string& str, uint32_t len)
		{
				if(size < len) throw UnpackError("pop_str: not enough data");

				size -= len;

				while(true){
						size_t xlen = xlb.item(pos.index)->read(pos.offset, str, len);
						len -= xlen;
						if(len == 0){
								pos.offset += xlen;
								break;
						}
						pos.index	++ ;
						pos.offset	= 0;
				}
		}

		size_t length(){
				return size;
		}
protected:
		typename BlockBuff::position pos;
		size_t size;
		BlockBuff& xlb;
};

template<size_t BLOCKBITS = BLOCK_BITS_4K, class BlockBuff = block_buffer<BLOCKBITS> >
class unpack
{
public:
		static uint16_t xntohs(uint16_t i16)  { return XNTOHS(i16); }
		static uint32_t xntohl(uint32_t i32)  { return XNTOHL(i32); }
		static uint64_t xntohll(uint64_t i64)  { return XNTOHLL(i64); }

		unpack_buffer<BLOCKBITS, BlockBuff> xupb;
		unpack(BlockBuff& upb,size_t off=0, size_t len = BlockBuff::npos):xupb(upb,off,len)
		{				
		}
		virtual ~unpack() {  }

		uint8_t pop_uint8(){
				return xupb.template pop_int<uint8_t>();
		}

		uint16_t pop_uint16(){
				return xntohs(xupb.template pop_int<uint16_t>());
		}

		uint32_t pop_uint32(){
				return xntohl(xupb.template pop_int<uint32_t>());
		}

		void	 pop_str(std::string& str)
		{
				 uint32_t len = pop_uint32();
				 xupb.pop_str(str,len);
		}
};

template<size_t BLOCKBITS, class BlockBuff>
inline pack<BLOCKBITS, BlockBuff> & operator << (pack<BLOCKBITS> & p, bool sign)
{
		p.push_uint8(sign ? 1 : 0);
		return p;
}

template<size_t BLOCKBITS, class BlockBuff>
inline pack<BLOCKBITS, BlockBuff> & operator << (pack<BLOCKBITS, BlockBuff> & p, uint16_t  i16)
{
		p.push_uint16(i16);
		return p;
}

template<size_t BLOCKBITS, class BlockBuff>
inline pack<BLOCKBITS, BlockBuff> & operator << (pack<BLOCKBITS, BlockBuff> & p, uint32_t  i32)
{
		p.push_uint32(i32);
		return p;
}

template<size_t BLOCKBITS, class BlockBuff>
inline pack<BLOCKBITS, BlockBuff> & operator << (pack<BLOCKBITS, BlockBuff> & p, uint64_t  i64)
{
		p.push_uint64(i64);
		return p;
}

template<size_t BLOCKBITS, class BlockBuff>
inline pack<BLOCKBITS, BlockBuff> & operator << (pack<BLOCKBITS, BlockBuff> & p, const std::string & str)
{
		p.push_str(str);
		return p;
}




// pair.first helper
// XXX std::map::value_type::first_type unpack 需要特别定义
template<size_t BLOCKBITS, class BlockBuff>
inline const unpack<BLOCKBITS, BlockBuff> & operator >> (const unpack<BLOCKBITS, BlockBuff> & p, uint32_t & i32)
{
		i32 =  p.pop_uint32();
		return p;
}

template<size_t BLOCKBITS, class BlockBuff>
inline const unpack<BLOCKBITS, BlockBuff> & operator >> (const unpack<BLOCKBITS, BlockBuff> & p, uint64_t & i64)
{
		i64 =  p.pop_uint64();
		return p;
}

template<size_t BLOCKBITS, class BlockBuff>
inline const unpack<BLOCKBITS, BlockBuff> & operator >> (const unpack<BLOCKBITS, BlockBuff> & p, std::string & str)
{
		// XXX map::value_type::first_type
		str = p.pop_varstr();
		return p;
}

template<size_t BLOCKBITS, class BlockBuff>
inline const unpack<BLOCKBITS, BlockBuff> & operator >> (const unpack<BLOCKBITS, BlockBuff> & p, uint16_t & i16)
{
		i16 =  p.pop_uint16();
		return p;
}

template<size_t BLOCKBITS, class BlockBuff>
inline const unpack<BLOCKBITS, BlockBuff> & operator >> (const unpack<BLOCKBITS, BlockBuff> & p, bool & sign)
{
		sign =  (p.pop_uint8() == 0) ? false : true;
		return p;
}


template <class T1, class T2>
inline std::ostream& operator << (std::ostream& s, const std::pair<T1, T2>& p)
{
		s << p.first << '=' << p.second;
		return s;
}

template <class T1, class T2, size_t BLOCKBITS, class BlockBuff>
inline pack<BLOCKBITS>& operator << (pack<BLOCKBITS>& s, const std::pair<T1, T2>& p)
{
		s << p.first << p.second;
		return s;
}

template <class T1, class T2, size_t BLOCKBITS, class BlockBuff>
inline const unpack<BLOCKBITS, BlockBuff>& operator >> (const unpack<BLOCKBITS, BlockBuff>& s, std::pair<const T1, T2>& p)
{
		const T1& m = p.first;
		T1 & m2 = const_cast<T1 &>(m);
		s >> m2 >> p.second;
		return s;
}

/*
// vc . only need this
template <class T1, class T2>
inline const sox::Unpack& operator>>(const sox::Unpack& s, std::pair<T1, T2>& p)
{
s >> p.first;
s >> p.second;
return s;
}
*/

// container marshal helper
template < typename ContainerClass, size_t BLOCKBITS>
inline void marshal_container(pack<BLOCKBITS> & p, const ContainerClass & c)
{
		p.push_uint32(uint32_t(c.size())); // use uint32 ...
		for (typename ContainerClass::const_iterator i = c.begin(); i != c.end(); ++i)
				p << *i;
}

template < typename OutputIterator, size_t BLOCKBITS, class BlockBuff>
inline void unmarshal_container(const unpack<BLOCKBITS, BlockBuff> & p, OutputIterator i)
{
		for (uint32_t count = p.pop_uint32(); count > 0; --count)
		{
				typename OutputIterator::container_type::value_type tmp;
				p >> tmp;
				*i = tmp;
				++i;
		}
}

//add by heiway 2005-08-08
//and it could unmarshal list,vector etc..
template < typename OutputContainer, size_t BLOCKBITS, class BlockBuff>
inline void unmarshal_containerEx(const unpack<BLOCKBITS, BlockBuff> & p, OutputContainer & c)
{
		for(uint32_t count = p.pop_uint32() ; count >0 ; --count)
		{
				typename OutputContainer::value_type tmp;
				tmp.unmarshal(p);
				c.push_back(tmp);
		}
}

template < typename ContainerClass >
inline std::ostream & trace_container(std::ostream & os, const ContainerClass & c, char div='\n')
{
		for (typename ContainerClass::const_iterator i = c.begin(); i != c.end(); ++i)
				os << *i << div;
		return os;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

typedef pack_buffer<>	 default_pack_buffer;
typedef unpack_buffer<>  default_unpack_buffer;
typedef pack<>			 default_pack;
typedef unpack<>		 default_unpack;

}
#endif
