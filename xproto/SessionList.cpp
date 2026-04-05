#include "sessionlist.h"
#include "sitem.h"
#include "helper/utf8to.h"
#include "timer/ExtraTimer.h"
#include "common/core/request.h"
#include "protocol/pkg_tag.h"
#include "sessioninfo.h"
#include "protolink.h"
#include "helper/base64.h"

#include <json/json.h>

#define REFRESH_TIME	(24*60*1000)

using namespace xproto;
using namespace protocol::slist; // no solution, in order to make faster
using namespace protocol::session;
using namespace protocol::link;
using namespace helper;

BEGIN_REQUEST_MAP_EXT(SessionList)		
	REQUEST_HANDLER(XCast<PSyncSListRes>, 		onSync)
	REQUEST_HANDLER(XCast<PRemoveSessRes>,		onRemoveSession)
	REQUEST_HANDLER(XCast<PRemoveFolderRes>,	onRemoveFolder)
	REQUEST_HANDLER(XCast<PAddFolderRes>,		onAddFolder)
	REQUEST_HANDLER(XCast<PAddSessRes>,			onAddSession)
	REQUEST_HANDLER(XCast<PSessionSyncInfoGet>, onSyncInfo)
	REQUEST_HANDLER(XCast<PSyncSInfoRes>,		onSyncSInfo)
	REQUEST_HANDLER(XCast<PSyncSubListVecRes>, 		onSubSync)
	REQUEST_HANDLER(XCast<PGetFondRes>, onGetFondRes)
	REQUEST_HANDLER(XCast<PUpdateFondRes>, onUpdateFondRes)
END_REQUEST_MAP()


SessionList::SessionList(void)
{
		__protoLink = NULL;
		__refreshTimer.init(this, &SessionList::syncInfo);
}



SessionList::~SessionList(void)
{
}

void SessionList::bindLike() {
		session_likes_proxyer_.init(this, &SessionList::OnUpdateSessionLikes, SESSION_LIKES_PATH);
}

void SessionList::unbindLike() {
		session_likes_proxyer_.uninit();
}

void	SessionList::setProtoLink(xproto::CProtoLink* link)
{
		__protoLink  = link;
}

void SessionList::onSync(XCast<PSyncSListRes>& cmd,
							nproto::connection* c)
{

	if(cmd.resCode == 200)
	{
		last_access_order_ .clear();
		__tree.clear();
		for(std::vector<SListItem>::iterator it = cmd.items.begin(); it != cmd.items.end(); ++it){
			SItem &si = __tree[(*it).sid];
			si.sid = it->sid;
			si.asid = it->asid;
			if(!(*it).snick.empty()){
				si.nick = net2app((*it).snick);
			}
			si.owner = (*it).owner;
			si.bLimit = it->bLimit;
			si.users = it->users;
			si.intro = net2app(it->intro);
			si.slogan = net2app(it->slogan);
			si.lables = net2app(it->lables);
			if(!it->bPublic){
				si.bPub = it->bPublic;
			}
			if(it->pid != 0 && __tree[(*it).sid].hasParent(it->pid) == 0) {
				__tree[(*it).sid].pids.push_back((*it).pid);
			}	
			if (it->pid == RECENT_FOLDER) {
				last_access_order_.push_back(si.sid);
			}
		}


/*		for(std::vector<SListItemAddit>::iterator it = cmd.addItems.begin(); it != cmd.addItems.end(); ++it){
				SItem &si = __tree[(*it).sid];
				si.users = (*it).users;
				si.intro = net2app((*it).intro);
		}
*/
		XConnPoint<IListEvent>::forEachWatcher0(&IListEvent::onRefresh);
		syncInfo();
		__refreshTimer.start(REFRESH_TIME);

		
		
		std::vector<uint32_t> get_likes;
		for (tree_doc_t::const_iterator it = __tree.begin(); it != __tree.end(); ++it) {
			get_likes.push_back(it->first);
		}
		if (get_likes.size()) {
			updateSinfoLikes(get_likes);
		}
		
	}
	else
	{
		XConnPoint<IActionEvent>::forEachWatcher2(&IActionEvent::onResult, (int)SYNC, (int)cmd.resCode);
	}

}

