#pragma once

#ifndef __NU_block_buffer_H__
#define __NU_block_buffer_H__


#define  BLOCK_BITS_4K 12

namespace nsox{


class buff_over_flow : public std::runtime_error
{
public:
	buff_over_flow(const std::string& _w): std::runtime_error(_w) {}
};

class buff_over_flow_all : public buff_over_flow
{
public:
	buff_over_flow_all(const std::string& _w): buff_over_flow(_w) {}
};


template<size_t BLOCKBITS>
class fix_memory_pool //blocksize must greater than 4
{
	enum{block_size = 1 << BLOCKBITS};
public:
	fix_memory_pool()
	{
		if(block_size < sizeof(int)){
			throw "block size must greater than sizeof int";
		}
		xp			= NULL;
		size		= 0;
		maxidlesize = 48;
	}
	virtual~ fix_memory_pool()
	{
			unsigned char* p = xp;
			while(p){
					int x = *(int*)p;
					delete[] p;
					p = (unsigned char*)x;
			}
	}
	static fix_memory_pool* inst()
	{
		static fix_memory_pool inst;
		return &inst;
	}
	void* alloc(size_t len)
	{
		if(size){
			unsigned char* temp = xp;
			xp = (unsigned char*)*(int*)xp; 
			size --;
			return temp;
		}else{
			return new unsigned char[block_size];
		}
	}
	void dealloc(void *p)
	{
		if(size == maxidlesize){
			delete[] (unsigned char*)p;
		}else{
			*(int*)p	= (int)xp;
			xp			= (unsigned char*)p;
			size		++;
		}
	}
protected:
	unsigned char*	 xp;
	size_t			 size;
	size_t			 maxidlesize;
};

template<size_t BLOCKBITS>
struct nu_buffer_alloc
{
	static void* alloc(int len)
	{
		return fix_memory_pool<BLOCKBITS>::inst()->alloc(len);
	}
	static void  dealloc(void* p)
	{
		return fix_memory_pool<BLOCKBITS>::inst()->dealloc(p);
	}
};

#define min(x,y) x<y?x:y


template<size_t BLOCKBITS, class Alloc = nu_buffer_alloc<BLOCKBITS> >
struct nu_buffer_item
{
private:
	Alloc __alloc;
	nu_buffer_item(const nu_buffer_item& val)
	{
	}
	nu_buffer_item& operator=(const nu_buffer_item& val)
	{
		return *this;
	}
public:
	enum{block_size = 1 << BLOCKBITS};

	nu_buffer_item()
	{
		_pos = _len = NULL;
		_data = (char*)__alloc.alloc(block_size);
	}

	virtual ~nu_buffer_item()
	{
		__alloc.dealloc(_data);
		_data = NULL;
	}
	inline size_t erase(size_t len)
	{
		size_t xlen = min(len, _len);
		_pos += xlen;
		_len -= xlen;
		return xlen;
	}
	inline size_t write(const char* d, size_t len)
	{
		size_t xlen = min(len, block_size-_pos-_len);
		memmove(_data+_pos+_len, d, xlen);
		_len += xlen;
		return xlen;
	}
	inline size_t	read(size_t offset, char* d, size_t len)
	{
		size_t xlen = min(len, _len - offset);
		memmove(d, _data + _pos + offset, xlen);
		return xlen;
	}
	inline size_t	read(size_t offset, std::string& str, size_t len)
	{
		size_t xlen = min(len, _len - offset);
		str.append(_data + _pos + offset, xlen);
		return xlen;
	}
	template<typename TInt>
		inline size_t read(size_t offset, TInt& var)
	{
		size_t xlen = min(sizeof(TInt), _len - offset);
		var = *(TInt*)(_data + _pos + offset);
		return xlen;
	}
	inline	size_t replace(size_t offset, const char* d, size_t len)
	{
		size_t xlen = min(len, _len - offset);
		memmove(_data + _pos + offset, d , xlen);
		return xlen;
	}

	inline const char* data(){
		return _data + _pos;
	}
	inline size_t	size(){
		return _len;
	}
	inline bool	bad(){
		return _pos == block_size;
	}
protected:
	char*	_data;
	size_t  _pos;
	size_t  _len;
};

#include <vector>

template< size_t BLOCKBITS, class buffer_item = nu_buffer_item<BLOCKBITS> >
class block_buffer
{
public:
	typedef buffer_item		item_type;
	typedef item_type*		item_type_ptr;

