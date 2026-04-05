#include "microphonelistimp.h"
#include "helper/utf8to.h"
#include "common/core/ilink.h"
#include "SessionImp.h"
#include "protocol/psession.h"
#include "SessionMemberListImp.h"
#include "common/core/form.h"
#include "common/res_code.h"

using namespace protocol::session;
using namespace protocol;
using namespace core;
using namespace xproto;

BEGIN_REQUEST_MAP_EXT(MicrophoneListImp)
	REQUEST_HANDLER(XCast<PDisableQueue>,	onDisableQueue)
	REQUEST_HANDLER(XCast<PJoinQueue>,		onJoinQueue)
	REQUEST_HANDLER(XCast<PJoinQueueRes>,	onMulJoinQueue)
	REQUEST_HANDLER(XCast<PLeaveQueue>,		onLeaveQueue)
	REQUEST_HANDLER(XCast<PKickOffQueue>,	onKickOffQueue)
	REQUEST_HANDLER(XCast<PDoubleTimeQueue>,onDoubleTimeQueue)
	REQUEST_HANDLER(XCast<PMuteQueue>,		onMuteQueue)
	REQUEST_HANDLER(XCast<PMoveQueue>,		onMoveQueue)
	REQUEST_HANDLER(XCast<PTurnQueue>,		onTurn)
	REQUEST_HANDLER(XCast<PTimeOutQueue>,	onTimeout)
	REQUEST_HANDLER(XCast<PTuoRenQueue>,	onTuoRenQueue)
	REQUEST_HANDLER(XCast<PKickAllQueue>,	onKickAllQueue)
	REQUEST_HANDLER(XCast<PTopQueue>,		onTopQueue)
	REQUEST_HANDLER(XCast<PMaixuJoin>,		OnSync)
	REQUEST_HANDLER(XCast<PQueueAddGuest>,	onAddGuest)
	REQUEST_HANDLER(XCast<PQueueRemoveGuest>,		onRemoveGuest)
	REQUEST_HANDLER(XCast<PQueueClearGuest>,		onClearGuest)
END_REQUEST_MAP()


MicrophoneListImp::MicrophoneListImp():pSession(NULL), hasGuests(false)
{
}

MicrophoneListImp::~MicrophoneListImp(void)
{
}

SID_T  MicrophoneListImp::getAliaseId(const SID_T& hid) const
{
	return hid;
}

void MicrophoneListImp::setSession(SessionImp* s)
{
	pSession = s;
}

bool MicrophoneListImp::isFolder(const HULI& id) const
{
	assert(false);
	return false;
}

LPCWSTR MicrophoneListImp::getNick(const UID_T& uid) const
{
	return pSession->getSessionList()->getNick(ITreeNode(uid, ITreeNode::E_USER));
}

void MicrophoneListImp::getParent(const HULI& hid, std::vector<HULI>& parents) const
{
}


IListBase::PlainTree_t MicrophoneListImp::getTreeData() const
{
	IListBase::PlainTree_t ret;
	std::list<UID_T>::const_iterator it = channelqueue.getUserList().begin();
	for( ; it!=channelqueue.getUserList().end(); ++it)
	{
		ret.push_back( std::make_pair( *it, UID_NULL));
	}
	return ret;
}

void MicrophoneListImp::fillTreeData(IListFiller*,int) const
{
}

bool MicrophoneListImp::hasQueue(const UID_T& uid)
{
	return std::find(
					channelqueue.getUserList().begin(),
					channelqueue.getUserList().end(),
					uid)
			!=channelqueue.getUserList().end();
}

bool MicrophoneListImp::CanJoin()
{
	IChannelInfo* info = pSession->getChannelInfo(pSession->getCur());
	if (info == NULL) {
			return false;
	}

	bool normal_limit = pSession->GetRoler(pSession->GetUid(), 0) == NORMAL 
						&& info->isMicLimitQueue() == true;
	return !getDisable() && !hasQueue(pSession->GetUid()) && !normal_limit && pSession->getSessionList()->isUserVoiceable(pSession->GetUid());
}

bool MicrophoneListImp::CanLeave()
{
	return hasQueue(pSession->GetUid());
}