void SessionList::onSubSync(XCast<PSyncSubListVecRes>& cmd,
							nproto::connection* c)
{

	if(cmd.resCode == 200)
	{
		__tree.clear();
		for(std::vector<SListItem>::iterator it = cmd.items.begin(); it != cmd.items.end(); ++it){
			SItem &si = __tree[(*it).sid];
			si.sid = it->sid;
			si.asid = it->asid;
			if(!(*it).snick.empty()){
				si.nick = net2app((*it).snick);
			}
			si.owner = (*it).owner;
			si.bLimit = it->bLimit;
			si.users = it->users;
			si.intro = net2app(it->intro);
			si.slogan = net2app(it->slogan);
			si.lables = net2app(it->lables);
			if(!it->bPublic){
				si.bPub = it->bPublic;
			}
			if(it->pid != 0 && __tree[(*it).sid].hasParent(it->pid) == 0) {
				__tree[(*it).sid].pids.push_back((*it).pid);
			}			
		}


/*		for(std::vector<SListItemAddit>::iterator it = cmd.addItems.begin(); it != cmd.addItems.end(); ++it){
				SItem &si = __tree[(*it).sid];
				si.users = (*it).users;
				si.intro = net2app((*it).intro);
		}
*/
		XConnPoint<IListEvent>::forEachWatcher0(&IListEvent::onRefresh);
		syncInfo();
		__refreshTimer.start(REFRESH_TIME);
	}
	else
	{
		XConnPoint<IActionEvent>::forEachWatcher2(&IActionEvent::onResult, (int)ADDFOLDER, (int)cmd.resCode);
	}

}


void SessionList::onRemoveSession(XCast<PRemoveSessRes>& cmd,
										nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		tree_doc_t::iterator it = __tree.find(cmd.sid);
		if (it != __tree.end()) {
		//for(tree_doc_t::iterator it = __tree.begin(); it != __tree.end(); ++it){
			if((*it).first == cmd.sid){
				int res = (*it).second.hasParent(cmd.pid);
				if(res == 2){
					__tree[cmd.sid].rmParent(cmd.pid);
					XConnPoint<IListEvent>::forEachWatcher2(&IListEvent::onRemove, cmd.sid, cmd.pid);
					//break;
				}else if(res == 1){
					__tree.erase(it);
					XConnPoint<IListEvent>::forEachWatcher2(&IListEvent::onRemove, cmd.sid, cmd.pid);
					//break;
				}
			}
		}

		std::vector<uint32_t> ids;
		ids.push_back(cmd.sid);
		updateSinfoLikes(ids);

	}else{
		XConnPoint<IActionEvent>::forEachWatcher2(&IActionEvent::onResult, (int)DELFAVOR, (int)cmd.resCode);
	}

}
void SessionList::onRemoveFolder(XCast<PRemoveFolderRes>& cmd,	
										nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){

			for(tree_doc_t::iterator it = __tree.begin(); it != __tree.end(); ++it){
				if((*it).first == cmd.fid){
					int res = (*it).second.hasParent(cmd.pid);
					if(res == 2){
						__tree[cmd.fid].rmParent(cmd.pid);
						XConnPoint<IListEvent>::forEachWatcher2(&IListEvent::onChange, cmd.fid, cmd.pid);
					}else if(res == 1){
						__tree.erase(it);
						XConnPoint<IListEvent>::forEachWatcher2(&IListEvent::onRemove, cmd.fid, cmd.pid);
						break;
					}
				}
			}
			for(tree_doc_t::iterator it = __tree.begin(); it != __tree.end(); ++it){
				if((*it).first != cmd.fid){
					int res = (*it).second.hasParent(cmd.fid);

					if(res == 2){
						__tree[(*it).first].rmParent(cmd.pid);
						XConnPoint<IListEvent>::forEachWatcher2(&IListEvent::onChange, cmd.fid, cmd.pid);
					}
				}
			}
	}else{
			XConnPoint<IActionEvent>::forEachWatcher2(&IActionEvent::onResult, (int)DELFOLDER, (int)cmd.resCode);
	}
}
void SessionList::onAddFolder(XCast<PAddFolderRes>& cmd,
									nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		__tree[cmd.fid].nick = net2app(cmd.nick);

		if(cmd.pid != 0)
			__tree[cmd.fid].pids.push_back(cmd.pid);
		XConnPoint<IListEvent >::forEachWatcher2(&IListEvent::onAdd, cmd.fid, cmd.pid);
	}else{
		XConnPoint<IActionEvent>::forEachWatcher2(&IActionEvent::onResult, (int)ADDFOLDER, (int)cmd.resCode);
	}
}

