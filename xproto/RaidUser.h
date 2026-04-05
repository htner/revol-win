#pragma once

#include "proto/proto_obj_name.h"
#include "common/nsox/default_proto_handler.h"
#include "proto/iproto.h"
#include "common/nsox/proto_cast.h"
#include "common/nsox/nuautoptr.h"
//#include "protocol/puinfo.h"
#include "protocol/plink.h"
#include "protocol/psession.h"
#include "common/iproperty.h"
#include "infobase.h"
#include <map>

using namespace protocol::link;
using namespace protocol::uinfo;
using namespace protocol::session;

namespace xproto
{
	class CProtoLink;
	class CRaidUserManager;



	class CRaidUser : public IUserInfo
					, public XConnPoint<IUserEvent>
					, public XConnPoint<IGetUidEvent>
					, public InfoBase
	{
			friend class CRaidUserManager;
	public:
			
			CRaidUser();
			virtual ~CRaidUser(){}

			virtual UID_T		getUid()		const;
			virtual ACCOUNT_T	getAccount() const	;
			virtual xstring		getNick()	const	;
			virtual void	setNick(xstring&)	;
			virtual void	setPasswd(xstring&)	;
			virtual SEX		getSex() const;
			virtual void	setSex(SEX);
			virtual long	getBirthday() const ;
			virtual void	setBirthday(long)	;
			virtual int		getArea() const		;
			virtual void	setArea(int)		;
			virtual int		getProvince() const ;
			virtual void	setProvince(int)	;
			virtual int		getCity() const		;
			virtual void	setCity(int)		;
			virtual uint32_t getUserJF() const	;

			//Ö÷¶¯refresh
			virtual void	refresh()			;
			virtual void	update()			;
			virtual void	updateUid()			;
			virtual void	updateUserSess(uint32_t pid)	;
			virtual void	updateEffect()		;

			virtual xstring  getSign()			;
			virtual void	setSign(xstring&)	;
			virtual xstring	getIntro() const	;
			virtual void	setIntro(xstring&)	;

			virtual uint32_t	getUpdateTime()	; 

			// facebook
			virtual xstring  getFbId() const	;
			virtual xstring  getFbToken() const	;

			//money
			virtual int		getGold() const		;
			virtual int		getSilver() const		;
			virtual int		getFlowers() const		;
			virtual IUserInfo::PrivacyLevel  getPrivacy() const;
			virtual void	setPrivacy(IUserInfo::PrivacyLevel privacy);

			virtual bool  getVisitHistoryPrivacy() const;
			virtual void	setVisitHistoryPrivacy(bool);
			virtual bool  getCollectSessionPrivacy() const;
			virtual void	setCollectSessionPrivacy(bool);
		



			// user session
			virtual const userJoinSess& getUserSess() const;
			virtual const userEffects& getEffects() const;


			virtual const OpenPlatformInfoVector& getOpenPlatformInfo();
			virtual void setOpenPlatformInfo(OpenPlatformInfo);
			virtual void removeOpenPlatformInfo(OpenPlatformInfo);
			void AddPlatformInfo(xproto::OpenPlatformInfoVector);
	private:
			void updateProperty();
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	protected:
			void	onGetUInfo(XCast<PRepGetUInfo>& cmd, nproto::connection* c);
			void	onUpdated(XCast<PRepUpdateUInfo>& cmd, nproto::connection* c);
			void	onGetUid(XCast<PRepGetUid>& cmd, nproto::connection* c);
			void	onGetUserInSess(XCast<PRepGetUserInSessRes>& cmd, nproto::connection* c);
			void	onGetEffect(XCast<PGetEffectRes>& cmd, nproto::connection* c);

			void	setManager(CRaidUserManager* mgr);
	protected:
		UID_T						__uid;
		xstring						__account;
		//sox::Properties				__setTemp;
		CRaidUserManager*			__userMgr;
		uint32_t					__utime;
		uint32_t					__effecttime;
		userJoinSess				__userSess;
		userEffects					__userEffects;
		std::set<uint32_t>			__userGetSess;
		OpenPlatformInfoVector		__userPlatformvInfos;

	};

	class CRaidUserManager : public IUserManager
						   , public nproto::default_proto_handler
	{
		friend class CRaidUser;
	public:
		CRaidUserManager();
		virtual ~CRaidUserManager(void);

		DECLARE_PROTO_OBJECT_NAME(USER_MANAGER_OBJ);

		BEGIN_REQUEST_MAP(CRaidUser)
			REQUEST_HANDLER(XCast<PRepGetUInfo>, onGetUInfo)
			REQUEST_HANDLER(XCast<PRepUpdateUInfo>, onUpdated)
			REQUEST_HANDLER(XCast<PRepGetUid>, onGetUid)
			REQUEST_HANDLER(XCast<PRepGetUserInSessRes>, onGetUserInSess)
			REQUEST_HANDLER(XCast<PGetEffectRes>, onGetEffect)
			REQUEST_HANDLER(XCast<PGetEffectInfoRes>, onGetEffectInfo)
		END_REQUEST_MAP()
		void onclose(int err, nproto::connection* c){};		
		void oncreate(nproto::connection* c){};

		IUserInfo*  getUserInfo(const UID_T &uid);
		IUserInfo*  getUserInfo(const ACCOUNT_T &account);
		void setProtoLink(CProtoLink* link);
	protected:
		void writeRequest(nproto::request* req, int uri);
	protected:
		void onGetUInfo(XCast<PRepGetUInfo>& cmd, nproto::connection* c);
		void onUpdated(XCast<PRepUpdateUInfo>& cmd, nproto::connection* c);
		void onGetUid(XCast<PRepGetUid>& cmd, nproto::connection* c);
		void onGetUserInSess(XCast<PRepGetUserInSessRes>& cmd, nproto::connection* c);
		void onGetEffect(XCast<PGetEffectRes>& cmd, nproto::connection* c); 
		void onGetEffectInfo(XCast<PGetEffectInfoRes>& cmd, nproto::connection* c);
	protected:
		CProtoLink* __protoLink;
		typedef std::map<uint32_t, nsox::nu_auto_ptr<CRaidUser> > raid_usr_map;
		typedef std::map<xstring,  nsox::nu_auto_ptr<CRaidUser> > account_usr_map;
		typedef std::map<xstring, uint32_t> account2uid_map;

		raid_usr_map __users;
		account_usr_map __accounts;
		account2uid_map __trans;
	};

}