bool MicrophoneListImp::CanMove(const UID_T& target,bool down)
{
	const std::list<UID_T> &reflist = channelqueue.getUserList();
	std::list<UID_T>::const_iterator itmov;

	if(!hasQueue(target)) return false;

	if(down)
	{
		if( reflist.empty() || reflist.size()<=2 || reflist.front()==target || reflist.back()==target ) 
			return false;
	} else {
		if( reflist.empty() || reflist.size()<=2 || reflist.front()==target ) 
			return false;
		itmov=reflist.begin();
		itmov++;
		if( *itmov==target ) return false;
	}
	return true;
}

bool MicrophoneListImp::CanSay(const UID_T& target)
{
	if (guests.find(target) != guests.end()) return true;
	if( channelqueue.mute ) return false;
	if( channelqueue.getUserList().empty() ) return false;

	return channelqueue.getUserList().front() == target;
}


bool MicrophoneListImp::CanDoubleTime(const UID_T& target)
{
	return CanSay(target);
}

bool MicrophoneListImp::CanTuoRenQueue(const UID_T& target)
{
	return	pSession->getCurrentStyle()==MicrophoneStyle &&
			!getDisable() && 
			!hasQueue(target) && 
			pSession->getSessionList()->isSameParent(pSession->GetUid(),target) &&
			pSession->getSessionList()->isUserVoiceable(pSession->GetUid());
}

bool MicrophoneListImp::CanTopQueue(const UID_T& target)
{
	const std::list<UID_T> &reflist = channelqueue.getUserList();
	if ( reflist.empty() || reflist.size()<=2 ) {
		return false;
	}
	std::list<UID_T>::const_iterator itmov=reflist.begin();

	for(int i = 0; i < 2; ++i){
		if(*itmov == target)
			return false;
		++itmov;
	}
	return true;
}

void MicrophoneListImp::OnChangeStyle(const UID_T& sid, ChannelStyle style, uint32_t microtime)
{
	channelqueue.mute=false;
	channelqueue.disable=false;
	channelqueue.ring=microtime;
	channelqueue.count=0;
	channelqueue.validring=microtime;
	channelqueue.getUserListRef().clear();
	_onClearGuest();

	XConnPoint<IListEvent>::forEachWatcher0(&IListEvent::onRefresh);
}


bool MicrophoneListImp::getDisable(void)
{
	return channelqueue.disable;
}

bool MicrophoneListImp::getMute(void)
{
	return channelqueue.mute;
}

void MicrophoneListImp::joinQueue(void)
{
	if (pSession->getAuthChecker()->canDo(&SessOperation(SessOperation::JOIN_QUEUE,	SID_NULL, pSession->GetUid() ) )) {
		XCast<PJoinQueue> pj;
		pj.uid=pSession->GetUid();

		pSession->SendNprotoRequest(pj);
	}
}

void MicrophoneListImp::leaveQueue()
{
	if (pSession->getAuthChecker()->canDo(&SessOperation(SessOperation::LEAVE_QUEUE, SID_NULL, pSession->GetUid()) )) {
			XCast<PLeaveQueue> pl;
			pl.uid=pSession->GetUid();
			pSession->SendNprotoRequest(pl);
	}
}

void MicrophoneListImp::kickOff(const UID_T& uid)
{
	if (pSession->getAuthChecker()->canDo(&SessOperation(SessOperation::KICK_QUEUE, SID_NULL,  uid))) {
			XCast<PKickOffQueue> pk;
			pk.uid=uid;
			pSession->SendNprotoRequest(pk);
	}
}

void MicrophoneListImp::doubleTime()
{
	if (pSession->getAuthChecker()->canDo(&SessOperation(SessOperation::DOUBLETIME_QUEUE,
								SID_NULL, channelqueue.getUserList().front() ))) {
			if( !channelqueue.getUserList().empty() )
			{
				XCast<PDoubleTimeQueue> pd;
				//pd.uid=pSession->GetUid();
				pd.uid=channelqueue.getUserList().front();
				pSession->SendNprotoRequest(pd);
			}
	}
}

void MicrophoneListImp::mute(bool mute)
{
	XCast<PMuteQueue> pm;
	pm.uid=pSession->GetUid();
	pm.time=0;
	pm.mute=mute;
	pSession->SendNprotoRequest(pm);
}

