#ifndef nsox_listen_h
#define nsox_listen_h

#include <stdexcept>
#include "socket.h"
#include "socketutil.h"
#include "listen.h"
#include "sockcomm.h"
#include "nulog.h"
#include "nu_except.h"

nsox_namespace_begin

class serversocket : public socket
{
public:
		serversocket();
		virtual~ serversocket();
		virtual void create();

		/************************************************************************/
		/* Accept a connect from client                                         */
		/************************************************************************/
		socket_result<HSOCKET>				accept(address& addr);

		/************************************************************************/
		/*  Bind a local address and port                                       */
		/************************************************************************/

		void								listen(const address& address);

       /************************************************************************/
		/* If this socket is bind to a address, return that address, else return 
		   invalid_address                                                      */
       /************************************************************************/
		address								get_bind_addr();

		/************************************************************************/
		/* Is address reuse mode on                                             */
		/************************************************************************/        

		bool								is_reuse_addr();

		/************************************************************************/
		/* Enable/Disable the reuse address mode                                */
		/************************************************************************/

		void								set_reuse_addr(boolean on);		

};

/************************************************************************/
/* Implemen                                                             */
/************************************************************************/

nsox_namespace_end

#include "listen.hpp"

#endif

