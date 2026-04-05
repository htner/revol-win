#pragma once
#include "proto/isession.h"
#include "sessionimp.h"
#include "SessionMemberListImp.h"
#include "common/xstring.h"
#include "memmisc.h"

#define BEGIN_SESS_OPER_MAP(theClass) \
public : \
bool RulerChecker::canDo(SessOperation* operation)\
{\
		SessOperation::OpCode _opcode = operation->opcode;\
		__op_user_room_id = pSession->getSessionList()->innerGetUserPid(operation->user);

#define OPERTOR_HANDLER(opcode, func) \
if(_opcode == opcode) \
{\
		return   func(operation);\
}

#define END_SESS_OPER_MAP \
return true;\
}


#define OPERATOR_FUCNTION(func) \
bool func(SessOperation* operation);



						   
namespace protocol{
	namespace session{
		class client_role_source_imp;
		class sess_auth_checker_imp;

		class RulerChecker :
			public IAuthChecker
		{
			SessionImp *pSession;
			uint32_t mine;			
			BEGIN_SESS_OPER_MAP(RulerChecker)
					OPERTOR_HANDLER(SessOperation::CREATE_SUBCHANNEL_CHK,	canCreateSubChannel)	
					OPERTOR_HANDLER(SessOperation::DISSMISS_SUBCHANNEL_CHK,canRemoveSubChannel)	
					OPERTOR_HANDLER(SessOperation::ADD_MANAGER,			canAddManager)				
					OPERTOR_HANDLER(SessOperation::REMOVE_MANAGER,		canRemoveManager)	
					OPERTOR_HANDLER(SessOperation::ADD_SUB_MANAGER,		canAddSubManager)				
					OPERTOR_HANDLER(SessOperation::REMOVE_SUB_MANAGER,	canRemoveSubManager)				
					OPERTOR_HANDLER(SessOperation::ADD_MEMBER,			canAddMember)			
					OPERTOR_HANDLER(SessOperation::REMOVE_MEMBER,		canRemoveMember)				
					OPERTOR_HANDLER(SessOperation::JOIN_CHANNEL,		canJoinChannel)			
					OPERTOR_HANDLER(SessOperation::JOIN_WITHOUTPASSWD,	canJoinWithoutPasswd)		
					OPERTOR_HANDLER(SessOperation::KICKOFF,				canKickOff)				
					OPERTOR_HANDLER(SessOperation::VOICE,				canVoiceChat)					
					OPERTOR_HANDLER(SessOperation::TEXT,				canTextChat)					
					OPERTOR_HANDLER(SessOperation::CHANGESTYLE,			canChangeStyle)				
					OPERTOR_HANDLER(SessOperation::SETKEYACTIVE,		canSetKeyActive)				
					OPERTOR_HANDLER(SessOperation::DISABLE_VOICE,		canDisableVoice)			
					OPERTOR_HANDLER(SessOperation::ENABLE_VOICE,		canEnableVoice)			
					OPERTOR_HANDLER(SessOperation::ENABLE_TEXT,			canEnalbeText)				
					OPERTOR_HANDLER(SessOperation::DISABLE_TEXT,		canDisabeText)			
					OPERTOR_HANDLER(SessOperation::BROADCAST,			canBroadCast)			
					OPERTOR_HANDLER(SessOperation::DRAGETO_QUEUE,		canDragtoQueue)			
					OPERTOR_HANDLER(SessOperation::JOIN_QUEUE,			canJoinQueue)			
					OPERTOR_HANDLER(SessOperation::LEAVE_QUEUE,			canLeaveQueue)			
					OPERTOR_HANDLER(SessOperation::MOVEUP_QUEUE,		canQueueOrderUp)			
					OPERTOR_HANDLER(SessOperation::MOVEDOWN_QUEUE,		canQueueOrderDown)			
					OPERTOR_HANDLER(SessOperation::KICK_QUEUE,			canKickFromQueue)			
					OPERTOR_HANDLER(SessOperation::MUTE_QUEUE,			canMuteQueue)			
					OPERTOR_HANDLER(SessOperation::UNMUTE_QUEUE,		canUnMuteQueue)			
					OPERTOR_HANDLER(SessOperation::DOUBLETIME_QUEUE,	canDoubleTimeQueue)			
					OPERTOR_HANDLER(SessOperation::DISABLE_QUEUE,		canDisableQueue)				
					OPERTOR_HANDLER(SessOperation::ALLOW_QUEUE,			canAllowQueue)			
					OPERTOR_HANDLER(SessOperation::DRAG_PEOPLE,			canDragPeople)			
					OPERTOR_HANDLER(SessOperation::DROP_PEOPLE,			canDropPeople)			
					OPERTOR_HANDLER(SessOperation::DRAG_ROOM,			canDragRoom)			
					OPERTOR_HANDLER(SessOperation::DROP_ROOM,			canDropRoom)		
					OPERTOR_HANDLER(SessOperation::QUIT_GUILD,			canQuitGuild)		

					OPERTOR_HANDLER(SessOperation::DIRECT_KICKOFF, canDirectKickoff)

					

					OPERTOR_HANDLER(SessOperation::EDIT_CHANNEL_INFO,	canEditChannelInfo)	
					OPERTOR_HANDLER(SessOperation::SHOW_AS_MANAGER,		canShowAsManager)		
					OPERTOR_HANDLER(SessOperation::DISABLE_ALL_TEXT,	canDisableAllText)
					OPERTOR_HANDLER(SessOperation::ENABLE_ALL_TEXT,		canEnableAllText)
					OPERTOR_HANDLER(SessOperation::SETALLKEYACTIVE,		canSetAllKetActive)
					OPERTOR_HANDLER(SessOperation::ADD_VIP,				canAddVip)
					OPERTOR_HANDLER(SessOperation::REMOVE_VIP,			canRemoveVip)
					OPERTOR_HANDLER(SessOperation::ADD_TMPVIP,			canAddTempVip)
					OPERTOR_HANDLER(SessOperation::REMOVE_TMPVIP,		canRemoveTempVip)
					OPERTOR_HANDLER(SessOperation::ADD_BAN_ID,			canAddBanID)
					OPERTOR_HANDLER(SessOperation::REMOVE_BAN_ID,		canRemoveBanID)
					OPERTOR_HANDLER(SessOperation::ADD_BAN_IP,			canAddBanIp)
					OPERTOR_HANDLER(SessOperation::REMOVE_BAN_IP,		CanRemoveBanIp)

					OPERTOR_HANDLER(SessOperation::MOVETOP_QUEUE,		canMoveTopQueue)
					OPERTOR_HANDLER(SessOperation::CLEAR_QUEUE,			canClearQueue)
					OPERTOR_HANDLER(SessOperation::IGNORE_TEXT,			canIngoreText)
					OPERTOR_HANDLER(SessOperation::IGNORE_VOICE,		canIngoreVoice)

					OPERTOR_HANDLER(SessOperation::EDIT_SESSION_INFO,	canChangeOrder)
					OPERTOR_HANDLER(SessOperation::CHANGE_SESS_ORDER,	canChangeOrder)
					OPERTOR_HANDLER(SessOperation::MEMBER_TEXT,			canMemberText)
					OPERTOR_HANDLER(SessOperation::UPDATE_MEMBER_CARD,	canUpdateMemberCard)

					OPERTOR_HANDLER(SessOperation::ADD_QUEUE_GUEST,	canAddQueueGuest)
					OPERTOR_HANDLER(SessOperation::REMVOE_QUEUE_GUEST, canRemoveQueueGuest)
					OPERTOR_HANDLER(SessOperation::CLEAR_QUEUE_GUEST,	canClearQueueGuest)
			END_SESS_OPER_MAP
		public:
				OPERATOR_FUCNTION(canCreateSubChannel)
				OPERATOR_FUCNTION(canRemoveSubChannel)
				OPERATOR_FUCNTION(canAddManager)
				OPERATOR_FUCNTION(canRemoveManager)	
				OPERATOR_FUCNTION(canAddSubManager)
				OPERATOR_FUCNTION(canRemoveSubManager)
				OPERATOR_FUCNTION(canAddMember)			
				OPERATOR_FUCNTION(canRemoveMember)			
				OPERATOR_FUCNTION(canJoinChannel)			
				OPERATOR_FUCNTION(canJoinWithoutPasswd)		
				OPERATOR_FUCNTION(canKickOff)				
				OPERATOR_FUCNTION(canVoiceChat)					
				OPERATOR_FUCNTION(canTextChat)					
				OPERATOR_FUCNTION(canChangeStyle)				
				OPERATOR_FUCNTION(canSetKeyActive)			
				OPERATOR_FUCNTION(canDisableVoice)			
				OPERATOR_FUCNTION(canEnableVoice)			
				OPERATOR_FUCNTION(canEnalbeText)				
				OPERATOR_FUCNTION(canDisabeText)			
				OPERATOR_FUCNTION(canBroadCast)			
				OPERATOR_FUCNTION(canDragtoQueue)			
				OPERATOR_FUCNTION(canJoinQueue)			
				OPERATOR_FUCNTION(canLeaveQueue)			
				OPERATOR_FUCNTION(canQueueOrderDown)		
				OPERATOR_FUCNTION(canQueueOrderUp)		
				OPERATOR_FUCNTION(canKickFromQueue)			
				OPERATOR_FUCNTION(canMuteQueue)			
				OPERATOR_FUCNTION(canUnMuteQueue)			
				OPERATOR_FUCNTION(canDoubleTimeQueue)			
				OPERATOR_FUCNTION(canDisableQueue)			
				OPERATOR_FUCNTION(canAllowQueue)			
				OPERATOR_FUCNTION(canDragPeople)			
				OPERATOR_FUCNTION(canDropPeople)
				OPERATOR_FUCNTION(canDragRoom)			
				OPERATOR_FUCNTION(canDropRoom)	
				OPERATOR_FUCNTION(canDirectKickoff)	

				

				OPERATOR_FUCNTION(canEditChannelInfo)	
				OPERATOR_FUCNTION(canShowAsManager)		
				OPERATOR_FUCNTION(canDisableAllText)
				OPERATOR_FUCNTION(canEnableAllText)
				OPERATOR_FUCNTION(canSetAllKetActive)
				OPERATOR_FUCNTION(canAddVip)
				OPERATOR_FUCNTION(canRemoveVip)
				OPERATOR_FUCNTION(canAddTempVip)
				OPERATOR_FUCNTION(canRemoveTempVip)
				OPERATOR_FUCNTION(canAddBanID)
				OPERATOR_FUCNTION(canRemoveBanID)
				OPERATOR_FUCNTION(canAddBanIp)
				OPERATOR_FUCNTION(CanRemoveBanIp)
				OPERATOR_FUCNTION(canQuitGuild)


				OPERATOR_FUCNTION(canMoveTopQueue)
				OPERATOR_FUCNTION(canClearQueue)
				OPERATOR_FUCNTION(canIngoreText)
				OPERATOR_FUCNTION(canIngoreVoice)

				OPERATOR_FUCNTION(canChangeOrder)

				OPERATOR_FUCNTION(canMemberText)
				OPERATOR_FUCNTION(canUpdateMemberCard)

				OPERATOR_FUCNTION(canAddQueueGuest)
				OPERATOR_FUCNTION(canRemoveQueueGuest)
				OPERATOR_FUCNTION(canClearQueueGuest)
		protected:
				bool recurCheckAuth(session_auth::auth_code code, const UID_T& fuid, const UID_T& tuid, const SID_T& channel_id);
				bool recurCheckAuth(session_auth::auth_code code, const SID_T& channel_id);
				bool recurCheckAuth(session_auth::auth_code code, const SID_T& tuid, const SID_T& channel_id);
		public:
			DEFINE_NEW_DELETE_OPERATOR();
			RulerChecker(SessionImp *ss, const UID_T &me);
			~RulerChecker(void);
			void	push_auth(AuthCode& auth);


			SID_T getParent(SID_T sid);

		private:
			bool isMeGreateThanManager();
			ChannelRoler getRoller(const HULI &room);
			ChannelRoler getRoller(const UID_T &uid, const HULI& room);
	   
		protected:
			client_role_source_imp*		__rs;
			SID_T						__op_user_room_id; // 被操作用户所在的频道
			sess_auth_checker_imp*		__auth_checker;
		};
	}
}

