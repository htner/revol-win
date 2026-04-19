#ifndef IProtocolManager_Protocol_H
#define IProtocolManager_Protocol_H
#include "common/unistr.h"
namespace core{
	struct IClientConnFactory;
	struct ILinkHandler;
}
namespace protocol {
	namespace login{
		struct ILogin;
		struct ICheckVersion;
	}
	namespace conf{
		struct IConfigManager;
	}
	namespace uinfo{
		struct IUInfo;
		//struct IUInfoFactory;
	}
	namespace session
	{
		struct ISessionManager;
		struct IGuildInfoManager;
	};
	namespace slist{
		struct ISessionList;
	}
	namespace report{
		struct IReport;
	}

	namespace catalog{
		struct ICatalogList;
	}
	struct IProtocolManager {
		virtual login::ICheckVersion *getCheckVersion() = 0;
		virtual login::ILogin* getLogin() = 0;
		virtual conf::IConfigManager* getConfigManager() = 0;
		virtual uinfo::IUInfo* getUInfo(const UniString &uid) = 0;
		virtual UniString whoAreU() = 0;
		virtual slist::ISessionList* getSessionList(void) = 0;
		virtual session::ISessionManager* getSessionManager(void) = 0;
		virtual report::IReport* getReport() = 0;
		virtual session::IGuildInfoManager *getGuildManager() = 0;
		virtual catalog::ICatalogList *getCatalogList() = 0;
		//virtual uinfo::IUInfoFactory *getUInfoFactory(const UniString &uid) = 0;
		virtual void init(core::IClientConnFactory *f = NULL) = 0;
		virtual void releaseProtocol() = 0;
		//virtual void release() = 0;
	};
}

#endif
