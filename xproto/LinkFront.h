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

#define LINK_PING_TIME	20000
#define LOGIN_TRY_TIME  20000
#define LOGIN_TIME 20


typedef struct rsa_st RSA;

namespace xproto
{

class LinkLogin : public nproto::default_proto_handler {
public:
	LinkLogin();
	virtual ~LinkLogin();

	DECLARE_REQUEST_MAP

	void GetLinkFrontTokenByUid(const xstring& account, const xstring& pass, BOOL encrypt);
	void GetLinkFrontTokenByAccount(uint32_t uid, const xstring& pass, BOOL encrypt);
	void GetLinkFrontTokenByAccount(uint32_t uid, const xstring& pass, BOOL encrypt);
	void DoLogout();
	
	void SetMachineCode(const xstring&);
	void SetClientVersion(uint32_t version);
	void SetAddrConfig(xproto::IAddrConfig* config);
}

class LinkFront : public nproto::default_proto_handler {
public:
	LinkFront();
	virtual ~LinkFront(void);

	DECLARE_REQUEST_MAP

	void onclose(int err, nproto::connection* c);		
	void oncreate(nproto::connection* c);

	void WriteRequest(nproto::request* req, int uri);

	UID_T getUid();
	ACCOUNT_T getAccount();
	xstring getSessKey();

	LinkStatus getStatus();
	xstring	getLoginSessionToken();


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



