#ifndef ISERVER_H_
#define ISERVER_H_
#include "common/socketinc.h"
#include "base_svid.h"
#include "common/int_types.h"
#include "common/core/ilink.h"
#include <string>
#include <vector>

namespace core {
struct IServerDispatcher;
struct IServerConnFactory;
struct IAppContextEx;
struct ILinkHandler;
struct IClientDaemon;


struct IAppContextAware {
protected:
	IAppContextEx *appContext;
public:
	IAppContextAware(): appContext(NULL){}
	virtual ~IAppContextAware() {}
	virtual void setAppContext(IAppContextEx *c){
		appContext = c;
	}
};

struct IServer: public virtual IConnManagerAware, 
				public virtual IAppContextAware, 
				public ILinkHandlerAware,
				public ISvidDispatcherAware{
	
	virtual ~IServer() {}

	virtual ILinkHandler *getLinkHandler() = 0;

	virtual void start() = 0;

	virtual void hangUp() = 0;

	virtual void wakeUp() = 0;

	virtual void setServerType(protocol::svid svid) = 0;

	virtual uint32_t getServerId() = 0;
};

struct IServerAware{
	IServer *server;
public:
	virtual ~IServerAware(){}
	virtual void setServer(IServer *s){
		server = s;
	}
};
struct IServerWatcher{
public:
	virtual ~IServerWatcher(){}
	virtual void onServerRegistered(uint32_t serverId, const std::string &dip, const std::string &wip, std::vector<uint16_t> &ports) = 0;
	virtual void onServerType(protocol::svid &svid) {}
};


struct IServerCreator{
	virtual ~IServerCreator() {}
	virtual IServer *createServer(const char *ip, int port) = 0 ;
};

struct IServerFactory: public virtual IConnManagerAware, 
					   public ILinkHandlerAware,
					   public ISvidDispatcherAware, 
					   public IAppContextAware
{
	virtual ~IServerFactory() {}
	
	virtual void setServerType(protocol::svid svid) = 0;

	virtual void start() = 0;

	virtual IServer * createServer(const char *ip, int port, uint32_t serverId) = 0;
	
	virtual uint32_t getServerId() const = 0;

	virtual IServer* getServerById(uint32_t sid) = 0;
	//xxx todo true virtual 
	virtual void setServerCreator(IServerCreator *creator) = 0;
};

struct IDaemonServerWatch{
	virtual ~IDaemonServerWatch(){}
	virtual void watch(IServerWatcher *w) = 0;
	virtual void revoke(IServerWatcher *w) = 0;
};

}
#endif /*ISERVER_H_*/
