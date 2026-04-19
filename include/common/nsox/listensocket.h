#ifndef nsox_listen_h
#define nsox_listen_h
#include "socket.h"

namespace nsox
{
template<class TConnSocket, class TSocket = Socket>
class ListenSocket : public TSocket
{
public:
        Listen_Socket();
        TConnSocket*                accept();
        void                        bind(address addr);
        void                        close();
        /*
         * if this socket is bind to a address, return that address, else return 
         * invalid_address
         */
        address                      get_bind_address();
        /*
         * get the socket address which listen on
         */
        address                      get_local_address();
}    
}// namespace end
#endif
