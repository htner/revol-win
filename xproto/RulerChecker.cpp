#include "rulerchecker.h"
#include "SessionImp.h"
#include "helper/utf8to.h"
#include "MicrophoneListImp.h"
#include "SessRoleSource.h"
#include "client/proto/iproto.h"

#define ROLE_CANNOT_CHANGE(from, to)\
		if(from >= to) return false;

#define OPERATOR_CANNOT_SELF(user)\
		if(mine == user) return false;

#define OPERATOR_CAN_SELF(user)\
		if(mine == user) return true;


#define ROLE_CAN_NOT_CHANGE(from, to) \
if(fromRole == from && toRole==to) return false;$


using namespace protocol::session;
using namespace protocol;
using namespace xproto;

RulerChecker::RulerChecker(SessionImp *ss, const UID_T &me): pSession(ss)
{
		mine = me;

		__rs = new client_role_source_imp(ss);
		__auth_checker = new sess_auth_checker_imp();
}

RulerChecker::~RulerChecker(void)
{
		SAFE_DELETE(__rs);
		delete __auth_checker;
}

void	RulerChecker::push_auth(AuthCode& auth)
{
		__auth_checker->push((ChannelRoler)auth.sess_from_role, (ChannelRoler)auth.sess_to_role,
							 (session_auth::auth_code)auth.auth_code);
}


ChannelRoler RulerChecker::getRoller(const SID_T &room)
{
		return getRoller(mine, room);
}