void SessionList::updateSinfo(uint32_t sid, const SItem &s)
{

	//uint32_t asid, const xstring& nick, uint32_t owner, const xstring& intro, uint32_t users, const xstring& slogan
	tree_doc_t::iterator it = __tree.find(sid);
	if (it != __tree.end()) {
		SItem &item = __tree[sid];
		item.sid = sid;
		item.nick = s.nick;
		item.intro = s.intro;
		item.owner = s.owner;
		item.users = s.users;
		item.slogan = s.slogan;
		item.asid = s.asid;
		item.bPub = s.bPub;
		item.bLimit = s.bLimit;
		for (int i = 0; i < item.pids.size(); ++i) {
			XConnPoint<IListEvent >::forEachWatcher2(&IListEvent::onChange, sid, item.pids[i]);
		}
	}
}

void SessionList::updateMyList(const SID_T& sid, bool badd)
{
	tree_doc_t::iterator it = __tree.find(sid);
	if (it == __tree.end() || it->second.pids.size()==0) {
			__tree[sid].sid = sid;
			__tree[sid].asid = 0;
	}
	SItem &item = __tree[sid];

	if (badd) {
		if (item.hasParent(MYCREATED_FOLDER) == 0) {
			item.pids.push_back(MYCREATED_FOLDER);
			XConnPoint<IListEvent >::forEachWatcher2(&IListEvent::onAdd, sid, MYCREATED_FOLDER);
		}
	} else {
		tree_doc_t::iterator it = __tree.find(sid);
		if (it != __tree.end()) {
		//for(tree_doc_t::iterator it = __tree.begin(); it != __tree.end(); ++it){
			if((*it).first == sid){
				int res = (*it).second.hasParent(MYCREATED_FOLDER);
				if(res == 2){
					__tree[sid].rmParent(MYCREATED_FOLDER);
					XConnPoint<IListEvent>::forEachWatcher2(&IListEvent::onRemove, sid, MYCREATED_FOLDER);
					//break;
				}else if(res == 1){
					__tree.erase(it);
					XConnPoint<IListEvent>::forEachWatcher2(&IListEvent::onRemove, sid, MYCREATED_FOLDER);
					//break;
				}
			}
		}
	}
		
	
}

void SessionList::onAddSession(XCast<PAddSessRes>& cmd,
									nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
			SItem &item = __tree[cmd.sid];
			item.sid = cmd.sid;
			item.asid = cmd.asid;
			item.nick = net2app(cmd.nick);
			item.pids.push_back(cmd.pid);
			//item.owner = cmd.owner;
			//item.bLimit = cmd.bLimit;
			//item.slogan = net2app(cmd.slogan);
			//item.lables = net2app(cmd.lables);

			XConnPoint<IListEvent >::forEachWatcher2(&IListEvent::onAdd, cmd.sid, cmd.pid);

			std::vector<uint32_t> ids;
			ids.push_back(cmd.sid);
			updateSinfoLikes(ids);
	}else{
			XConnPoint<IActionEvent>::forEachWatcher2(&IActionEvent::onResult, (int)ADDFAVOR, (int)cmd.resCode);
	}
}
void SessionList::onSyncInfo(XCast<PSessionSyncInfoGet>& cmd,
										nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		//delete top
		ClearHot();
		for(std::vector<SListItem>::iterator it = cmd.items.begin(); it != cmd.items.end(); ++it)
		{
			if(it->pid==HOT_FOLDER){
				if(__tree.find((*it).sid)==__tree.end()){
					SItem &si = __tree[it->sid];
					si.sid = it->sid;
					si.asid = it->asid;
					if(!(it->snick.empty())){
						si.nick = net2app(it->snick);
					}					
					si.owner = (*it).owner;
					si.bLimit = it->bLimit;	
					si.users = it->users;
				}				
				__tree[it->sid].pids.push_back(it->pid);
			} else{
				if(__tree.find(it->sid)!=__tree.end()){
					SItem &si = __tree[it->sid];
					si.sid =  it->sid;
					si.asid = it->asid;
					if(!it->snick.empty()){
						si.nick = net2app(it->snick);
						si.owner = it->owner;
						si.bLimit = it->bLimit;	
					}
					si.users = it->users;
				}
			}
		}
		XConnPoint<IListEvent >::forEachWatcher0(&IListEvent::onRefresh);

	}
	else{
		XConnPoint<IActionEvent>::forEachWatcher2(&IActionEvent::onResult, (int)SYNC, (int)cmd.resCode);
	}
}
void SessionList::onSyncSInfo(XCast<PSyncSInfoRes>& cmd,
									nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		UID_T sid = sox::properties::getint(cmd, SIT_SID);

		
		std::vector<uint32_t> ids;
		ids.push_back(sid);
		updateSinfoLikes(ids);

		sid2info_map_t::iterator it = infoMap.find(sid);
		if(it != infoMap.end()){
			return it->second->setProperty(cmd);
		}
	}
}


