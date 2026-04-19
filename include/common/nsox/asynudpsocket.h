#ifndef __nsox_asyn_udpsocket_h
#define __nsox_asyn_udpsocket_h
#include "asynsocket.h"
#include <assert.h>

nsox_namespace_begin

#define UDP_BUFFER_LEN 2048

class asyn_udp_socket : public asyn_socket<udpsocket>
{
public:
	virtual void onread(){			
			address udpaddr;
			int len = udpsocket::readstream(m_udp_buffer, sizeof(m_udp_buffer),udpaddr);
			if(len){
					nsox::xlog(NsoxDebug, "receivce data via udp from %s:%d", udpaddr.get_ip_addr().c_str(), udpaddr.port);
					ondata(m_udp_buffer, len, udpaddr);
			}
	};
	virtual void onwrite(){
				nsox::xlog(NsoxDebug, "write event ignored");
	};
	virtual void onexception(){
				nsox::xlog(NsoxDebug, "exception event ignored");
	};
	virtual void ontimeout(){
				nsox::xlog(NsoxDebug, "timeout event ignored");
	}
	void bind(const address& addr)
	{
				asyn_socket<udpsocket>::bind(addr);		
	}

public:
	virtual void ondata(const char* p, int len, address& addr){};

protected:
	char m_udp_buffer[UDP_BUFFER_LEN];
};

nsox_namespace_end
#endif