ChannelRoler RulerChecker::getRoller(const UID_T &uid, const SID_T& room)
{
		return __rs->get_user_role(uid, room);
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
bool RulerChecker::recurCheckAuth(session_auth::auth_code code, const SID_T& ssid)
{
		return recurCheckAuth(code, mine, SID_NULL,  ssid);
}
bool RulerChecker::recurCheckAuth(session_auth::auth_code code, const UID_T& tuid, const SID_T& ssid)
{
		return recurCheckAuth(code, mine, tuid, ssid);
}

HULI RulerChecker::getParent(HULI sid)
{
		std::vector<ITreeNode> parents;
		pSession->getSessionList()->getParent(ITreeNode(sid), parents);	
		if(parents.empty()) return HULI_NULL;

		return parents.front().value;
}
bool RulerChecker::recurCheckAuth(session_auth::auth_code code, const UID_T& fuid,
								  const UID_T& tuid,  const SID_T& sid)
{
		ChannelRoler fRole =  __rs->get_user_role(fuid, sid);
		ChannelRoler tRole =  __rs->get_user_role(tuid, sid);

		return __auth_checker->check(fRole, tRole, code);
}
bool RulerChecker::canCreateSubChannel(SessOperation* opera)
{
		//__op_user_room_id = opera->room; // 父频道
	SID_T ppid =pSession->getSessionList()->innerGetChannelPid(opera->room);

	if(ppid != 0 || opera->room == pSession->getHall()){
		return false;
		//SID_T pppid =pSession->getSessionList()->innerGetChannelPid(ppid);
		//if(!pppid){
		//	return false;
		//}
	}
	return recurCheckAuth(session_auth::add_remove_child_room, opera->room);
}
bool RulerChecker::canRemoveSubChannel(SessOperation* opera)
{
		//__op_user_room_id = opera->room;
		if (pSession->getHall() == opera->room)
			return false;
		return  recurCheckAuth(session_auth::add_remove_child_room,  getParent(opera->room));
}

bool RulerChecker::canDirectKickoff(SessOperation* opera)
{
		OPERATOR_CANNOT_SELF(opera->user)

		return recurCheckAuth(session_auth::direct_kick_from_room,  opera->user,  opera->room);
}

bool RulerChecker::canAddSubManager(SessOperation* opera)
{
		OPERATOR_CANNOT_SELF(opera->user)

		__op_user_room_id = opera->room; //传入参数

		ChannelRoler roler = getRoller(opera->user, __op_user_room_id);
		ROLE_CANNOT_CHANGE(roler, CMANAGER)

		return recurCheckAuth(session_auth::add_remove_child_room_admin, opera->user, __op_user_room_id);
}
bool RulerChecker::canRemoveSubManager(SessOperation* opera)
{
		OPERATOR_CANNOT_SELF(opera->user)

       __op_user_room_id = opera->room;//传入参数

		ChannelRoler roler = getRoller(opera->user, __op_user_room_id);
		if (__op_user_room_id == 0 && roler == MEMBER) {
			roler = pSession->getSessionList()->getMemberRoler(opera->user);
		}

		return  (roler == CMANAGER) && 			
				recurCheckAuth(session_auth::add_remove_child_room_admin, opera->user, __op_user_room_id);
}

bool RulerChecker::canAddManager(SessOperation* opera)
{
		OPERATOR_CANNOT_SELF(opera->user)

		__op_user_room_id = opera->room;//传入参数

		ChannelRoler roler = getRoller(opera->user, 0);

		ROLE_CANNOT_CHANGE(roler, MANANGER)

		return recurCheckAuth(session_auth::add_remove_room_admin, opera->user, 0);
}
bool RulerChecker::canRemoveManager(SessOperation* opera)
{
		OPERATOR_CANNOT_SELF(opera->user)

		__op_user_room_id = opera->room;//传入参数

		ChannelRoler roler = getRoller(opera->user, 0);

		return  roler == MANANGER && 
				recurCheckAuth(session_auth::add_remove_room_admin, opera->user, 0);
}
bool RulerChecker::canAddMember(SessOperation* opera)
{
		OPERATOR_CANNOT_SELF(opera->user)

		ChannelRoler roler = getRoller(opera->user, __op_user_room_id);

		ROLE_CANNOT_CHANGE(roler, MEMBER)

		bool r =  recurCheckAuth(session_auth::add_remove_room_member, opera->user, __op_user_room_id);

		return r;
}
bool RulerChecker::canRemoveMember(SessOperation* opera)
{
		//__op_user_room_id = opera->room;

		OPERATOR_CANNOT_SELF(opera->user)

		ChannelRoler roler = getRoller(opera->user, __op_user_room_id);
		if (roler ==  NORMAL) {
			roler = pSession->getSessionList()->getMemberRoler(opera->user);
		}
		roler = ( roler == CMANAGER ||  roler == PMANAGER) ?   MEMBER : roler;

		bool r =  (roler == MEMBER) &&
			   recurCheckAuth(session_auth::add_remove_room_member, opera->user, __op_user_room_id);

		return r;
}
bool RulerChecker::canJoinChannel(SessOperation* opera)			
{
	IChannelInfo* info = pSession->getChannelInfo(opera->room);
	ISessionInfo* sessinfo = pSession->getSessionInfo();
	if (info == NULL) {
			return false;
	}
	if (sessinfo->isLimit() == true && getRoller(mine, opera->room) < TMPVIP) {
		return false;
	}
	return	(!info->isChannelDisable() || !info->isLimit() || getRoller(mine, opera->room) >= TMPVIP || 
				pSession->getSid() == opera->room);
}
bool RulerChecker::canJoinWithoutPasswd(SessOperation* opera)
{
		IChannelInfo* info = pSession->getChannelInfo(opera->room);
		if (info == NULL) {
				return false;
		}
		return recurCheckAuth(session_auth::exclude_from_passwd_room, opera->room) || 
				(!info->hasPass());
}
bool RulerChecker::canKickOff(SessOperation* opera)	
{
		return recurCheckAuth(session_auth::kick_user_from_room, opera->user, __op_user_room_id);
}
bool RulerChecker::canVoiceChat(SessOperation* opera)
{
		__op_user_room_id = pSession->getCur();

		bool user_voice = pSession->getSessionList()->isUserVoiceable(mine);
		if (user_voice == false) {
			opera->retCode = SessOperation::R_VOICE_USER_UNABLE;
		}

		bool channel_voice	= pSession->IsChannelVoiceable(__op_user_room_id);
		if (channel_voice == false) {
			opera->retCode = SessOperation::R_VOICE_CHANNEL_UNABLE;
		}

		if (!recurCheckAuth(session_auth::exclude_from_voice_ban, __op_user_room_id) && !(user_voice && channel_voice)) {
			return false;
		}

		if(pSession->getCurrentStyle() == ChairStyle){
			opera->retCode = SessOperation::R_VOICE_CHAIR_MAN_UNABLE;
				return recurCheckAuth(session_auth::exclude_from_chairman_mode, __op_user_room_id);
		}

		if(pSession->getCurrentStyle() == MicrophoneStyle){
				opera->retCode = SessOperation::R_VOICE_ORDER_UNABLE;
				if (pSession->getMicrophoneList()->getMute()) {
					opera->retCode = SessOperation::R_VOICE_CONTROL_UNABLE;
				}
				return  recurCheckAuth(session_auth::exclude_from_mic_order_mode, __op_user_room_id)/* && pSession->getMicrophoneList()->getMute())*/||
					pSession->getMicrophoneList()->CanSay(mine);				
		}

		if (pSession->getCurrentStyle() == FreeStyle) {
			IChannelInfo* info = pSession->getChannelInfo(__op_user_room_id);
			if (info == NULL) {
					opera->retCode = SessOperation::R_INNER_ERROR;
					return false;
			}

			bool normal_limit = ((pSession->GetRoler(mine, __op_user_room_id) == NORMAL ) && (info->isMicLimitFree() == true));
			if (normal_limit == true) {
				opera->retCode = SessOperation::R_VOICE_MEMBER_UNABLE;
			}
			return  !normal_limit;
		}
		return false;

		
		
}
bool RulerChecker::canTextChat(SessOperation* opera)
{
		__op_user_room_id = pSession->getCur();

		IChannelInfo* info = pSession->getChannelInfo(__op_user_room_id);
		if (info == NULL) {
					TxtFormatLimit tfl = info->getTxtLimitFormat();
					switch(tfl) {
						case tfl_nolimit:
							break;
						case tfl_urllimit:				//游客不能发送 URL
							break;
						case tfl_vislimit:				//游客不能说话
							if (pSession->GetRoler(mine, __op_user_room_id) <= MEMBER) {
										return false;
							}
							break;
						case tfl_onlymanager:
							if (pSession->GetRoler(mine, __op_user_room_id) <= CMANAGER) {
										return false;
							}
							break;
						default:
							break;
					}
					return false;
		}

		return (pSession->getSessionList()->isUserTextableInCh(mine) ||
				recurCheckAuth(session_auth::exclude_from_room_text_ban, __op_user_room_id)) && 
				(pSession->getSessionList()->isUserTextable(mine));
}
bool RulerChecker::canChangeStyle(SessOperation* opera)	
{
		return recurCheckAuth(session_auth::change_room_mode, pSession->getCur());
}
bool RulerChecker::canSetKeyActive(SessOperation* opera)	
{
		return recurCheckAuth(session_auth::set_others_voice_by_key, opera->user, pSession->getCur());
}
bool RulerChecker::canDisableVoice(SessOperation* opera)			
{
		return pSession->getSessionList()->isUserVoiceable(opera->user) &&
				recurCheckAuth(session_auth::control_user_voice, opera->user, __op_user_room_id);
}
bool RulerChecker::canEnableVoice(SessOperation* opera)			
{
		return (!pSession->getSessionList()->isUserVoiceable(opera->user)) &&
				recurCheckAuth(session_auth::control_user_voice, opera->user,__op_user_room_id);
}
bool RulerChecker::canEnalbeText(SessOperation* opera)			
{
		return (!pSession->getSessionList()->isUserTextable(opera->user)) &&
				recurCheckAuth(session_auth::control_user_text, opera->user, __op_user_room_id);		
}			
bool RulerChecker::canDisabeText(SessOperation* opera)			
{
		return pSession->getSessionList()->isUserTextable(opera->user) &&
					recurCheckAuth(session_auth::control_user_text,opera->user, __op_user_room_id);
}		
bool RulerChecker::canBroadCast(SessOperation* opera)			
{
		return  recurCheckAuth(session_auth::room_broadcast, opera->room);
}		
bool RulerChecker::canDragtoQueue(SessOperation* opera)				
{
				//(!recurCheckAuth(session_auth::exclude_from_queue, opera->user, __op_user_room_id)) && 

		return 	(pSession->getCur() == __op_user_room_id) && 
				pSession->getCurrentStyle() == MicrophoneStyle && 
				recurCheckAuth(session_auth::change_mic_order,  __op_user_room_id) && 
				pSession->getMicrophoneList()->CanTuoRenQueue(opera->user);
}	
bool RulerChecker::canJoinQueue(SessOperation* opera)			
{
	IChannelInfo* info = pSession->getChannelInfo(pSession->getCur());
	if (info == NULL) {
			return false;
	}

	if (!pSession->getSessionList()->isUserVoiceable(mine)) {
		opera->retCode = SessOperation::R_USER_QUEUN_UNABLE;
	}
	if (info->isMicLimitQueue()) {
		opera->retCode = SessOperation::R_VISITOR_QUEUN_UNABLE;
	}
	if (pSession->getMicrophoneList()->getDisable()) {
		opera->retCode = SessOperation::R_PROHIBIT_QUEUN_UNABLE;
	}
	return  pSession->getCurrentStyle() == MicrophoneStyle && 
				pSession->getMicrophoneList()->CanJoin();
}		
bool RulerChecker::canLeaveQueue(SessOperation* opera)			
{
		return  mine == opera->user &&  
				pSession->getCurrentStyle() == MicrophoneStyle && 
				pSession->getMicrophoneList()->CanLeave();
}		
bool RulerChecker::canQueueOrderDown(SessOperation* opera)			
{
		return	pSession->getCurrentStyle() == MicrophoneStyle && 
				recurCheckAuth(session_auth::change_mic_order,  pSession->getCur()) && 
				pSession->getMicrophoneList()->CanMove(opera->user,true);
}	
bool RulerChecker::canQueueOrderUp(SessOperation* opera)			
{
		return  pSession->getCurrentStyle() == MicrophoneStyle && 
				recurCheckAuth(session_auth::change_mic_order,  pSession->getCur()) && 
				pSession->getMicrophoneList()->CanMove( opera->user,false);
}	
bool RulerChecker::canKickFromQueue(SessOperation* opera)		
{
		return	pSession->getCurrentStyle() == MicrophoneStyle && 
				recurCheckAuth(session_auth::add_remove_from_mic_queue,  pSession->getCur());
}			
bool RulerChecker::canMuteQueue(SessOperation* opera)		
{
		return  pSession->getCurrentStyle() == MicrophoneStyle && 
				(!pSession->getMicrophoneList()->getMute()) &&
				recurCheckAuth(session_auth::change_mic_owner, pSession->getCur());
}			
bool RulerChecker::canUnMuteQueue(SessOperation* opera)			
{
		return  pSession->getCurrentStyle() == MicrophoneStyle && 
				(pSession->getMicrophoneList()->getMute()) &&
				recurCheckAuth(session_auth::change_mic_owner,  pSession->getCur());
}		
bool RulerChecker::canDoubleTimeQueue(SessOperation* opera)			
{

		return  pSession->getCurrentStyle() == MicrophoneStyle && 
				(pSession->getMicrophoneList()->CanDoubleTime(opera->user)) &&
				recurCheckAuth(session_auth::change_mic_time,  pSession->getCur());
}		
bool RulerChecker::canDisableQueue(SessOperation* opera)		
{
		return  pSession->getCurrentStyle() == MicrophoneStyle && 
				(!pSession->getMicrophoneList()->getDisable()) &&
				recurCheckAuth(session_auth::change_mic_mode,  pSession->getCur());
}			
bool RulerChecker::canAllowQueue(SessOperation* opera)		
{
		return pSession->getCurrentStyle() == MicrophoneStyle && 
				(pSession->getMicrophoneList()->getDisable()) &&
				recurCheckAuth(session_auth::change_mic_mode,  pSession->getCur());
}			
bool RulerChecker::canDragPeople(SessOperation* opera)		
{
		return  recurCheckAuth(session_auth::schedule_people, opera->user,__op_user_room_id);
}			
bool RulerChecker::canDropPeople(SessOperation* opera)		
{
		__op_user_room_id = opera->room;

		return  (__op_user_room_id !=pSession->getSessionList()->innerGetUserPid(opera->user)) &&  
				recurCheckAuth(session_auth::schedule_people, opera->user, __op_user_room_id);
}	

bool RulerChecker::canDragRoom(SessOperation* opera)		
{
		return recurCheckAuth(session_auth::schedule_room, SID_NULL, opera->room);
}
bool RulerChecker::canDropRoom(SessOperation* opera)		
{
		return recurCheckAuth(session_auth::schedule_room, SID_NULL, opera->room);
}


bool RulerChecker::canEditChannelInfo(SessOperation* opera)	
{
		return recurCheckAuth(session_auth::edit_room_info, opera->room);
}

bool RulerChecker::canShowAsManager(SessOperation* opera)
{
			ChannelRoler roler = getRoller(opera->user, pSession->getCur());
			SID_T uroom = opera->room;
			SID_T nr = getParent(opera->user);
			
			SID_T rr = getParent(uroom);

			return (roler == SA ||
					roler == KEFU || 
					roler == OWNER || 
					roler == MANANGER ||
					(roler == PMANAGER && (uroom == nr || rr == nr))|| 
					(roler == CMANAGER && uroom == nr));
}
bool RulerChecker::canDisableAllText(SessOperation* opera)
{
		return (pSession->IsChannelTextable(opera->room)) && 
				recurCheckAuth(session_auth::control_room_text, opera->room);
}
bool RulerChecker::canEnableAllText(SessOperation* opera)
{
		
		return (!pSession->IsChannelTextable(opera->room)) && 
				recurCheckAuth(session_auth::control_room_text, opera->room);
}
bool RulerChecker::canSetAllKetActive(SessOperation* opera)	
{
		return recurCheckAuth(session_auth::set_room_voice_by_key, opera->room);
}
bool RulerChecker::canAddVip(SessOperation* opera)
{
		OPERATOR_CANNOT_SELF(opera->user)

		ChannelRoler roler = getRoller(opera->user, __op_user_room_id);

		ROLE_CANNOT_CHANGE(roler, VIP)


		return  roler != VIP && 
				recurCheckAuth(session_auth::add_remove_room_vip,opera->user, __op_user_room_id);
}		
bool RulerChecker::canRemoveVip(SessOperation* opera)
{
		OPERATOR_CANNOT_SELF(opera->user)

		ChannelRoler roler = getRoller(opera->user, __op_user_room_id);
		return  roler == VIP && 
				recurCheckAuth(session_auth::add_remove_room_vip,opera->user, __op_user_room_id);
}		
bool RulerChecker::canAddTempVip(SessOperation* opera)
{
		OPERATOR_CANNOT_SELF(opera->user)

		ChannelRoler roler = getRoller(opera->user, __op_user_room_id);
		ROLE_CANNOT_CHANGE(roler, TMPVIP)
		return  roler != TMPVIP &&
				recurCheckAuth(session_auth::add_remove_room_tvip,opera->user, __op_user_room_id);
}		
bool RulerChecker::canRemoveTempVip(SessOperation* opera)
{
		OPERATOR_CANNOT_SELF(opera->user)

		ChannelRoler roler = getRoller(opera->user, __op_user_room_id);
		return  roler == TMPVIP &&
				recurCheckAuth(session_auth::add_remove_room_tvip, opera->user, __op_user_room_id);
}		
bool RulerChecker::canAddBanID(SessOperation* opera)	
{
		//pSession->getSessionList()->isUserInBanlist();
		return   recurCheckAuth(session_auth::add_remove_room_ban_list, opera->user, __op_user_room_id);
}	
bool RulerChecker::canRemoveBanID(SessOperation* opera)
{
		//pSession->getSessionList()->isUserInBanlist();
		return recurCheckAuth(session_auth::add_remove_room_ban_list,  opera->user, __op_user_room_id);
}		
bool RulerChecker::canAddBanIp(SessOperation* opera)	
{
		return  recurCheckAuth(session_auth::add_remove_room_ban_list, opera->user, __op_user_room_id);
}	
bool RulerChecker::CanRemoveBanIp(SessOperation* opera)	
{
		return recurCheckAuth(session_auth::add_remove_room_ban_list, opera->user, __op_user_room_id);
}	

bool RulerChecker::canMoveTopQueue(SessOperation* opera)	
{
		return recurCheckAuth(session_auth::change_mic_order, SID_NULL, pSession->getCur()) && 
				pSession->getMicrophoneList()->CanTopQueue(opera->user);		
}	

bool RulerChecker::canQuitGuild(SessOperation* opera)
{
		return recurCheckAuth(session_auth::withdraw_from_guild, 0);
}
bool RulerChecker::canClearQueue(SessOperation* opera)		
{
		return recurCheckAuth(session_auth::change_mic_mode, opera->room);
}
bool RulerChecker::canIngoreText(SessOperation* opera)	
{
	return (opera->user != mine) && (!pSession->IsIgnoreText(opera->user)) ;
}	
bool RulerChecker::canIngoreVoice(SessOperation* opera)	
{
	return (opera->user != mine) && (!pSession->IsIgnoreVoice(opera->user));
}	
bool RulerChecker::canChangeOrder(SessOperation* opera)	
{
	return recurCheckAuth(session_auth::change_sess_order, 0);
}

bool RulerChecker::canMemberText(SessOperation* opera)	
{
	return recurCheckAuth(session_auth::member_text,0);
}

bool RulerChecker::canUpdateMemberCard(SessOperation* opera)	
{
	if (pSession->getSessionList()->isSelfUpdateMemberCards(opera->user)) {
		OPERATOR_CAN_SELF(opera->user)
	}
	if (pSession->getSessionList()->isAdminUpdateMemberCards(opera->user)) {
		return recurCheckAuth(session_auth::update_member_car, opera->user,0);
	}
	return false;
}


bool RulerChecker::canAddQueueGuest(SessOperation* opera)	
{
	if (pSession->getSessionList()->innerGetUserPid(opera->user) != pSession->getCur()) {
		return false;
	}
	return  pSession->getCurrentStyle() == MicrophoneStyle && 
				(pSession->getMicrophoneList()->canAddGuest(opera->user)) &&
				recurCheckAuth(session_auth::change_mic_owner,  pSession->getCur());
}


bool RulerChecker::canRemoveQueueGuest(SessOperation* opera)	
{
	OPERATOR_CAN_SELF(opera->user)
	return  pSession->getCurrentStyle() == MicrophoneStyle && 
				(pSession->getMicrophoneList()->canRemoveGuest(opera->user)) &&
				recurCheckAuth(session_auth::change_mic_owner,  pSession->getCur());
}

bool RulerChecker::canClearQueueGuest(SessOperation* opera)	
{
	return  pSession->getCurrentStyle() == MicrophoneStyle && 
				(pSession->getMicrophoneList()->canClearGuests()) &&
				recurCheckAuth(session_auth::change_mic_owner,  pSession->getCur());
}
