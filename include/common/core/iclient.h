#ifndef CORE_ICLIENT_H_
#define CORE_ICLIENT_H_
#include "request.h"
#include <vector>
#include "base_svid.h"
#include "common/core/ilink.h"

namespace core {

struct IClientWatcher {
	virtual ~IClientWatcher() {
	}
	virtual bool onServerAdd(uint32_t serverId, protocol::svid vid, 
		const std::string &dip, const std::string &wip, uint32_t isp, std::vector<uint16_t> &port) = 0;
	virtual void onServerRemoved(uint32_t serverId, bool) = 0;
};

struct IClient: public ILinkHandlerAware{
	virtual ~IClient() {
	}
	
	virtual ILinkHandler *getLinkHandler() = 0;

	virtual void start() = 0;
};

struct IDaemonClient: public IClient{
	virtual void watch(IClientWatcher *) = 0;

	virtual IDaemonClient &addFavour(protocol::svid s) = 0;
};

}
#endif /*ICLIENT_H_*/
