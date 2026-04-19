#ifndef __nsox_tcpsocket_h
#define __nsox_tcpsocket_h
#include "socket.h"

nsox_namespace_begin
/*
 *	Abstract tcp socket, you should use upper socket class
 */

class tcpsocket : public socket
{
public:
		tcpsocket();    
		virtual~ tcpsocket();
		virtual	 void create();
       
	   /************************************************************************/
	   /* write data to kernel, if socket in block mode, this operation may be 
	      block 															   */	
	   /************************************************************************/

		virtual nu_size_t   write(const char* data, nu_size_t len);

		/************************************************************************/
		/* read data from kernel, if socket in block mode, this operation may be 
		   block 																*/
		/************************************************************************/

		virtual nu_size_t   read( char* buffer,  nu_size_t len); 

		/************************************************************************/
		/* get the local address of the connection                              */
		/************************************************************************/
		
		address			get_local_address();

		/************************************************************************/
		/* get the remote address of the connection                             */
		/************************************************************************/

		address			get_peer_address();  

		/************************************************************************/
		/* tell wheather the socket  is connected                               */
		/************************************************************************/
		        
		bool			is_connected();
		void			set_connected(bool on);
		
		/************************************************************************/
		/* shutdown the read side of the socket                                 */
		/************************************************************************/

		void			close_read();
		bool			is_read_closed();

		/************************************************************************/
		/* shutdown the write side of the socket                                 */
		/************************************************************************/

		void			close_write();
		bool			is_write_closed();

		/************************************************************************/
		/* set the timeout of the socket, if milliseconds equals 0, 
		   will cacel the timeout mode                                          */
		/************************************************************************/

		void			set_recv_timeout(nu_time_t milliseconds);
		void			set_send_timeout(nu_time_t milliseconds);

		/************************************************************************/
		/* Returns the milliseconds of timeout                                  */
		/************************************************************************/
		nu_time_t			get_recv_timeout();
		nu_time_t			get_send_timeout();

		/************************************************************************/
		/* disable/enable Nagle's algorithm                                     */
		/************************************************************************/

		void			set_tcp_nodelay(boolean on);

		/************************************************************************/
		/* Returns wheather the tcp no delay flag is set on						*/
		/************************************************************************/

		boolean			is_tcp_nodelay();
		
		/************************************************************************/
		/* Enable/disable SO_LINGER with the specified linger time in seconds   */
		/************************************************************************/

		void			set_solinger(nu_time_t linger);
		
		/************************************************************************/
		/* Returns setting for SO_LINGER. -1 returns implies that the option 
		  is disabled. The setting only affects socket close                    */
		/************************************************************************/
		nu_time_t			get_solinger();			

		// get/set the send buffer size
		void			set_sendbuff_size(nu_size_t size);
		nu_size_t			get_sendbuff_size();

		// get/set the recv buffer size
		void			set_recvbuff_size(nu_size_t size);
		nu_size_t			get_recvbuff_size();
private:
		bool __is_connected;
		bool __is_read_closed;
		bool __is_write_closed;
}; 
nsox_namespace_end

#include "tcpsocket.hpp"

#endif


