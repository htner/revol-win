#ifndef __nu_types_h__
#define __nu_types_h__
#include "int_types.h"


#ifdef NSOX_WIN32
		#include "nu_win_sock.h"
#else
		#include "nu_unix_sock.h"
#endif

namespace nsox{

#ifdef NSOX_WIN32
		typedef int socklen_t;
		#define nsox_last_error GetLastError()
#else
		#define nsox_last_error errno
#endif


#define NSOX_INVALID_SOCKET  (HSOCKET)(~0)
#define NSOX_SOCKET_ERROR            (-1)
#define VALID_SOCKET(x)	  (x != NSOX_INVALID_SOCKET)

#define NSOX_RECV_TIMEOUT SO_RCVTIMEO
#define NSOX_SEND_TIMEOUT SO_SNDTIMEO
#define INFINITY_TIME	  -1

#ifndef NULL
#define NULL 0
#endif


#define  SET_FALSE(val) (val = false)

#define memzero(a)  memset(&a, 0 , sizeof(a))
#define nsox_namespace_begin namespace nsox{
#define nsox_namespace_end	 }



typedef		uint32_t			HSOCKET;
typedef		uint32_t 			nu_size_t;
typedef		uint32_t 			u_long;
typedef		int32_t 			nu_time_t;
typedef		bool 				boolean;
typedef		int32_t				error_t;


typedef		uint32_t			timer_span;
typedef		uint32_t			timer_id;



}
#endif