void MicrophoneListImp::move(const UID_T& uid, bool down)
{
	SessOperation::OpCode opcode = (down == true) ?
					SessOperation::MOVEDOWN_QUEUE : SessOperation::MOVEUP_QUEUE;
	if (pSession->getAuthChecker()->canDo(&SessOperation(opcode, SID_NULL, uid))) {
			XCast<PMoveQueue> pm;
			if(!uid) return;
			pm.uid = uid;
			pm.down=down;
			pSession->SendNprotoRequest(pm);
	}
}

uinfo::SEX MicrophoneListImp::getSex(const UID_T& uid)
{
	return pSession->getSessionList()->getSex(uid);
}

ChannelRoler MicrophoneListImp::getRoler(const UID_T &hid, const SID_T &channelId)
{
	return pSession->GetRoler(hid,pSession->getCur());
}

void MicrophoneListImp::disableQueue(void)
{
	XCast<PDisableQueue> pd;
	pd.uid=pSession->GetUid();
	pd.disable=true;
	pSession->SendNprotoRequest(pd);
}

void MicrophoneListImp::allowQueue(void)
{
	XCast<PDisableQueue> pd;

	pd.uid=pSession->GetUid();
	pd.disable=false;
	pSession->SendNprotoRequest(pd);
}

int MicrophoneListImp::getTime()
{
	if(channelqueue.getUserList().empty()) return 0;
	return channelqueue.validring-channelqueue.count;
}

void MicrophoneListImp::tuorenQueue(const UID_T& uid)
{
	XCast<PTuoRenQueue> pt;
	pt.uid = uid;
	pSession->SendNprotoRequest(pt);
}

void MicrophoneListImp::kickAllOff()
{
	XCast<PKickAllQueue> pk;
	pSession->SendNprotoRequest(pk);
}

void MicrophoneListImp::move2top(const UID_T& uid) 
{
	if (pSession->getAuthChecker()->canDo(&SessOperation(SessOperation::KICK_QUEUE,	SID_NULL, uid))) {
			XCast<PTopQueue> pt;
			if( uid == 0) return;
			pt.uid = uid;
			pSession->SendNprotoRequest(pt);
	}
}

//////////////////////////////////////////////////////////////////////////

void MicrophoneListImp::onChange(const HULI &id, const HULI &p)
{
	const std::list<UID_T> &ref = channelqueue.getUserList();
	if(ref.empty()) return;
	if( find(ref.begin(), ref.end(),id ) == ref.end() ) return;

	XConnPoint<IListEvent>::forEachWatcher2(&IListEvent::onChange,
											id,
											SID_NULL);
}

void MicrophoneListImp::clearQueue(){
	channelqueue.getUserListRef().clear();
	channelqueue.mute = true;
	channelqueue.disable = true;
	//_onClearGuest();
}


/************************************************************************/
/* NetWork Commands                                                     */
/************************************************************************/


void MicrophoneListImp::onDisableQueue(XCast<PDisableQueue>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		channelqueue.disable=cmd.disable;

		if(cmd.disable){
				XConnPoint<IMicrophoneListEvent>::forEachWatcher1(&IMicrophoneListEvent::onDisable,
																cmd.uid);	
		}else{
				XConnPoint<IMicrophoneListEvent>::forEachWatcher1(&IMicrophoneListEvent::onAllow,
																cmd.uid);		
		}
	}
}
void MicrophoneListImp::onJoinQueue(XCast<PJoinQueue>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		channelqueue.getUserListRef().push_back(cmd.uid);
		XConnPoint<IListEvent>::forEachWatcher2(&IListEvent::onAdd,
												cmd.uid,
												HULI_NULL);
	}
}
void MicrophoneListImp::onMulJoinQueue(XCast<PJoinQueueRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		for(std::vector<uint32_t>::iterator it = cmd.uids.begin(); it != cmd.uids.end(); ++it){
			channelqueue.getUserListRef().push_back(*it);
		}
		XConnPoint<IListEvent>::forEachWatcher0(&IListEvent::onRefresh);
	}
}
void MicrophoneListImp::onLeaveQueue(XCast<PLeaveQueue>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		channelqueue.getUserListRef().remove(cmd.uid);

		XConnPoint<IListEvent>::forEachWatcher2(&IListEvent::onRemove,
												cmd.uid,
												HULI_NULL);

		XConnPoint<IMicrophoneListEvent>::forEachWatcher1(&IMicrophoneListEvent::onLeave,
														cmd.uid);
	}
}
void MicrophoneListImp::onKickOffQueue(XCast<PKickOffQueue>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		channelqueue.getUserListRef().remove(cmd.uid);
		if( channelqueue.getUserListRef().empty() ) 
		{
			channelqueue.count=channelqueue.validring;
		}
		
		XConnPoint<IListEvent>::forEachWatcher2(&IListEvent::onRemove,
												cmd.uid,
												HULI_NULL);

		XConnPoint<IMicrophoneListEvent>::forEachWatcher2(&IMicrophoneListEvent::onKickOffQueue,
															cmd.admin,
															cmd.uid);
	}	
}

