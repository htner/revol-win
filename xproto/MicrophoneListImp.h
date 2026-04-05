#pragma once
#include "common/core/ibase.h"
#include "proto/isession.h"
#include "protocol/psession.h"
#include "common/nsox/default_proto_handler.h"
#include "common/nsox/nu_request.h"
#include "common/nsox/proto_cast.h"
#include "memmisc.h"

namespace protocol{
	namespace session{
		class SessionImp;
		class MicrophoneListImp :
			public protocol::session::IMicrophoneList,
			public XConnPoint<xproto::IListEvent >,
			public XConnPoint<xproto::IActionEvent>,
			public XConnPoint<IMicrophoneListEvent>,
			public nproto::default_proto_handler
		{
			OBJECT_LEAK_CHECK(MicrophoneListImp);
			SessionImp *pSession;
			ChannelQueueMar channelqueue;
			std::set<uint32_t> guests;
			bool	hasGuests;

		public:
			MicrophoneListImp();
			~MicrophoneListImp(void);

			void setSession(SessionImp* s);
			
			DECLARE_REQUEST_MAP

			virtual void onclose(int err, nproto::connection* c){};
			virtual void oncreate(nproto::connection* c){};


			virtual bool isFolder(const HULI &hid) const;
			virtual void getParent(const HULI &hid, std::vector<HULI>& parents) const;
			virtual LPCWSTR getNick(const HULI &hid) const;
			virtual HULI  getAliaseId(const HULI &hid) const;
			virtual PlainTree_t getTreeData() const;
			virtual void fillTreeData(IListFiller*,int) const;


			virtual bool isExisit(const HULI& hid, const HULI&  pid) const {
				return false;
			}	
			virtual bool hasQueue(const UID_T& uid);
			virtual bool CanJoin();
			virtual bool CanLeave();
			virtual bool CanMove(const UID_T& target,bool down);
			virtual bool CanSay(const UID_T& target);
			virtual bool CanDoubleTime(const UID_T& target);
			virtual bool CanTuoRenQueue(const UID_T& target);
			virtual bool CanTopQueue(const UID_T& target);
			virtual void OnChangeStyle(const UID_T& sid, ChannelStyle style, uint32_t microtime);

			virtual bool getDisable(void);
			virtual bool getMute(void);
			virtual void joinQueue(void);
			virtual void leaveQueue();
			virtual void kickOff(const UID_T& uid);
			virtual void doubleTime();
			virtual void mute(bool mute);
			virtual void move(const UID_T& uid, bool down);
			virtual uinfo::SEX getSex(const UID_T& uid);
			virtual ChannelRoler getRoler(const UID_T& hid, const SID_T& channelId);
			virtual void disableQueue(void);
			virtual void allowQueue(void);
			virtual int getTime(void);
			virtual void tuorenQueue(const UID_T& uid);

			void onDisableQueue(XCast<PDisableQueue>& cmd, nproto::connection* c);
			void onJoinQueue(XCast<PJoinQueue>& cmd, nproto::connection* c);
			void onMulJoinQueue(XCast<PJoinQueueRes>& cmd, nproto::connection* c);
			void onLeaveQueue(XCast<PLeaveQueue>& cmd, nproto::connection* c);
			void onKickOffQueue(XCast<PKickOffQueue>& cmd, nproto::connection* c);

			void onDoubleTimeQueue(XCast<PDoubleTimeQueue>& cmd, nproto::connection* c);
			void onMuteQueue(XCast<PMuteQueue>& cmd, nproto::connection* c);
			void onMoveQueue(XCast<PMoveQueue>& cmd, nproto::connection* c);
			void onTurn(XCast<PTurnQueue>& cmd, nproto::connection* c);

			void onTimeout(XCast<PTimeOutQueue>& cmd, nproto::connection* c);
			void onTuoRenQueue(XCast<PTuoRenQueue>& cmd, nproto::connection* c);
			void onKickAllQueue(XCast<PKickAllQueue>& cmd, nproto::connection* c);
			void onTopQueue(XCast<PTopQueue>& cmd, nproto::connection* c);
			void OnSync(XCast<PMaixuJoin>& cmd, nproto::connection* c);


			void onAddGuest(XCast<PQueueAddGuest>& cmd, nproto::connection* c);
			void onRemoveGuest(XCast<PQueueRemoveGuest>& cmd, nproto::connection* c);
			void onClearGuest(XCast<PQueueClearGuest>& cmd, nproto::connection* c);
			

			virtual void onChange(const UID_T& id, const SID_T& p);
			

			virtual void kickAllOff();
			virtual void move2top(const UID_T& uid);

			void clearQueue();


			void _onRemoveGuest (uint32_t admin, uint32_t uid);
			void _onClearGuest (uint32_t admin = 0) ;
			void _onAddGuest(uint32_t admin, uint32_t uid);


			virtual void addGuest(const UID_T &uid); 
			virtual void removeGuest(const UID_T &uid); 
			virtual void clearGuests();
			virtual bool canAddGuest(const UID_T &uid); 
			virtual bool canRemoveGuest(const UID_T &uid); 
			virtual bool canClearGuests();

			virtual bool isGuest(const UID_T &uid);
			virtual const std::set<uint32_t>& getGuests();
		};

	}
}
