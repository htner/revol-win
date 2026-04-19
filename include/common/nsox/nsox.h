#ifndef __nsox_h
#define __nsox_h


nsox_namespace_begin

struct asyn_server_handler
{
	virtual void onsocket(HSOCKET hsocket, const address& addr) = 0;
};

struct nsox_app_handler 
{
	 virtual void start() = 0;
	 virtual void stop()  = 0;
};

nsox_namespace_end
#endif