void MicrophoneListImp::onDoubleTimeQueue(XCast<PDoubleTimeQueue>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		channelqueue.validring=cmd.time;

		XConnPoint<IMicrophoneListEvent>::forEachWatcher3(&IMicrophoneListEvent::onDoubleTimeQueue,
															cmd.admin,
															cmd.uid,
															cmd.time);

	}
}
void MicrophoneListImp::onMuteQueue(XCast<PMuteQueue>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		channelqueue.mute=cmd.mute;
		channelqueue.count=0;
		channelqueue.validring=cmd.time;

		XConnPoint<IMicrophoneListEvent>::forEachWatcher2(&IMicrophoneListEvent::onMute,													
															cmd.uid,
															cmd.mute);
	}
}
void MicrophoneListImp::onMoveQueue(XCast<PMoveQueue>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		uint32_t who = cmd.uid;
		bool down = cmd.down;
		std::list<uint32_t> &reflist = channelqueue.getUserListRef();
		std::list<uint32_t>::iterator itmov;
		if(down)
		{
			if( reflist.empty() || reflist.size()<=2 || reflist.front()==who || reflist.back()==who ) return;

			itmov=reflist.begin();
			itmov++;

			for(; itmov!=reflist.end(); itmov++)
			{
				if( *itmov==who )
				{
					itmov++;
					reflist.remove(who);
					break;
				}
			}
			reflist.insert(++itmov,who);
		}
		else
		{
			if( reflist.empty() || reflist.size()<=2 || reflist.front()==who ) return;

			itmov=reflist.begin();
			itmov++;

			if( *itmov==who ) return;

			for(; itmov!=reflist.end(); itmov++){
				if( *itmov==who ){
					itmov++;
					reflist.remove(who);
					break;
				}
			}

			reflist.insert(--itmov,who);	
		}
		XConnPoint<IListEvent>::forEachWatcher0(&IListEvent::onRefresh);
	}
}
void MicrophoneListImp::onTurn(XCast<PTurnQueue>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		
		channelqueue.count=0;
		channelqueue.validring=cmd.time;
		channelqueue.ring=cmd.time;
		XConnPoint<IMicrophoneListEvent>::forEachWatcher2(&IMicrophoneListEvent::onTurn,													
															cmd.uid,
															cmd.time);
	}
}

void MicrophoneListImp::onTimeout(XCast<PTimeOutQueue>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		channelqueue.getUserListRef().remove(cmd.uid);

		XConnPoint<IListEvent >::forEachWatcher2(&IListEvent::onRemove,
												cmd.uid,
												HULI_NULL);

		XConnPoint<IMicrophoneListEvent>::forEachWatcher1(&IMicrophoneListEvent::onTimeout,													
															cmd.uid);
	}
}
void MicrophoneListImp::onTuoRenQueue(XCast<PTuoRenQueue>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		XCast<PJoinQueue> pj;
		pj.uid = cmd.uid;
		pj.resCode = RES_SUCCESS;

		onJoinQueue(pj, c);

		if(cmd.uid == pSession->GetUid())
		{
			XConnPoint<IMicrophoneListEvent>::forEachWatcher2(&IMicrophoneListEvent::onDrag,													
																cmd.admin,
																cmd.uid);
		}
	}
}
void MicrophoneListImp::onKickAllQueue(XCast<PKickAllQueue>& cmd, nproto::connection* c)
{
	channelqueue.getUserListRef().clear();
	//_onClearGuest();

	XConnPoint<IListEvent>::forEachWatcher0(&IListEvent::onRefresh);
	XConnPoint<IMicrophoneListEvent>::forEachWatcher1(&IMicrophoneListEvent::onClearQueue,													
														cmd.admin);
}
void MicrophoneListImp::onTopQueue(XCast<PTopQueue>& cmd, nproto::connection* c)
{
	std::list<uint32_t> &reflist = channelqueue.getUserListRef();
	std::list<uint32_t>::iterator itmov;

	if( cmd.uid == 0 ) return;

	if ( reflist.empty() || reflist.size()<=2 ) {
		return;
	}

	itmov=reflist.begin();
	itmov++;
	if( *itmov == cmd.uid ) {
		return;
	}

	reflist.remove(cmd.uid);
	reflist.insert(itmov,cmd.uid);

	XConnPoint<IListEvent>::forEachWatcher0(&IListEvent::onRefresh);
}

