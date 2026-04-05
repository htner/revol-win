#pragma once

#include "proto/iproto.h"
#include "proto/islist.h"
#include "common/nsox/default_proto_handler.h"
#include "common/nsox/nuautoptr.h"
#include "common/nsox/proto_cast.h"
//#include "protocol/pslist.h"
#include "protocol/plink.h"
#include "protocol/psession.h"
#include "timer/TimerWrap.h"
#include "proto/proto_obj_name.h"

#include "proxyManager.h"

#include "sitem.h"

using namespace protocol::link;

namespace xproto
{
		class CProtoLink;
}
namespace protocol{
	namespace session{
			class SessionListInfo;
	}
	namespace slist{

	class SessionList : public nproto::default_proto_handler	
					  , public ISessionList
					  , public XConnPoint<xproto::IListEvent>
					  , public XConnPoint<xproto::IActionEvent>
					  , public XConnPoint<ISessListEvent>
	{
	public:
		DECLARE_PROTO_OBJECT_NAME(SESSION_LIST_OBJ);

		DECLARE_REQUEST_MAP


	public:

		void	bindLike();
		void	unbindLike();
		virtual void	updateSinfoLikes(const std::vector<uint32_t>& sids);
		void	updateSinfo(uint32_t sid,  const SItem &s);
		void	updateMyList(const SID_T& sid, bool badd);

		void	setProtoLink(xproto::CProtoLink* link);

		virtual void onclose(int err,nproto::connection* c){};
		virtual void oncreate(nproto::connection* c){};

		SessionList(void);
		virtual ~SessionList(void);


		virtual UID_T getOwner(const HULI &fid) const;
		virtual void addFolder(const HULI &pid, const xstring &fdNick);
		virtual void delFolder(const HULI &fid, const HULI &pid);
		virtual void addFavor(const HULI &sid, const HULI &pid);
		virtual void removeFavor(const HULI &fid, const HULI &pid);
		
		virtual void sync();		
		virtual void syncSub(const HULI &pid);
		//virtual void checkSub(const HULI $pid, const HULI& sid);
		virtual int isFond(const HULI &sid);   // -1等待更新, 0 unFond, 1 Fond
		virtual void setFond(const HULI &sid, bool bFond);
		virtual void updateFonds();


		virtual uint32_t getOnline(const SID_T &fid) const;				//獲取總人數
		virtual xstring  getIntro(const HULI& cid) const;
		virtual bool isLimitChannel(const SID_T &cid) const;
		virtual bool IsExistSess(const HULI &sid) const;
		virtual session::ISessionInfoBase *getSessionInfo(const SID_T &sid, bool update_force = false);

		virtual xstring  getLables(const HULI& hid) const;
		virtual xstring  getSlogan(const HULI& hid) const;

		virtual int  getCollect(const HULI &hid) const;
		virtual int  getMember(const HULI &hid) const;
		


		//??????????????????????????????????????????????????????????

		virtual bool isPublic(const HULI& hid )const;
		virtual LPCWSTR getNick(const HULI &hid) const;		
		virtual void getParent(const HULI &hid, std::vector<HULI>& parents) const;
		virtual HULI getAliaseId(const HULI &hid) const;
		virtual PlainTree_t getTreeData() const;
		virtual void fillTreeData(IListFiller*,int) const;


		virtual bool isExisit(const HULI& hid, const HULI& pid) const;
		virtual void syncInfo();
		virtual bool isFolder(const HULI &id) const;


	public:
		void onAddSession(XCast<PAddSessRes>& cmd,			nproto::connection* c);
	
	protected:
		void onSync(XCast<PSyncSListRes>& cmd,				nproto::connection* c);
		void onRemoveSession(XCast<PRemoveSessRes>& cmd,	nproto::connection* c);
		void onRemoveFolder(XCast<PRemoveFolderRes>& cmd,	nproto::connection* c);
		void onAddFolder(XCast<PAddFolderRes>& cmd,			nproto::connection* c);
		
		void onSyncInfo(XCast<session::PSessionSyncInfoGet>& cmd,	nproto::connection* c);
		void onSyncSInfo(XCast<PSyncSInfoRes>& cmd,		nproto::connection* c);

		void onSubSync(XCast<PSyncSubListVecRes>& cmd,				nproto::connection* c);
		void onGetFondRes(XCast<PGetFondRes>& cmd,				nproto::connection* c);
		void onUpdateFondRes(XCast<PUpdateFondRes>& cmd,	 nproto::connection* c);
		
		void ClearHot();
		void OnUpdateSessionLikes(const xstring& method, uint32_t resCode, const xstring& res, uint32_t idd);
		
		template<class T>
		void sendRequest(T& xx){
			__protoLink->writeRequest(&xx, T::uri);
		}
	protected:
		
		typedef std::map<SID_T, SItem> tree_doc_t;
		std::set<uint32_t>		beGetFonds;
		std::list<uint32_t>		last_access_order_;
		tree_doc_t __tree;
		TimerHandler<SessionList>  __refreshTimer;

		typedef nsox::nu_auto_ptr<session::SessionListInfo> SessInfoPtr;
		typedef std::map<SID_T, SessInfoPtr> sid2info_map_t;
		sid2info_map_t infoMap;

		xproto::CProtoLink* __protoLink;

		xproto::ProxyHanler<SessionList>	session_likes_proxyer_;
		std::map<uint32_t, uint32_t> session_last_sync_;
	};
}
}

