#ifndef nsox_socket_common_h
#define nsox_socket_common_h

#include <exception>
#include <string>


#include "nu_types.h"

namespace nsox{

template <typename t_result>
struct socket_result{
	socket_result(t_result res, error_t err){
			__result = res;
			__errno	 = err;
	}
	socket_result(t_result res) : __errno(0){
			__result = res;
	}
	operator t_result(){
			return __result;
	}
	bool error(){
			return  __errno != 0;
	}
	t_result __result;
	error_t  __errno;
};
template <>
struct socket_result<void>
{
	socket_result(error_t err = 0){
			__errno = err;
	}
	bool error(){
			return  __errno != 0;
	}
	error_t  __errno;
};

struct address {
		address(uint16_t port = 0, u_long ipAddr = INADDR_ANY)
		{	
				this->ip	 = ipAddr;
				this->port	 = port;
		}
		address(uint16_t port , const char* ipAddr)
		{
				this->ip	= INADDR_ANY;
				if(NULL != ipAddr)
				{
						this->ip = ::inet_addr(ipAddr);
				}
				this->port = port;
		}
		std::string get_ip_addr() const
		{
				struct in_addr addr;
				addr.s_addr = ip;

				return std::string(inet_ntoa(addr));
		}
		u_long			ip;
		uint16_t	   port;			
};

enum EVENT
{
	EVENT_NONE	= 0,
	EVENT_READ	= 1,
	EVENT_WRITE = 2 ,
	EVENT_EXCPTION = 4,
	EVENT_TIMTOUT = 8,
	EVENT_ALL	=  ~0
};

}
#endif //nsox_socket_common_h