	struct position {
		size_t index;
		size_t offset;
	};

	enum{block_size = (1 << BLOCKBITS)};
	enum{npos = size_t(-1)};


	block_buffer(){__size = 0;}
	virtual ~block_buffer()
	{
		for(int i=0; i<__items.size(); ++i){
				delete __items[i];
		}
	}

	inline item_type_ptr item(size_t i)
	{
		return __items[i];
	}

	inline size_t read(size_t offset, std::string& str, size_t len)
	{
		position xpos  = find(offset);
		size_t   oldlen = len;
		while(xpos.index < __items.size()){				
			size_t xlen		= __items[xpos.index]->read(xpos.offset, 
				str, len);				
			len				-= xlen;
			if(len == 0)  break;

			xpos.offset = 0;
			xpos.index ++;
		}
		return oldlen - len;
	}

	inline size_t read(size_t offset, char* buffer, size_t len)
	{
		position xpos  = find(offset);
		size_t   oldlen = len;


		while(xpos.index < __items.size()){				
			size_t xlen		= __items[xpos.index]->read(xpos.offset, 
				buffer, len);
			buffer			+= xlen;				
			len				-= xlen;
			if(len == 0)  break;

			xpos.offset = 0;
			xpos.index ++;
		}
		return oldlen - len;
	}

	inline void write(const char* buffer, size_t len)
	{	
		if(empty()){create_new_item();}
		while(true)
		{
			item_type_ptr& item = __items.back();
			size_t xlen = item->write(buffer, len);
			__size += xlen;
			if(xlen == len){break;}
			create_new_item();
			len		 -=  xlen;
			buffer   +=  xlen;
		}					
	}



	inline size_t erase_front(size_t len)
	{
		if(len >= __size) { 						
			return clear(); 
		}

		size_t xlen = len = min(len, __size);
		while(!__items.empty()){
			item_type_ptr& item = __items.front();
			len -= item->erase(len);
			if(len){
				delete item;
				__items.erase(__items.begin());
			}else{
				break;
			}				
		}
		__size -= xlen;
		return xlen;
	}

	inline size_t erase_back(size_t len)
	{
		if(len >= __size) { 						
			return clear(); 
		}

		size_t xlen = len = min(len, __size);
		while(!__items.empty()){
			item_type_ptr& item = __items.back();
			len -= item.erase(len);
			if(len){
				delete item;
				__items.erase(__items.back());
			}else{
				break;
			}				
		}
		__size -= xlen;
		return xlen;
	}
	inline size_t clear()
	{
		size_t tmp = __size; 
		for(int i=0; i<__items.size(); ++ i)
		{
			delete __items[i];
		}
		__items.clear();
		__size = 0;
		return tmp;
	}
	inline bool   empty() const	 { return __size == NULL;}

	size_t	size() const{return __size;}


	bool	replace(size_t pos, const char* data, size_t len)

	{
		position xpos  = find(pos);
		size_t   oldlen = len;
		while(xpos.index < __items.size()){				
			size_t xlen		= __items[xpos.index]->replace(xpos.offset, 
															data, len);
			data			+= xlen;				
			len				-= xlen;
			if(len == 0)  break;

			xpos.offset = 0;
			xpos.index ++;
		}
		return !len;
	}
	position	find(size_t from)
	{
		position xpos;
		size_t len		= __items[0]->size();
		xpos.index		= from < len ? 0:  ((from - len) >> BLOCKBITS) + 1;
		xpos.offset		= from < len ? from : (from - len) & ~(block_size-1);
		return xpos;
	}	
protected:
	void create_new_item()
	{
		__items.push_back(new item_type);
	}
protected:
	std::vector<item_type_ptr>		__items;
	size_t							__position;
	size_t							__size;
};

typedef nu_buffer_alloc<BLOCK_BITS_4K>					def_buffer_alloc;
typedef nu_buffer_item<BLOCK_BITS_4K>					def_buffer_item;
typedef block_buffer<BLOCK_BITS_4K>						def_block_buffer;

}
#endif
