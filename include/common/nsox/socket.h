#ifndef __nsox_socket_h        //prevent multi include
#define __nsox_socket_h

#include "sockcomm.h"
#include "nu_except.h"
#include "socketutil.h"
#include "nulog.h"


nsox_namespace_begin

class socket
{
public:
		socket();
		virtual ~socket();
		socket(HSOCKET hSocket);
		/*
		*   is socket in block mode
		*/
		virtual bool			is_block_mode();     
		/*
		*   set the socket block mode, true is block, false is unblock
		*/                           
		virtual void			set_block_mode(bool bBock);
		/*
		*   get the descriptor of the socket
		*/        
		inline	HSOCKET	get_socket_handler(){return __socket_fd;}            
		/*
		*   set the descriptor of the socket
		*/     
		virtual void			set_socket_handler(HSOCKET hSocket);             

		virtual void			onclose(error_t err);
		virtual void			close();
		virtual void			create();

		virtual bool			is_closed();
private:
		HSOCKET __socket_fd;
		bool	__is_blocking;
		bool	__is_closed;
};

nsox_namespace_end

#include "socket.hpp"

#endif //nsox_socket_h
