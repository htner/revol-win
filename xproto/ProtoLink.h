#pragma once

#include "proto/iproto.h"
#include "common/nsox/default_proto_handler.h"
#include "common/nsox/nuautoptr.h"
#include "ImBase.h"


namespace protocol
{
	namespace slist{
			class SessionList;
	}
	namespace session{
			class SessionManager;
	}
}
namespace xproto
{
	class CRaidProto;
	class Login;
	class CRaidUserManager;
	class CProxyManager;


	class CProtoLink : public nproto::default_proto_handler
					 , public IProtoLink  
					 , public XConnPoint<ILinkEvent>
					 , public XConnPoint<IProxyNoticeEvent>
					  
	{
	public:
		friend class CRaidProto;
		CProtoLink(void);
		virtual ~CProtoLink(void);

		DECLARE_REQUEST_MAP

		void onclose(int err, nproto::connection* c);		
		void oncreate(nproto::connection* c);


		int			login(const ACCOUNT_T &account, const xstring& pass, BOOL encrypt);
		int			login(const UID_T &uid, const xstring& pass, BOOL encrypt);
		int			loginByToken(const UID_T &uid, const xstring& token,const xstring& kind);
		int			loginByToken(const ACCOUNT_T& account, const xstring& token,const xstring& kind);

		void		logout();
		UID_T		getUid();
		ACCOUNT_T	getAccount();
		LinkStatus	getStatus();
		IUserInfo*  getUinfo();
		void		cancelLogin();
		LPCSTR		getObjectName();

		xstring		getPasswd();
		xstring		getEncryptPasswd();
		xstring		getSessKey();
		uint32_t	getVersion();
		xstring		getLoginSessionToken();

		virtual void			setMachineCode(const xstring&);
		virtual xstring			geytMachineCode() const;
		virtual void			setClientVersion(uint32_t version);
		virtual uint32_t		getClientVersion() const;
		virtual void			setLbsConfig(ILbsConfig*);
		virtual void			SetAddrConfig(IAddrConfig*);
		virtual xproto::IAddrConfig*	GetAddrConfig();

		IProtoObject* getUserMgr();
		IProtoObject* getSessList();
		IProtoObject* getImBase();
		IProtoObject* getProxyMgr();


		void		onLoginStatus(LinkStatus stat);
		void		writeRequest(nproto::request* req, int uri);
		void		proxyNotice(int type, int resources, const xstring& info);
		void		onImResponse(protocol::im::PZImResponse &imRes);
		void		reConnSession();
		void		onUpdateSessKey(const xstring& key) ;

		Login*  GetLogin();


protected:
		Login*		__login;
		nsox::nu_auto_ptr<CRaidUserManager>						__userMgr;
		nsox::nu_auto_ptr<protocol::slist::SessionList>			__sessionList;
		nsox::nu_auto_ptr<protocol::session::SessionManager>	__sessManager;
		nsox::nu_auto_ptr<xproto::CImBase>						__imBase;
		nsox::nu_auto_ptr<CProxyManager>						__proxyMgr;
	};

}