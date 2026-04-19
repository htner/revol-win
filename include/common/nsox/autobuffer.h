
#ifndef __NOX_AUTOBUFFER_H_INCLUDE__
#define __NOX_AUTOBUFFER_H_INCLUDE__

template<typename T>
struct auto_buffer
{
	T*		__data;
	size_t  __len;
	auto_buffer(int len){
		__len  = len;
		__data = new T[__len];
	}
	~auto_buffer(){
		delete[] __data;
	}
	operator T*(){
		return __data;
	}
	T& operator[](unsigned int pos)
	{
			return __data[pos];
	}
	size_t size(){
		return __len;
	}
};

#endif // __SNOX_BLOCKBUFFER_H_INCLUDE__
