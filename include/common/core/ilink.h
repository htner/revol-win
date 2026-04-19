#ifndef ILINK_PROTOCOL_H
#define ILINK_PROTOCOL_H

#include "common/socketinc.h"
#include "common/blockbuffer.h"
#include "common/int_types.h"
#include "common/sorted_vector.h"
#include "base_svid.h"
#include <string>
#include <set>
#include <map>
#include <stdexcept>
#include <vector>
#include <utility>
#define TCP_ENCODE 0
#define TCP_UNCODE 1
#define UDP_ENCODE 2
#define UDP_UNCODE 3
#define NONEEXP_CID 0xffffffff

namespace core {
class Request;
struct IContext;
struct IConn;
struct ILinkHandler {
	//callback funcs
	//virtual void	onEvent(int ev, const char* msg, class IConn* from) = 0;
	virtual ~ILinkHandler() {
	}
	virtual int onData(const char*, size_t, IConn *conn, int type = 0) = 0;
};

struct ILinkHandlerAware{
	ILinkHandler *handler;
	ILinkHandlerAware():handler(NULL){}
	virtual ~ILinkHandlerAware(){}
	virtual void setLinkHandler(ILinkHandler *h){
		handler = h;
	}
};


struct ILinkEvent {
	virtual ~ILinkEvent() {
	}
	virtual void onConnected(IConn *conn) = 0;
	virtual void onClose(IConn *conn) = 0;
	//主动关闭
	virtual void onInitiativeClose(IConn *conn) = 0;
	virtual void onError(int ev, const char *msg, IConn *conn) = 0;
	//virtual void onMultiError(int ev, const char *msg, std::vector<IConn *> &conn) = 0;
	virtual void onTimeout(IConn *conn) = 0;
};

struct IConn {
	// flush : 关闭之前, 刷空缓存
	//virtual void release(bool flush) throw () = 0;
	//virtual std::string getuid() const = 0;
	IConn();
	
	virtual ~IConn() {
	}

	virtual void send(Request &resp) = 0;

	virtual void sendBin(const char *data, size_t sz, uint32_t uri) = 0;

	virtual IContext *findAndOwnCtx(uint16_t sid) = 0;

	virtual void setTimeout(int tm) = 0;

	virtual void *getData() = 0;
	//virtual void init_() = 0;
	//virtual void connect_(const std::string& ip, unsigned port) = 0;
	//virtual void close_() = 0;
	virtual void setEncKey(const unsigned char *key, size_t len) = 0;

	void setHandler(ILinkHandler* hdl) ;
	void setLinkEvent(ILinkEvent *ehdl);

	ILinkEvent *getLinkEvent();
	//ILinkHandler *getHandler() ;
	/*void bindUid(const std::string &id);
	std::string getUid() const ;*/
	uint32_t getPeerIp() const ;
	int getPeerPort() const ;
	uint32_t getConnId() const ;
	void setConnId(uint32_t i);

	void setSerialId(uint32_t s);
	uint32_t getSerialId() const;

	void setEnable(bool be = false){
		bEnable = be;
	}

	virtual bool available() const;

	virtual bool isEncrypto() const;

	
	virtual void reconnect() = 0;

	virtual int		getLocalIp();
	virtual void	close(){
		bClose = true;
	}
	virtual	bool	closed(){
		return bClose;
	}
protected:
	ILinkEvent *eHandler;
	ILinkHandler *dHandler;
	//std::string uid;
	uint32_t id;
	uint32_t peerIp;
	uint32_t serialId;
	int peerPort;
	bool bEnable;
	bool bClose;
};



struct IConnDispatcher {
	virtual ~IConnDispatcher() {
	}
	virtual bool dispatchById(uint32_t cid, Request &request) = 0;
	virtual bool dispatchByIds(const std::set<uint32_t> &ids, Request &request, uint32_t exp = NONEEXP_CID) = 0;
	virtual bool dispatchByIds(const std::sorted_vector<uint32_t> &ids, Request &request, uint32_t exp = NONEEXP_CID) = 0;
};

struct CreateCallback{
	virtual ~CreateCallback(){}
	virtual void onConnCreate(IConn *conn) = 0;
};
struct IClientConnCreator{
public:
	virtual ~IClientConnCreator() {}
	virtual IConn *createConnect(const std::string& ip, uint32_t port, ILinkHandler *ih, ILinkEvent *ie, CreateCallback *) = 0;
};

struct IServerConnCreator{
public:
	virtual ~IServerConnCreator(){}
	virtual IConn *creatConnect(SOCKET, uint32_t ip, int port, ILinkHandler *ih, ILinkEvent *ie, CreateCallback *) = 0;
};

struct IConnManager: public IConnDispatcher, public ILinkEvent, public CreateCallback{
protected:
	IClientConnCreator *clientCreator;
	IServerConnCreator *serverCreator;
public:
	IConnManager():clientCreator(NULL), serverCreator(NULL){}
	~IConnManager();

	virtual void eraseConnect(IConn *conn) = 0;
	virtual void eraseConnectById(uint32_t id) = 0;

	virtual IConn *getConnectById(uint32_t id) = 0;

	virtual void setClientConnCreator(IClientConnCreator *cc){
		clientCreator = cc;
	};

	virtual void setServerConnCreator(IServerConnCreator *sc){
		serverCreator = sc;
	}

	virtual IConn *createServerConn(SOCKET, uint32_t ip, int port, ILinkHandler *iH, ILinkEvent *iE) = 0;

	virtual IConn *createClientConn(const std::string& ip, uint32_t port, ILinkHandler *iH, ILinkEvent *iE) = 0;
};

struct ISvidDispatcher{
	virtual ~ISvidDispatcher(){}
	
	virtual void removeSvidConn(IConn *c) = 0;

	virtual bool dispatchBySvid(Request &request) = 0;

	virtual bool forceDispatchBySvid(protocol::svid, Request &request) = 0;

	virtual void broadcastBySvid(uint32_t svid, Request &request) = 0;

	virtual void addSvidConn(protocol::svid id, protocol::HashType type, IConn *conn) = 0;

//	virtual std::vector<IConn *> getSvidConn(protocol::svid id) = 0;
};

struct IServerIdDispatcher{
	virtual ~IServerIdDispatcher(){}
	virtual void dispatchByServerId(uint32_t serverId, core::Request &) = 0;
};

struct IServerIdDispatcherAware{
protected:
	IServerIdDispatcher *sDispatcher;
public:
	virtual void setServerIdDispatcher(IServerIdDispatcher *d){
		sDispatcher = d;
	}
};

struct IConnManagerAware {
protected:
	IConnManager *connManager;
public:
	IConnManagerAware() :
		connManager(NULL) {
	}
	virtual ~IConnManagerAware() {
	}
	virtual void setConnManager(IConnManager *c) {
		connManager = c;
	}

	IConnManager *getConnManager(){
		return connManager;
	}
};

struct IConnDispatcherAware{
protected:
	IConnDispatcher *dispatcher;
public:
	IConnDispatcherAware() : dispatcher(NULL) {
	  }
	  virtual ~IConnDispatcherAware() {
	  }
	  virtual void setConnDispatcher(IConnDispatcher *c) {
		  dispatcher = c;
	  }
};

struct ISvidDispatcherAware{
protected:
	ISvidDispatcher *svidDispatcher;
public:
	ISvidDispatcherAware() : svidDispatcher(NULL) {
	}
	virtual ~ISvidDispatcherAware() {
	}
	virtual void setSvidDispatcher(ISvidDispatcher *c) {
		svidDispatcher = c;
	}
};


}
#endif