void SessionList::ClearHot()
{
	/*xstring sidHot=itow10(HOT_FOLDER);
	xstring sid;
	tree_doc_t::iterator itdel;

	for(tree_doc_t::iterator it = __tree.begin(); it != __tree.end();)
	{
		itdel=it++;
		sid=itdel->first;
		int res = (*itdel).second.hasParent(sidHot);
		if(res == 2)
		{
			__tree[sid].rmParent(sidHot);
		}
		else if(res == 1)
		{
			__tree.erase(itdel);
		}
	}
	*/
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

UID_T SessionList::getOwner(const HULI &fid) const
{
		tree_doc_t::const_iterator it = __tree.find(fid);
		if(it != __tree.end()){
				return it->second.owner;
		}
		return UID_NULL;
}
void SessionList::addFolder(const HULI &pid, const xstring &fdNick)
{
		XCast<PAddFolder> p;
		p.pid = pid;
		p.nick = app2net(fdNick.data());
		sendRequest(p);
}
void SessionList::delFolder(const HULI &fid, const HULI &pid)
{
		XCast<PRemoveFolder> p;
		p.pid = pid;
		p.fid = fid;
		sendRequest(p);
}
void SessionList::addFavor(const HULI &sid, const HULI &pid)
{
		XCast<PAddSess> p;
		p.pid = pid;
		p.sid = sid;
		sendRequest(p);
}
void SessionList::removeFavor(const HULI &sid, const HULI &pid)
{
		XCast<PRemoveSess> p;
		p.pid = pid;
		p.sid = sid;
		sendRequest(p);
}

void SessionList::sync()
{
		XCast<PSyncSListRes> p;
		sendRequest(p);
}

void SessionList::syncSub(const HULI &pid)
{
		XCast<PSyncSubListVec> p;
		p.pids.push_back(pid);
		sendRequest(p);
}

//virtual void checkSub(const HULI $pid, const HULI& sid)
//{
		//XCast<PSyncSubList> p;
		//sendRequest(p);
//}

void SessionList::syncInfo()
{
	XCast<PSessionSyncInfoGet> inforeq;

	inforeq.top=20;
	inforeq.items.clear();
	for(tree_doc_t::const_iterator it = __tree.begin(); it != __tree.end(); ++it)
	{
		if(isFolder(it->first)) continue;
		SListItem item;
		item.sid=it->second.sid;
		if(item.sid==HOT_FOLDER) continue;
		item.users=0;
		inforeq.items.push_back(item);
	}
	sendRequest(inforeq);
}

bool SessionList::isFolder(const HULI &id) const
{
	return id == MYCOLLECTION_FOLDER || 
		   id == PUBLICCHANNEL_FOLDER || 
		   id == RECEMMONDCHANNEL_FOLDER || 
		   id == HOT_FOLDER;
}

bool SessionList::isLimitChannel(const SID_T &cid) const
{
		tree_doc_t::const_iterator it = __tree.find(cid);
		if(it != __tree.end()){
			return it->second.bLimit;
		}else{
			return false;
		}
}
bool SessionList::IsExistSess(const SID_T &sid) const
{
		tree_doc_t::const_iterator it;
		it=__tree.find(sid);
		if ( it!=__tree.end() )
		{
			SItem const &tt = it->second;
			return tt.hasParent(MYCOLLECTION_FOLDER);
		} 
		else 
		{
			for(it=__tree.begin(); it!=__tree.end(); it++)
			{
				if( it->second.asid  == sid )
				{
					SItem const &tt = it->second;
					return tt.hasParent(MYCOLLECTION_FOLDER);
				}
			}
		}
		return false;
}
ISessionInfoBase* SessionList::getSessionInfo(const SID_T &sid, bool update_force)
{
	sid2info_map_t::iterator it = infoMap.find(sid);
	if(it != infoMap.end()){
		if (update_force == false) {
			return it->second;
		}
		std::map<uint32_t, uint32_t>::iterator it_sync = session_last_sync_.find(sid);
		if (it_sync != session_last_sync_.end() && (timeGetTime() - it_sync->second < ( 1000 * 30))) {
			// 如果上一次更新的时候不到半分钟，则不更新资料
			assert(0);
		} else {
			XCast<PSyncSInfo> syncSinfo;
			syncSinfo.sid = sid;
			sendRequest(syncSinfo);
		}
		session_last_sync_[sid] = timeGetTime();
		return it->second;
	}else{
		XCast<PSyncSInfo> syncSinfo;
		syncSinfo.sid = sid;
		sendRequest(syncSinfo);
		session_last_sync_[sid] = timeGetTime();

		SessInfoPtr imp = nsox::nu_create_object<SessionListInfo>::create_inst();
		infoMap[sid] = imp;
		uint32_t asRes = sid;
		imp->initSid(sid, asRes);
		return imp;
	}
}

LPCWSTR SessionList::getNick(const HULI &hid) const
{
		tree_doc_t::const_iterator it = __tree.find(hid);
		if(it != __tree.end() && !it->second.nick.empty()){
			return it->second.nick.c_str();
		}
		return _T("");
}

bool SessionList::isPublic(const HULI& hid )const
{
	tree_doc_t::const_iterator it = __tree.find(hid);
	if(it != __tree.end()){
		return it->second.bPub;
	}
	return true;

}

void SessionList::getParent(const HULI &hid, std::vector<HULI>& parents) const
{
		tree_doc_t::const_iterator it = __tree.find(hid);
		if(it != __tree.end()){
			std::copy(it->second.pids.begin(), it->second.pids.end(), std::back_inserter(parents));
		}else{
			parents.clear();
		}
}
UID_T SessionList::getAliaseId(const HULI &hid) const
{
		tree_doc_t::const_iterator it = __tree.find(hid);
		if(it != __tree.end()){
			return it->second.asid;
		}else{
			return hid;
		}
}
IListBase::PlainTree_t SessionList::getTreeData() const
{
		IListBase::PlainTree_t ret;
		std::set<SID_T> ret_last_access;	
		for(tree_doc_t::const_iterator it = __tree.begin(); it != __tree.end(); ++it){
			const SItem & item= it->second;
			const std::vector<SID_T> &vec = (*it).second.pids;
			if(vec.empty()){
				ret.push_back(std::make_pair(it->first, SID_NULL));
			}else{
				for(std::vector<SID_T>::const_iterator it2 = vec.begin(); it2 != vec.end(); ++it2){
					if (*it2 != RECENT_FOLDER) {
						ret.push_back(std::make_pair(it->first, *it2));
					} else {
						ret_last_access.insert(it->first);
					}
				}
			}
		}
		for(std::list<uint32_t>::const_iterator it = last_access_order_.begin(); it != last_access_order_.end(); ++it) {
			if (ret_last_access.find(*it) != ret_last_access.end()) {
				ret.push_back(std::make_pair(*it, RECENT_FOLDER));
			}
		}
		return ret;
}

void SessionList::fillTreeData(IListFiller*,int) const
{
}

bool SessionList::isExisit(const HULI& hid, const HULI& pid) const
{
		return false;
}

xstring  SessionList::getIntro(const HULI& cid) const
{
	tree_doc_t::const_iterator it= __tree.find(cid);
	if(it !=__tree.end()) {
		return it->second.intro;
	}
	sid2info_map_t::const_iterator it_info = infoMap.find(cid);
	if(it_info != infoMap.end()){
		return it_info->second->getIntroduce();
	}
	return _T("");
}

uint32_t SessionList::getOnline(const SID_T &cid) const
{
		tree_doc_t::const_iterator it= __tree.find(cid);
		if(it !=__tree.end())  {
			return it->second.users;
		}
		
		sid2info_map_t::const_iterator it_info = infoMap.find(cid);
		if(it_info != infoMap.end()){
			return it_info->second->getOnlineCount();
		}
		return 0;

		//if(__tree.find(cid)==__tree.end()) return 0;
		//return __tree[cid].users;
}

xstring SessionList::getLables(const HULI &hid) const
{
	tree_doc_t::const_iterator it = __tree.find(hid);
	if(it != __tree.end() && !it->second.lables.empty()){
		return it->second.lables;
	}
	sid2info_map_t::const_iterator it_info = infoMap.find(hid);
	if(it_info != infoMap.end()){
		return it_info->second->getLables();
	}
	return _T("");
}

xstring SessionList::getSlogan(const HULI &hid) const
{
	tree_doc_t::const_iterator it = __tree.find(hid);
	if(it != __tree.end() && !it->second.slogan.empty()){
		return it->second.slogan;
	}
	sid2info_map_t::const_iterator it_info = infoMap.find(hid);
	if(it_info != infoMap.end()){
		return it_info->second->getSlogan();
	}
	return _T("");
}

int SessionList::isFond(const HULI &sid)
{
		if (beGetFonds.find(sid) == beGetFonds.end()) {
				XCast<PGetFond> p;
				p.sid = sid;
				sendRequest(p);
				return -1;
		}

		tree_doc_t::const_iterator it;
		it=__tree.find(sid);
		if ( it!=__tree.end() ) {
			SItem const &tt = it->second;
			return (int)tt.hasParent(MYFOND_FOLDER);
		} else {
			for(it=__tree.begin(); it!=__tree.end(); it++) {
				if( it->second.asid  == sid ) {
					SItem const &tt = it->second;
					return (int) tt.hasParent(MYFOND_FOLDER);
				}
			}
		}
		return 0;
}

void SessionList::setFond(const HULI &sid, bool bFond)
{
	if (isFond(sid) != -1) {
			bFond = !isFond(sid);
	}
	//if (isFond(sid) == (int)bFond) {
	//		return;
	//}
	XCast<PUpdateFond> p;
	p.sid = sid;
	p.fond = bFond;
	sendRequest(p);
}

void SessionList::updateFonds()
{
	syncSub(MYFOND_FOLDER);
}

void SessionList::onUpdateFondRes(XCast<PUpdateFondRes>& cmd,	 nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		uint32_t sid = cmd.sid;
		beGetFonds.insert(sid);
		bool badd = cmd.bFond;
		tree_doc_t::iterator it = __tree.find(sid);
		if (it == __tree.end() || it->second.pids.size()==0) {
				__tree[sid].sid = sid;
				__tree[sid].asid = 0;
		}
		SItem &item = __tree[sid];

		if (badd) {
			if (item.hasParent(MYFOND_FOLDER) == 0) {
				item.pids.push_back(MYFOND_FOLDER);
			}
		} else {
			tree_doc_t::iterator it = __tree.find(sid);
			if (it != __tree.end()) {
				if((*it).first == sid){
					int res = (*it).second.hasParent(MYFOND_FOLDER);
					if(res == 2){
						__tree[sid].rmParent(MYFOND_FOLDER);
					}else if(res == 1){
						__tree.erase(it);
					}
				}
			}
		}
		XConnPoint<ISessListEvent>::forEachWatcher1(&ISessListEvent::onFondResult, sid);
	}
}