void MicrophoneListImp::OnSync(XCast<PMaixuJoin>& cmd, nproto::connection* c)
{
	channelqueue = cmd;
	guests = cmd.guests;
	//hasGuests = cmd.hasGuests;
	XConnPoint<IListEvent>::forEachWatcher0(&IListEvent::onRefresh);
	XConnPoint<IMicrophoneListEvent>::forEachWatcher0(&IMicrophoneListEvent::onSync);

	//if (hasGuests) {
	//	XConnPoint<IMicrophoneListEvent>::forEachWatcher0(&IMicrophoneListEvent::onGuestSync);
	//}
}


void MicrophoneListImp::onAddGuest(XCast<PQueueAddGuest>& cmd, nproto::connection* c) {
	_onAddGuest(cmd.admin, cmd.uid);
}

void MicrophoneListImp::_onAddGuest(uint32_t admin, uint32_t uid) {
	guests.insert(uid);
	XConnPoint<IMicrophoneListEvent>::forEachWatcher2(&IMicrophoneListEvent::onGuestAdd, admin, uid);
}

void MicrophoneListImp::onRemoveGuest(XCast<PQueueRemoveGuest>& cmd, nproto::connection* c) {
	_onRemoveGuest(cmd.admin, cmd.uid);
}
void MicrophoneListImp::_onRemoveGuest (uint32_t admin, uint32_t uid) {
	guests.erase(uid);
	XConnPoint<IMicrophoneListEvent>::forEachWatcher2(&IMicrophoneListEvent::onGuestRemove, admin, uid);
}

void MicrophoneListImp::onClearGuest(XCast<PQueueClearGuest>& cmd, nproto::connection* c) {
	_onClearGuest(cmd.admin);
}

void MicrophoneListImp::_onClearGuest (uint32_t admin)  {
	guests.clear();
	XConnPoint<IMicrophoneListEvent>::forEachWatcher1(&IMicrophoneListEvent::onGuestClear,  admin);
}

const std::set<uint32_t>& MicrophoneListImp::getGuests() {
	return guests;
}

void MicrophoneListImp::addGuest(const UID_T &uid) {
	if (canAddGuest(uid)) {
			XCast<PQueueAddGuest> addGuest;
			if( uid == 0) return;
			addGuest.uid = uid;
			addGuest.admin = pSession->GetUid();
			pSession->SendNprotoRequest(addGuest);
	}
}

void MicrophoneListImp::removeGuest(const UID_T &uid) {
	if (canRemoveGuest(uid)) {
			XCast<PQueueRemoveGuest> removeGuest;
			if( uid == 0) return;
			removeGuest.uid = uid;
			removeGuest.admin = pSession->GetUid();
			pSession->SendNprotoRequest(removeGuest);
	}
}

void MicrophoneListImp::clearGuests() {
	if (canClearGuests()) {
			XCast<PQueueClearGuest> clearGuest;
			clearGuest.admin = pSession->GetUid();
			pSession->SendNprotoRequest(clearGuest);
	}
}

bool MicrophoneListImp::canAddGuest(const UID_T &uid) {
	if (hasGuests && uid && !isGuest(uid)) {
		return true;
	}
	return false;
}

bool MicrophoneListImp::canRemoveGuest(const UID_T &uid) {
	if (hasGuests == false || !uid || !isGuest(uid)) {
		return false;
	}
	return true;
}

bool MicrophoneListImp::canClearGuests() {
	if (hasGuests == false || guests.empty()) {
		return false;
	}
	return true;
}

bool MicrophoneListImp::isGuest(const UID_T &uid) {
	return guests.find(uid) != guests.end();
}