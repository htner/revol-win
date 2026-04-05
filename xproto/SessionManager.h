#pragma once
#include "proto/ismanager.h"
#include "proto/isession.h"
#include "common/core/ibase.h"
#include "common/core/ilink.h"

#include "protocol/psmanager.h"
#include "protocol/plogin.h"
#include "protocol/plink.h"
//#include "protocol/psstream.h"
#include "common/nsox/default_proto_handler.h"
#include "common/nsox/nu_request.h"
#include "common/nsox/nuautoptr.h"
#include "common/nsox/proto_cast.h"
#include "proto/proto_obj_name.h"
#include "timer/TimerWrap.h"

#include "sitem.h"

#define MAX_OPEN_SESSION 10


namespace xproto{
	class CProtoLink;
}
namespace protocol{
	namespace login{
		class LoginImp;
	}
	namespace slist{
		class SessionList;
	}
	namespace session{
		class SessionImp;
		struct PFindChannel;
		class SessionConnCreator;

		class SessionManager : public protocol::session::ISessionManager
							 , public XConnPoint<ISessMgrEvent>
							 , public XConnPoint<IFindEvent>
							 , public XConnPoint<xproto::IActionEvent>
							 , public xproto::IUserEvent
							 , public nproto::default_proto_handler
		{
			friend class SessionImp;
			enum SESSLBS_STATUS{
				e_init = 1,
				e_lbs,
				e_relbs,
				e_succ
			};

			nsox::nu_auto_ptr<SessionImp> __session;
		private:
			UID_T uid;
			std::string nick;
			
			std::string passwd;
			uinfo::SEX gender;
			std::string sign;

			nsox::nu_auto_ptr<xproto::IUserInfo> pUInfo;

			nsox::nu_auto_ptr<slist::SessionList> pSList;

			uint16_t findCtxId;

			bool initialCloseSession;

			int newsIndex;


			typedef std::pair<xstring, xstring> news_pair_t;
			typedef std::vector<news_pair_t> news_vec_t;

			news_vec_t		  news;
			SessionCreateInfo  __createInfo;

			xproto::CProtoLink*			__protoLink;

			TimerHandler<SessionManager> sessionLogTimer;
			TimerHandler<SessionManager> UserInfoChangeTimer;

			SESSLBS_STATUS _sl_status;
			int				_relbstime;

		public:
			SessionManager();
			virtual ~SessionManager(void);

			void setProtoLink(xproto::CProtoLink* protoLink);

			DECLARE_REQUEST_MAP
			DECLARE_PROTO_OBJECT_NAME(SESSION_MANAGER_OBJ)

			virtual void onclose(int , nproto::connection*){};
			virtual void oncreate(nproto::connection*){};

			virtual void onRefresh();
			virtual void onJoinSessRefresh(uint32_t pid){};
			virtual void onEffectRefresh(){};


			void	setSessionList(slist::SessionList* pl);

			void	updateSlist(uint32_t sid, const SItem &s);

			void	updateMyList(const SID_T& sid, bool badd);

			virtual ISession *join(const HULI &sid, const xstring& passwd, ISessionHandler* h);

			virtual ISession *GetJoined();

			virtual void changeSign(xstring& sign);

			void onJoin(XCast<protocol::link::PSessionLbsRes>& cmd ,nproto::connection* c);
			
			void setUInfo(xproto::IUserInfo* info);
			void setOwner(const UID_T& u, const std::string &p){uid = u; passwd = p;};
			//void NotifyLoginRescode(SessionImp *s, int sig);
			void setInitialClose(bool b);
			bool reconnect(bool b = false);
			bool leaveGuild(uint32_t sid, ILeaveWatcher *);
			void initialEraseSession(ISession *s);
			void shutdown();

			void eraseSession();
			void onSessionTimeOut();
			void UInfoChangeTimeout();
			virtual void onOpenPlatformRefresh() {}

		//	void OnGetStreamAddr(XCast<protocol::sstream::GetStreamAddrResPacket>& cmd, nproto::connection* c);


			template <typename T>
			void sendRequest(T& req)
			{
				if(__protoLink){
					__protoLink->writeRequest(&req, T::uri);
				}				
			}
		};

	}
}