void SessionList::onGetFondRes(XCast<PGetFondRes>& cmd,	 nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode))
	{
		uint32_t sid = cmd.sid;
		beGetFonds.insert(sid);
		bool badd = cmd.fond;
		tree_doc_t::iterator it = __tree.find(sid);
		if (it == __tree.end() || it->second.pids.size()==0) {
				__tree[sid].sid = sid;
				__tree[sid].asid = 0;
		}
		SItem &item = __tree[sid];

		if (badd) {
			if (item.hasParent(MYFOND_FOLDER) == 0) {
				item.pids.push_back(MYFOND_FOLDER);
			}
		} else {
			tree_doc_t::iterator it = __tree.find(sid);
			if (it != __tree.end()) {
				if((*it).first == sid){
					int res = (*it).second.hasParent(MYFOND_FOLDER);
					if(res == 2){
						__tree[sid].rmParent(MYFOND_FOLDER);
					}else if(res == 1){
						__tree.erase(it);
					}
				}
			}
		}
		XConnPoint<ISessListEvent>::forEachWatcher1(&ISessListEvent::onFondResult, sid);
	}
}
		
void SessionList::updateSinfoLikes(const std::vector<uint32_t>& sids) {
	bindLike();
	try {
		Json::Value param;
		for(int i = 0; i < sids.size(); ++i) {
			param["sids"].append(sids[i]);
		}
		Json::FastWriter fast_writer;
		std::string strJson = fast_writer.write(param);
		session_likes_proxyer_.request(0,net2app(strJson), _T(""));
	} catch(...) {
	}
}

