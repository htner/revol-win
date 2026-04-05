#pragma once
#include "xconnpoint.h"
#include <common/nsox/default_proto_handler.h>
#include <common/nsox/nuautoptr.h>
#include <common/nsox/proto_cast.h>
#include <protocol/const.h>
#include <protocol/plogin.h>
#include <protocol/pim.h>
#include "timer/ExtraTimer.h"
#include "timer/TimerWrap.h"
#include "common/xstring.h"
#include "protocol/psession.h"

#include "WsTcpSocket.h"
#include "client/proto/iproto.h"
#include "WinPingMgr.h"
#include "third/openssl/rsa.h"
//#include "Gateway.h"



#define LINK_PING_TIME	20000
#define LOGIN_TRY_TIME  20000
#define LOGIN_TIME 20

using namespace protocol::login;
using namespace protocol::im;

namespace xproto
{
	class Login;
	class LbsFetcher : public nproto::default_proto_handler
	{
	public:
		LbsFetcher();
		virtual ~LbsFetcher();

		BEGIN_REQUEST_MAP(LbsFetcher)		
			REQUEST_HANDLER(XCast<PRouteLinkDRes>, onLbs)
			REQUEST_HANDLER(XCast<PCheckVersionRes>, onCheckVersion)
			REQUEST_HANDLER(XCast<PServerShutDown>, onServerShutDown)
			
		END_REQUEST_MAP()

	public:
		void setLogin(Login* xx);
		void start(const char* ip, unsigned int port);
		void start(const std::list<xstring>* ips, unsigned int port);
		void cancel();

		void OnLbsTimeout();

	protected:
		void onclose(int err, nproto::connection* c);		
		void oncreate(nproto::connection* c);

	protected:
		void onLbs(XCast<PRouteLinkDRes>& cmd, nproto::connection* c);
		void onCheckVersion(XCast<PCheckVersionRes>& cmd, nproto::connection* c);
		void onServerShutDown(XCast<PServerShutDown>& cmd, nproto::connection* c);
		
	protected:
		int									status_;
		nsox::nu_auto_ptr<CWsTcpSocket> __socket;
		std::vector<nsox::nu_auto_ptr<CWsTcpSocket> > socket_list_;
		std::string						__lbs_msg;
		Login*							__login;
		uint32_t						__dns_random;
		TimerHandler<LbsFetcher>		lbs_timer_;			//LBS定时器
		
	};


	class CProtoLink;

	class Login : public nproto::default_proto_handler , public CWinPingCallBack
	{
		friend class LbsFetcher;
	public:
		Login(CProtoLink*);
		virtual ~Login(void);
		BEGIN_REQUEST_MAP(Login)		
			REQUEST_HANDLER(XCast<PLoginRes>, onLogin)
			REQUEST_HANDLER(XCast<PKickOff>, onKick)
			REQUEST_HANDLER(XCast<PExchangeKeyRes>, onExchangeKey)
			REQUEST_HANDLER(XCast<PPlus>, onPing)
			REQUEST_HANDLER(XCast<PServerChange>, onServerChange)
			REQUEST_HANDLER(XCast<protocol::session::PNotice>, onSessionNotice)
			REQUEST_HANDLER(XCast<PZImResponse>, onImResponse)
			REQUEST_HANDLER(XCast<PSessKey>, onSessKey)
			REQUEST_HANDLER(XCast<PGetTokenRes>, onGetTokenRes)
			REQUEST_HANDLER(XCast<PServerPing>, onServerPing)
			//REQUEST_HANDLER(XCast<PRaidToken>, onRaidToken)

		END_REQUEST_MAP()


		void onclose(int err, nproto::connection* c);		
		void oncreate(nproto::connection* c);

		void doLogin(const xstring& account, const xstring& pass, BOOL encrypt);
		void doLogin(uint32_t uid, const xstring& pass,BOOL encrypt);
		void doLoginByToken(const xstring& account, const xstring& token,const xstring& kind);
		void doLoginByToken(uint32_t uid, const xstring& token,const xstring& kind);
		void doLogout();

		void cancel();
		void writeRequest(nproto::request* req, int uri);

		UID_T getUid();

		xstring getPasswd();
		ACCOUNT_T getAccount();
		xstring getSessKey();

		LinkStatus getStatus();

		void			setMachineCode(const xstring&);
		xstring			getMachineCode() const;
		void			setClientVersion(uint32_t version);
		uint32_t		getClientVersion() const;
		void			setLbsConfig(xproto::ILbsConfig* config);
		void			SetAddrConfig(xproto::IAddrConfig* config);
		xproto::IAddrConfig* GetAddrConfig();


		xstring			getLoginSessionToken();
		void			TryHistorys();
		virtual void OnPingCallBack(const CWinPingTask& task);
		void SetLoginTime(int index);
		int GetTime(int index);



	protected:
		void notifyStatus(LinkStatus stat, int err=0);
		void doLbs(const char* ip, std::vector<uint16_t>& ports);
		void realLogin();
		void reLogin(bool retry = true);



		void onLoginSuccess();
		void onLoginError(LinkStatus ss);
		void onLoginTimeout();

		void ping();

		void realRelog();

		void rsaEncode();
		void rsaDecodeRc4(const std::string &ctext, std::string &ptext);
		void getRSAKeyString(std::string &pub, std::string &e);
		void  refreshSessKey();
	protected:
		void onLogin(XCast<PLoginRes>& cmd, nproto::connection* c);
		void onKick(XCast<PKickOff>& cmd, nproto::connection* c);
		void onExchangeKey(XCast<PExchangeKeyRes>& cmd, nproto::connection* c);
		void onPing(XCast<PPlus>& cmd, nproto::connection* c);
		void onServerChange(XCast<PServerChange>& cmd, nproto::connection* c);
		void onSessionNotice(XCast<protocol::session::PNotice>& cmd , nproto::connection* c);
		void onImResponse(XCast<PZImResponse> &cmd, nproto::connection *c);
		void onSessKey (XCast<PSessKey>& cmd, nproto::connection* c);
		void onGetTokenRes (XCast<PGetTokenRes>& cmd, nproto::connection* c);
		void onServerPing (XCast<PServerPing>& cmd, nproto::connection* c);


		
	private:

		int						__sessTimeCount;
		int						__link_port_rnd;
		int						__pingCount;
		int						__relogFailTime;
		int						__logSuccTime;
		int						__logTryTime;
		int						ping_;

		uint32_t				__clientVersion;
		xstring					__protoVersion;
		xstring					__sessKey;
		xstring					__loginSessToken;
		xstring					__oldPasswd;
		xstring					__encryptPwd;
		std::string				__linkIp;
		std::string				login_report_str_;

		xstring					__kind;
		xstring					__token;
		LinkStatus					__stat;
		std::vector<uint16_t>			__linkPorts;
		UID_T					__innerUid;
		ACCOUNT_T				__innerAccount;
		LbsFetcher					__lbs;
		//GatewayLink					gateway_link_;
		CProtoLink*				__protoLink;
		RSA	*					__rsaKey;
		xstring					__vid;
		xproto::ILbsConfig*			__lbsconfig;
		xproto::IAddrConfig*		addr_config_;
		uint32_t					times_[LOGIN_TIME];

		TimerHandler<Login>			__loginTimer;			//登陆定时器
		TimerHandler<Login>			__pingTimer;			//心跳定时器
		TimerHandler<Login>			__relogWaitTimer;		//等待重新登陆定时器
		TimerHandler<Login>			__sessTimer;			//等待重新登陆定时器

		nsox::nu_auto_ptr<CEncryTcpSocket>		__socket;
	};
}



