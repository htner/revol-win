#pragma once
#include "proto/iproto.h"
#include "proto/isession.h"
#include "common/nsox/default_proto_handler.h"
#include "common/nsox/nu_request.h"
#include "common/nsox/proto_cast.h"
#include "protocol/psession.h"
#include "protocol/pplugin.h"

#include "memMisc.h"
namespace protocol{
	namespace session{

	class SessionImp;
	class CPlugInManager;

	class SessionPlugInImp :
		public protocol::session::ISessionPlugBase,
		public XConnPoint<protocol::session::ISessionPlugEvent>
	{
	public:
		SessionPlugInImp(void);
		virtual ~SessionPlugInImp(void);
		void onFeatureResPonse(XCast<protocol::plugin::PFeatureResponse>& cmd, nproto::connection* c);
		void onFeatureBroadCast(XCast<protocol::plugin::PFeatureBroadCast>& cmd, nproto::connection* c);
		void send(const std::string&);
		void setManager(CPlugInManager*);
		void setType(uint32_t type);
		void onPlugChange(uint32_t type,  bool);

		uint32_t __type;
	private:
		CPlugInManager *__pluginMgr;
		
	};

	class CPlugInManager : public ISessionPluginManager
						   , public nproto::default_proto_handler
	{
		SessionImp *pSession;
		friend class SessionPlugInImp;
	public:
		void setSession(SessionImp* s);
		
	public:
		CPlugInManager();
		virtual ~CPlugInManager(void);

		BEGIN_REQUEST_MAP(CPlugInManager)
			REQUEST_HANDLER(XCast<protocol::session::PPlugToClient>, onPlugData)
			//REQUEST_HANDLER(XCast<protocol::plugin::PFeatureResponse>, onFeatureResPonse)
		END_REQUEST_MAP()
		void onclose(int err, nproto::connection* c){};		
		void oncreate(nproto::connection* c){};

		virtual protocol::session::ISessionPlugBase* getPlugIn(uint32_t type);
		virtual bool isPlugInOpen(uint32_t type) const;		
		void send(const char* data, uint32_t size,  uint32_t type);
		void openChange(const xstring&);

	protected:
		void writeRequest(nproto::request* req, int uri);
	protected:
		void onFeatureResPonse(XCast<protocol::plugin::PFeatureResponse>& cmd, nproto::connection* c);
		void onFeatureBroadCast(XCast<protocol::plugin::PFeatureBroadCast>& cmd, nproto::connection* c);
		void onPlugData(XCast<protocol::session::PPlugToClient>& cmd, nproto::connection* c);
	protected:
		typedef nsox::nu_auto_ptr<SessionPlugInImp> SessionPlugInImpPtr;
		typedef std::map<uint32_t , SessionPlugInImpPtr,std::less<uint32_t>, PairAllocator(uint32_t, SessionPlugInImpPtr) > plugin_map;
		typedef std::set<uint32_t, std::less<uint32_t>, TempAllocator(uint32_t) > plugin_set;
		plugin_map __plugins;
		plugin_set __opens;

	};



}
}