void SessionList::OnUpdateSessionLikes(const xstring& method, uint32_t resCode, const xstring& res, uint32_t idd) {
	try {
		if (resCode != 200) {
				return;
		}
		Json::Reader reader;
		Json::Value root;
		std::string str = Base64::decode(app2net(res)); //TODO FIX_ME
		if( !reader.parse(str, root) ) {
			assert(FALSE);
			return;
		}
		Json::Value elem = root["likes"];
		if (!elem.isArray()) {
			return;
		}
		int sid = 0;
		int online = 0;
		int collect = 0;
		int members = 0;
		int fond = 0;
		
		for (Json::UInt i = 0; i < elem.size(); ++i) {
			Json::Value node = elem[i];
			if (!node.isArray() || node.size() != 5) {
				return;
			}
			Json::UInt j = 0;
			sid = node[j].asInt();
			online = node[++j].asInt();
			collect = node[++j].asInt();
			members = node[++j].asInt();
			fond = node[++j].asInt();
			sid2info_map_t::iterator it = infoMap.find(sid);
			if(it != infoMap.end()){
				sox::Properties p;
				if (online)
					sox::properties::setint(p, SIT_ONLINECOUNT, online);
				if (collect)
					sox::properties::setint(p, SIT_COLLECTIONCOUNT, collect);
				if (members)
					sox::properties::setint(p, SIT_MEMBERCOUNT, members);
				if (fond)
					sox::properties::setint(p, SIT_FONDCOUNT, fond);
				it->second->setProperty(p);
				//it->second->u
			}
			tree_doc_t::iterator it_tree = __tree.find(sid);
			if (it_tree != __tree.end()) {
				it_tree->second.collect = collect; 
				it_tree->second.member = members;
				it_tree->second.users = online;
				it_tree->second.fond = fond;
			}
		}

	} catch(...) {
	}
		
}

int  SessionList::getCollect(const HULI &hid) const
{
		int collect = 0;
		tree_doc_t::const_iterator it = __tree.find(hid);
		if(it != __tree.end()){
			collect =  it->second.collect;
		}

		sid2info_map_t::const_iterator it_info = infoMap.find(hid);
		if(it_info != infoMap.end()){
			return max(collect, it_info->second->getCollectionCount());
		}

		return collect;
}

int  SessionList::getMember(const HULI &hid) const
{
		int member = 0;
		tree_doc_t::const_iterator it = __tree.find(hid);
		if(it != __tree.end()){
			member =  it->second.member;
		}
		sid2info_map_t::const_iterator it_info = infoMap.find(hid);
		if(it_info != infoMap.end()){
			return max(member, it_info->second->getMemberCount());
		}
		return member;
}

