#ifdef WRITEING__sdfawef
#define WRITEING__sdfawef
#include "login.h"
#include "helper/utf8to.h"
#include <common/res_code.h>
#include <common/core/request.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <common/version.h>
#include <location/default.h>
#include<algorithm>
#include "WinPingMgr.h"

#include "client/proto/iconfig.h"
#include "client/proto/proto_obj_name.h"
#include "ProtoLink.h"
//#include "../core/sox/soxhelper.h"
#include "helper/ufile.h"
#include "helper/utf8to.h"
#include "helper/helper.h"
#include "report.h"



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
END_REQUEST_MAP()


static const char rnd_seed[] = "Our product aim to make game player happy !!!";


#define USING_SALT_PWD_V1

#ifdef USING_SALT_PWD_V1
static const wchar_t salt_client[] = _T("-rex-client-salt-!@#$%^");
#endif



using namespace xproto;
using namespace protocol::login;
using namespace protocol::uinfo;

LinkStatus res2Link(int res){
	switch(res){
		case RES_SUCCESS:
			return LOGIN_SUCCESS;
		case RES_EPERM:
			return PASSWD_ERROR;
		case RES_ENONEXIST:
			return USER_NONEXIST;
		case RES_EBLACKACC:
			return BLACK_ACCOUNT;
		case RES_EBLACKMAC:
			return BLACK_MACHINE;
		case RES_EOVERTIMES:
			return LOCK_OVERTIME;
		default:
			return SERVER_ERROR;
	}
}

const char* statuToString(int status){
	switch(status){
		case LINK_INIT:
			return "LINK_INIT";
		case LINK_LBS:
			return "LINK_LBS";
		case EXCHANGE_PASSWORD:
			return "EXCHANGE_PASSWORD";
		case LBS_ERROR:
			return "LBS_ERROR";
		case LINK_AUTH:
			return "LINK_AUTH";
		case LOGIN_SUCCESS:
			return "LOGIN_SUCCESS";
		case PASSWD_ERROR:
			return "PASSWD_ERROR";
		case SERVER_ERROR:
			return "SERVER_ERROR";
		case NET_BROKEN:
			return "NET_BROKEN";
		case TIMEOUT:
			return "TIMEOUT";
		case KICKOFF:
			return "KICKOFF";
		case LOGOUT:
			return "LOGOUT";
		case UNKNOWN:
			return "UNKNOWN";
		case PROTOCOL_OLD:
			return "PROTOCOL_OLD";
		case LINK_GETMAIL:
			return "LINK_GETMAIL";
		case NON_EMAIL:
			return "NON_EMAIL";
		case LBS_SHUTDOWN:


			return "LBS_SHUTDOWN";
		case RELOGIN_SUCCESS:
			return "retry login success";
		case LBS_NET_ERROR:
			return "lbs net error";
		case LINK_NET_ERROR:
			return "link net error";
		case LOGIN_TIMEOUT:
			return "login time out";
		case USER_NONEXIST:
			return "user dont exit or passwor error";
		default:
			return "unkown status";
	}
}


/************************************************************************/
/*                                                                      */
/************************************************************************/


LbsFetcher::LbsFetcher(){
	__dns_random = 0;
	lbs_timer_.init(this, &LbsFetcher::OnLbsTimeout);
}

LbsFetcher::~LbsFetcher()
{
}

void LbsFetcher::setLogin(Login* xx)
{
	__login = xx;
}

void LbsFetcher::onLbs(XCast<PRouteLinkDRes>& cmd, 
					   nproto::connection* c)
{
	__login->SetLoginTime(2);
	nsox::xlog(NsoxInfo, "anwser from coneter server, rescode:%d", cmd.resCode);

	if (status_ != 1) {
		if(!core::Request::ifSuccess(cmd.resCode)){
			if(cmd.resCode == RES_EVOLATILE){
				__login->notifyStatus(PROTOCOL_OLD, 0);
			}else if (cmd.resCode == RES_EPROT) {
				__login->notifyStatus(CLIENT_OLD, 0);
			} else if (cmd.resCode == RES_EOVERTIMES) {
				__login->notifyStatus(LINK_OVERTIME, 0);
			} else {
				__login->notifyStatus(LBS_ERROR, 0);
			}
			status_ = -1;

		}else{
			__login->doLbs(cmd.ip.c_str(), cmd.ports);
			status_ = 1;
			//__login->doLbs("", cmd.ports);
		}
		
	}
	c->close_conn();
	for(std::vector<nsox::nu_auto_ptr<CWsTcpSocket> >::iterator it = socket_list_.begin(); it != socket_list_.end(); ++it) {
		(*it)->close_conn();
	}
	lbs_timer_.stop();
}
void LbsFetcher::onCheckVersion(XCast<PCheckVersionRes>& cmd, 
								nproto::connection* c)
{

}

void LbsFetcher::onServerShutDown(XCast<PServerShutDown>& cmd, 
								  nproto::connection* c)
{
	nsox::xlog(NsoxInfo, "get messaget  from coneter server, msg:%s, rescode:%d",
		cmd.word, cmd.resCode);
	__lbs_msg = cmd.word;
	__login->notifyStatus(LBS_SHUTDOWN, 0);		
	c->close_conn();
	socket_list_.clear();
}

void LbsFetcher::cancel()
{
	if(__socket){
			__socket->close_conn();
	}
	__socket = NULL;
	socket_list_.clear();
	status_ = 0;
	lbs_timer_.stop();
}

void LbsFetcher::OnLbsTimeout() {
	if (__socket)
		__socket->close_conn();
	for(std::vector<nsox::nu_auto_ptr<CWsTcpSocket> >::iterator it = socket_list_.begin(); it != socket_list_.end(); ++it) {
		(*it)->close_conn();
	}
	if (status_ != 1) {
		// TODO
		__login->TryHistorys();
	}
	lbs_timer_.stop();
}

void LbsFetcher::start(const std::list<xstring>* ips, unsigned int port)
{
	cancel();
	status_ = 0;
	lbs_timer_.start(3000);
	
	//nsox::xlog(NsoxInfo, "start connect to center server, host:%s, port:%d", host, port);

	if(__socket){
		__socket = NULL;
	}
	socket_list_.clear();

	__login->notifyStatus(LINK_LBS);

	for(std::list<xstring>::const_iterator it = ips->begin(); it != ips->end(); ++ips) {
		nsox::nu_auto_ptr<CWsTcpSocket> socket = nsox::nu_create_object<CWsTcpSocket>::create_inst();
		socket->Create();
		socket->SetProtoHandler(this);
		socket->AsynConnect(app2net(*it).data(), port);
		socket_list_.push_back(socket);
		if (socket_list_.size() >= 4) {
			break;
		}
	}
}

void LbsFetcher::start(const char* host, unsigned int port)
{
	cancel();
	status_ = 0;
	lbs_timer_.start(3000);
	nsox::xlog(NsoxInfo, "start connect to center server, host:%s, port:%d", host, port);

	if(__socket){
		__socket = NULL;
	}
	socket_list_.clear();


	__login->notifyStatus(LINK_LBS);
	/*struct hostent *hptr = gethostbyname(host);
	if(!hptr){
		nsox::xlog(NsoxInfo, "can not reslove host name :%s", host);
		__login->notifyStatus(LBS_ERROR_DNS, 0);
		return;
	}
	int totalLbs = 0 ;
	while (hptr->h_addr_list[totalLbs++] != 0 ) {}
	char* ip  = inet_ntoa(*(struct in_addr *)(hptr->h_addr_list[(__dns_random++)%(totalLbs-1)]));
	*/
	//host = "75.126.52.35";

	__socket = nsox::nu_create_object<CWsTcpSocket>::create_inst();
	__socket->Create();
	__socket->SetProtoHandler(this);
	__socket->AsynConnect(host, port);
}

void LbsFetcher::oncreate(nproto::connection* c)
{
	__login->SetLoginTime(1);
	nsox::xlog(NsoxInfo, "success connected to center server");
	XCast<PRouteLinkD> req;
	req.proto_version = PROTOCOL_VERSION;
	req.client_version = 0xffffffff;//
	// req.client_version = __login->getClientVersion();
	write_request(c, req);
}

void LbsFetcher::onclose(int err, nproto::connection* c)
{
	nsox::xlog_err(NsoxError, err, "center server closed the connection");
	//__login->notifyStatus(LBS_NET_ERROR, 0);
}


/************************************************************************/
/*                                                                      */
/************************************************************************/

Login::Login(CProtoLink* xxx)
{
	__rsaKey = NULL;

	RAND_seed(rnd_seed, sizeof rnd_seed); 

	if(__rsaKey){
		RSA_free(__rsaKey);
		CRYPTO_cleanup_all_ex_data();
	}

	__rsaKey =  RSA_generate_key(512, 3, NULL, NULL);

	while(RSA_check_key(__rsaKey) != 1){
		RSA_free(__rsaKey);
		__rsaKey = RSA_generate_key(512, 3, NULL, NULL);
	}


	__lbs.setLogin(this);
	__lbsconfig = NULL;
	addr_config_ = NULL;
	__protoLink = xxx;
	__loginTimer.init(this, &Login::onLoginTimeout);
	__pingTimer.init(this, &Login::ping);
	__sessTimer.init(this, &Login::refreshSessKey);
	__relogWaitTimer.init(this, &Login::realRelog);
	//__rsaKey = NULL;
	__pingCount = 0;
	__relogFailTime = 0;
	__clientVersion = 0;
	__logTryTime = 0;
	__innerUid = 0;

	__protoVersion  =  (xstring)_T(__VERSION_REGION__) + _T("_") + (xstring)_T(__BUILD_MACH__) + _T("_");
	__protoVersion += (xstring)_T(__DATE__) + _T(" : ");
	__protoVersion += (xstring)_T(__TIME__);
	//__proto_version = __DATE__;
}

Login::~Login(void)
{
		if(__rsaKey){
			RSA_free(__rsaKey);
			CRYPTO_cleanup_all_ex_data();
		}
}

void Login::ping()
{

	if(__pingCount ++ >= 3){
			nsox::xlog(NsoxDebug, "linkd ping time out ,relogin");					
			reLogin();
	}else{
		nsox::xlog(NsoxDebug, "send heart beat command, ping count value:", __pingCount);
		XCast<PPlus> plus;
		plus.stampc = ::timeGetTime();
		if(__socket){ // send ping data
			write_request(__socket, plus);
		}
	}
}

int Login::GetTime(int index) {
	if (index > LOGIN_TIME)
		return 0;
	if (times_[index] > times_[0]) {
		return times_[index] - times_[0];
	}
	return 0;
}

void Login::notifyStatus(LinkStatus stat, int err)
{
	if (stat != LINK_INIT && stat != LINK_LBS && stat !=  EXCHANGE_PASSWORD && stat != LINK_AUTH && stat != LOGOUT) {
		ReportManager::GetInst()->Report(1, 0, "f=login;dns=%d,lbs=%d;lc=%d;lk1=%d;lk2=%d;lk3=%d;lr=%d;stat=%d;acc=%S", 
			GetTime(1), GetTime(2) , GetTime(3), 
			GetTime(4), GetTime(5), GetTime(6), GetTime(7), stat, __innerAccount.data());
	}
		__stat = stat;
		nsox::xlog(NsoxInfo, "link status is: %s, err :%d", statuToString(stat), err);
		switch(stat) {
		case LBS_NET_ERROR:
		case LINK_NET_ERROR:
		case NET_BROKEN:
			if (__logSuccTime != 0 && __relogFailTime < MAX_RETIME) {
				 reLogin();
			} else {
				__protoLink->onLoginStatus(stat);
				cancel();
			}
			break;
		case PASSWD_ERROR:
		case SERVER_ERROR:
		case KICKOFF:
		case LOGOUT:
		case PROTOCOL_OLD:
		case CLIENT_OLD:
		case USER_NONEXIST:
		case LINK_KEY_ERROR:
			cancel();
		default:
			if (__relogFailTime == 0) {
					__protoLink->onLoginStatus(stat);
			} else if (stat == LOGIN_SUCCESS){
					__protoLink->onLoginStatus(RELOGIN_SUCCESS);
					__relogFailTime = 0;
			}
			break;
		}

}

LinkStatus Login::getStatus()
{
		return __stat;
}

void Login::cancel()
{		
		__pingCount = 0;

		nsox::xlog(NsoxInfo, "try to cancel login process");
		if(__stat != LOGIN_SUCCESS)
		{
				__lbs.cancel();
				if(__socket){
					__socket->close_conn();
					__socket = NULL;
				}		
		}
		__loginTimer.stop();
		__stat = LINK_INIT;				
}




void Login::doLogin(const xstring& account, const xstring& pass, BOOL encrypt)
{		
		__logSuccTime = 0;
		nsox::xlog(NsoxInfo, "-------------------  login  --------------");

		if(__stat != LOGIN_SUCCESS)
		{
				__innerUid = 0;	
				__innerAccount = account;
				__kind.clear();

				if(encrypt){
					__encryptPwd  = pass;
				}else{
					#ifdef USING_SALT_PWD_V1
						__oldPasswd = net2app(sha1sum(app2net(pass.c_str())));
						xstring saltPass = pass + salt_client;
						__encryptPwd =  net2app(sha1sum(app2net(saltPass.c_str())));
					#elif USING_SALT_PWD_V0
						__oldPasswd = net2app(sox::sha1sum(app2net(pass.c_str())));
						__encryptPwd  = net2app(sox::sha1sum(app2net(pass.c_str())));
					#endif
				}

				nsox::xlog(NsoxInfo, "start login as user:%d", __innerUid);
				reLogin(false);
		}		
}


void Login::doLogin(uint32_t uid, const xstring& pass ,BOOL encrypt)
{		
		__logSuccTime = 0;
		nsox::xlog(NsoxInfo, "-------------------  login  --------------");

		if(__stat != LOGIN_SUCCESS)
		{			
				__innerUid = uid;
				__innerAccount = _T("");
				__kind.clear();
				
				
				if(encrypt){
					__encryptPwd  = pass;
				}else{
					#ifdef USING_SALT_PWD_V1
						__oldPasswd = net2app(sha1sum(app2net(pass.c_str())));
						xstring saltPass = pass + salt_client;
						__encryptPwd =  net2app(sha1sum(app2net(saltPass.c_str())));
					#elif USING_SALT_PWD_V0
						__oldPasswd = net2app(sox::sha1sum(app2net(pass.c_str())));
						__encryptPwd  = net2app(sox::sha1sum(app2net(pass.c_str())));
					#endif
				}
				nsox::xlog(NsoxInfo, "start login as user:%d", __innerUid);
				reLogin(false);
		}		
}

void Login::doLoginByToken(uint32_t uid, const xstring& token,const xstring& kind) {
	assert(!kind.empty());
	__logSuccTime = 0;
	nsox::xlog(NsoxInfo, "-------------------  login  --------------");

	if(__stat != LOGIN_SUCCESS)
	{			
		__innerUid = uid;
		__innerAccount = _T("");
		__kind  = kind;
		__token = token;
		nsox::xlog(NsoxInfo, "start login as user:%d", __innerUid);
		reLogin(false);
	}		
}

void Login::doLoginByToken(const xstring& account, const xstring& token,const xstring& kind) {
	assert(!kind.empty());
	__logSuccTime = 0;
	nsox::xlog(NsoxInfo, "-------------------  login  --------------");

	if(__stat != LOGIN_SUCCESS)
	{			
		__innerUid = 0;
		__innerAccount = account;
		__kind  = kind;
		__token = token;
		nsox::xlog(NsoxInfo, "start login as user:%d", __innerUid);
		reLogin(false);
	}		
}

void Login::doLogout()
{
		nsox::xlog(NsoxInfo, "-------------------  logout  --------------");
		cancel();
		if(__socket){
				__socket->close_conn();
				__socket = NULL;
		}
		__relogFailTime = 0;
		__pingTimer.stop();
		notifyStatus(LOGOUT);
}

uint32_t Login::getUid()
{
		return __innerUid;
}

xstring Login::getAccount()
{
		return __innerAccount;
}

xstring Login::getPasswd()
{
		return __encryptPwd;
}

void Login::SetLoginTime(int index) {
	if (index < LOGIN_TIME && index > 0) {
		times_[index] = ::timeGetTime();
	}
}

void Login::reLogin(bool retry)
{
		cancel();		//clear all vars

		for(int i = 0; i<LOGIN_TIME; ++i) {
			times_[i] = 0;
		}
		times_[0] = ::timeGetTime();

		__linkIp = "";	
		__linkPorts.clear();
		UINT time = __relogFailTime;

		if (retry == false) {
			__relogFailTime = 0;
			realLogin();
			return;
		}  else {
			__relogFailTime ++;
		}
		time = 2 * time;
		if (time < MIN_RELOG) {
			time = MIN_RELOG;
		}
		
		time = time > MAX_RELOG ? MAX_RELOG : time;
		__relogWaitTimer.start(time * 1000);
		__pingTimer.stop();
		
}

void Login::doLbs(const char* ip, std::vector<uint16_t>& ports)
{
	//times_[1] = timeGetTime();
	
	xstring xx = getAppPath();
	xx += _T("\\rcEnv.ini");
	wchar_t host[1024];
	GetPrivateProfileStringW(L"xproto", L"link", L"", host, 1024-1, xx.c_str());
	std::string host_str = app2net(host);
	if (!host_str.empty()) {
		__linkIp = host_str;
	} else {
		__linkIp	= ip;
	}
	//__linkIp = "211.72.192.146";
	__linkPorts = ports;

	notifyStatus(LINK_AUTH);
	realLogin();
}

void Login::onclose(int err, nproto::connection* c)
{
	notifyStatus(LINK_NET_ERROR);
}

void Login::oncreate(nproto::connection* c)
{
	SetLoginTime(3);
	__loginTimer.stop();
	if(!__loginTimer.hasStart()){
			__loginTimer.start(LOGIN_TRY_TIME); //restart login timer
	}
	nsox::xlog(NsoxInfo, "success connected to im server");
	notifyStatus(EXCHANGE_PASSWORD);
	rsaEncode();
}

void Login::onLoginSuccess()
{
	__sessTimeCount = 0;
	__loginTimer.stop();
	__pingTimer.start(LINK_PING_TIME);
	__sessTimer.start(SESS_REFRESH_TIME);

	notifyStatus(LOGIN_SUCCESS,0);
	__logSuccTime++;

}

void Login::onLoginError(LinkStatus ss)
{
	nsox::xlog(NsoxInfo, "login logic error, status:%s", statuToString(ss));

	__loginTimer.stop();

	notifyStatus(ss, 0);
}

void Login::onLoginTimeout()
{
	nsox::xlog(NsoxInfo, "login timeout");

	if(__stat == LOGIN_SUCCESS){
			__loginTimer.stop();
	}else {
			cancel();
			if (__relogFailTime == 0) {
					notifyStatus(LOGIN_TIMEOUT);
			} else {
					notifyStatus(NET_BROKEN);
			}
	}	
}


void Login::realLogin()
{
	if(__stat == EXCHANGE_PASSWORD || __stat == LOGIN_SUCCESS) { // if success or login , return;
		nsox::xlog(NsoxInfo, "loing in process");
		return;
	}
	if(__linkIp.empty()) {
		//login_report_str("cmd=login;id=%d", 0);
		if (__lbsconfig) {
			if (__lbsconfig->lbsServerIpAddrs() && __lbsconfig->lbsServerIpAddrs()->size()) {
				__lbs.start(__lbsconfig->lbsServerIpAddrs(), __lbsconfig->lbsServerPort());
			} else {
				__lbs.start(app2net(__lbsconfig->lbsServerAddr()).data(), __lbsconfig->lbsServerPort()); // connect to center server and get link ip
			}
		} else {
			assert(0);  // must set __lbsconfig first
		}
	}else if (!__linkPorts.empty()){
		//�����S�C�˿�֧��
		static int __link_port_rnd=  0;
		int idx = (++__link_port_rnd) % __linkPorts.size();
		uint16_t port = __linkPorts[idx];

		nsox::xlog(NsoxInfo, "start connect to im server, ip:%s, port : %d", __linkIp.c_str(), port);

		__socket = nsox::nu_create_object<CEncryTcpSocket>::create_inst();
		__socket->Create();
		__socket->SetProtoHandler(__protoLink);
		__socket->AsynConnect(__linkIp.c_str(), port);
	} else {
		notifyStatus(LBS_NET_ERROR);
	}

}

void Login::onLogin(XCast<PLoginRes>& cmd, nproto::connection* c)
{
	SetLoginTime(7);
	if(!core::Request::ifSuccess(cmd.resCode)){
		nsox::xlog(NsoxInfo, "server anwser login with error rescode : %d", cmd.resCode);
		notifyStatus((LinkStatus)UNKNOWN);
	}else{
		nsox::xlog(NsoxInfo, "server anwser login with rescode : %d", cmd.res);

		LinkStatus ss = res2Link(cmd.res);
		switch(ss){
			case LOGIN_SUCCESS:
				__innerUid = cmd.uid;
				__innerAccount = net2app(cmd.account);
				onLoginSuccess();
			
				break;
			case SERVER_ERROR:				
			default:
				onLoginError(ss);
				break;
		}

		nsox::nu_auto_ptr<xproto::IConfigManager> config_manager = (xproto::IConfigManager*)xproto::CoCreateProto()->queryObject(CONFIG_MANAGER_OBJ);
		if (config_manager) {
			IConfig* conf = config_manager->getConfig(_T("link-historys"));
			if (conf) {
				std::vector<xstring> iplist;
				conf->getList(0, iplist);

				// �����ͬ�Ľ��
				std::vector<xstring>::iterator ip_old_it = find(iplist.begin(),iplist.end(), net2app(__linkIp));
				if (ip_old_it != iplist.end()) {
					iplist.erase(ip_old_it);
				}

				// ��������Ľ��
				if (iplist.size() > 10) {
					iplist.erase(iplist.begin());
				}
				
				// �ӵ���ǰ��
				iplist.push_back(net2app(__linkIp));
				conf->setList(0, iplist);

				// ���¶˿�
				if (__linkPorts.size()) {
					int idx = (__link_port_rnd) % __linkPorts.size();
					uint16_t port = __linkPorts[idx];
					conf->setInt(1, port);
				}
				conf->saveConfig();
			}
		}
	}
}

void Login::onKick(XCast<PKickOff>& cmd, nproto::connection* c)
{
		nsox::xlog(NsoxInfo, "kicked by server");
		doLogout();
		notifyStatus(KICKOFF);
		
}

void Login::onExchangeKey(XCast<PExchangeKeyRes>& exKey, nproto::connection* c)
{
	SetLoginTime(6);
	nsox::xlog(NsoxInfo, "make handler success");
	if(core::Request::ifSuccess(exKey.resCode)){
		std::string rc4key;

		rsaDecodeRc4(exKey.encSessionKey, rc4key);
		__socket->setEncKey((const unsigned char *)rc4key.data(), DEF_SESSIONKEY_LENGTH);
		if (__innerUid != 0) {
			if (__kind.empty()) {
				 #ifdef USING_SALT_PWD_V1
					XCast<PLogin3ByUid> nl;
					nl.anonymous = false;
					nl.uid		= __innerUid;
					nl.vid		= app2net(__vid);
					nl.sha1Pass = app2net(__oldPasswd);
					nl.newPasswd = app2net(__encryptPwd);
					nl.client_info = app2net(__protoVersion);
					write_request(c,nl);
				#else
					XCast<PLogin2ByUid> nl;
					nl.anonymous = false;
					nl.uid		= __innerUid;
					nl.vid		= app2net(__vid);
					nl.sha1Pass = app2net(__encryptPwd);
					nl.client_info = app2net(__protoVersion);
					write_request(c,nl);
				#endif
			} else {
				XCast<PLoginByToken> nl;
				nl.anonymous = false;
				nl.uid		= __innerUid;
				nl.vid		= app2net(__vid);
				nl.token = app2net(__token);
				nl.kind = app2net(__kind);
				nl.client_info = app2net(__protoVersion);
				write_request(c,nl);
			}
		} else if (!__innerAccount.empty()) {
			if (__kind.empty()) {
				 #ifdef USING_SALT_PWD_V1
					XCast<PLogin3ByAccount> nl;
					nl.anonymous = false;
					nl.account		= app2net(__innerAccount);
					nl.vid		= app2net(__vid);
					nl.sha1Pass = app2net(__oldPasswd);
					nl.newPasswd = app2net(__encryptPwd);
					nl.client_info = app2net(__protoVersion); 
					write_request(c,nl);
				#else
					XCast<PLogin2ByAccount> nl;
					nl.anonymous = false;
					nl.account		= app2net(__innerAccount);
					nl.vid		= app2net(__vid);
					nl.sha1Pass = app2net(__encryptPwd);
					nl.client_info = app2net(__protoVersion); 
					write_request(c,nl);
				#endif
			} else {
				XCast<PLoginByToken> nl;
				nl.anonymous = false;
				nl.uid		= __innerUid;
				nl.account	= app2net(__innerAccount);
				nl.vid		= app2net(__vid);
				nl.token = app2net(__token);
				nl.kind = app2net(__kind);
				nl.client_info = app2net(__protoVersion);
				write_request(c,nl);
			}
		}
	}else{
		notifyStatus(LINK_KEY_ERROR, 0);
	}

}
void Login::onPing(XCast<PPlus>& cmd, nproto::connection* c)
{
	nsox::xlog(NsoxInfo, "pplus come back, set pingCount as zero");
	__pingCount = 0;
	ping_ = ::timeGetTime() - cmd.stampc;
}
void Login::onServerChange(XCast<PServerChange>& cmd, nproto::connection* c)
{
}

void Login::onSessionNotice(XCast<protocol::session::PNotice>& cmd , nproto::connection* c)
{
	if (cmd.type == 100) {
	} else if (cmd.type == 101) {
	} else if (cmd.type == 102) {
	}
	__protoLink->proxyNotice(cmd.type, cmd.resources, net2app(cmd.info));

}

void Login::rsaEncode(){
	SetLoginTime(4);
	XCast<PExchangeKey> exk;

	/*
	RAND_seed(rnd_seed, sizeof rnd_seed); 

	if(__rsaKey){
		RSA_free(__rsaKey);
		CRYPTO_cleanup_all_ex_data();
	}

	__rsaKey =  RSA_generate_key(512, 3, NULL, NULL);

	while(RSA_check_key(__rsaKey) != 1){
		RSA_free(__rsaKey);
		__rsaKey = RSA_generate_key(512, 3, NULL, NULL);
	}
	*/

	getRSAKeyString(exk.publicKey, exk.e);

	if(__socket)
	{
			write_request(__socket, exk);
	}
	SetLoginTime(5);
}

void Login::getRSAKeyString(std::string &pub, std::string &e){
	unsigned char keybuf[1024];

	int size = BN_bn2bin(__rsaKey->n, keybuf);
	pub = std::string((char *)keybuf, size);

	size = BN_bn2bin(__rsaKey->e, keybuf);
	e = std::string((char *)keybuf, size);
}

void Login::rsaDecodeRc4(const std::string &ctext, std::string &ptext){
	unsigned char rc4key[100];

	int num = RSA_private_decrypt(ctext.length(), (const unsigned char *)ctext.data(), rc4key, __rsaKey,
		RSA_PKCS1_PADDING);

	if (num != DEF_SESSIONKEY_LENGTH)
	{
		assert(false);
		ptext = "";
		return;
	}
	rc4key[16] = 0;

	ptext = std::string((const char *)rc4key, DEF_SESSIONKEY_LENGTH);
}

void Login::writeRequest(nproto::request* req, int uri)
{
	if(__socket){
			write_request(__socket, req, uri);
	}
}


void Login::realRelog()
{
		if(!__loginTimer.hasStart()){
				__loginTimer.start(LOGIN_TRY_TIME); //start login timer
		}
		realLogin();
		__relogWaitTimer.stop();
}

void Login::onImResponse(XCast<PZImResponse> &cmd, nproto::connection *c) {
    PZImResponse response = cmd;
    __protoLink->onImResponse(response);
}




void Login::onSessKey (XCast<PSessKey>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		__sessKey = net2app(cmd.sesskey);
		__protoLink->onUpdateSessKey(__sessKey);
	}
}


void Login::onGetTokenRes (XCast<PGetTokenRes>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode) /*&& this->__kind.size()*/){
		if (cmd.type == 2 && cmd.kind == "link") {
			__loginSessToken = net2app(cmd.token);
		}
	}
}

void Login::onServerPing (XCast<PServerPing>& cmd, nproto::connection* c)
{
	if(core::Request::ifSuccess(cmd.resCode)){
		XCast<PServerPingRes> req;
		req.haomiao = cmd.haomiao;
		write_request(__socket, req);
	}
}

xstring  Login::getSessKey()
{
	return __sessKey;
}


void  Login::refreshSessKey()
{
	__sessTimeCount++;
	if(__sessTimeCount % 5 == 0 && __socket){ 
		nsox::xlog(NsoxInfo, "refresh sesskey timeout");
		XCast<PSessKey> req;
		write_request(__socket, req);
	}
	if(__sessTimeCount % 50 == 0 && __socket){ 
		nsox::xlog(NsoxInfo, "refresh token timeout");
		XCast<PGetToken> req;
		req.type = 2;
		req.kind = "link";
		write_request(__socket, req);


	}
}


void Login::setMachineCode(const xstring& mc)
{
		__vid = mc;
}

xstring	Login::getMachineCode() const
{
		return __vid;
}

void Login::setClientVersion(uint32_t version)
{
		__clientVersion = version;
}

uint32_t Login::getClientVersion() const
{
		return __clientVersion;
}


void Login::setLbsConfig(ILbsConfig* config)
{
		__lbsconfig = config;
}

xstring  Login::getLoginSessionToken() {
	return __loginSessToken;
}



void Login::TryHistorys() {
	nsox::nu_auto_ptr<xproto::IConfigManager> config_manager = (xproto::IConfigManager*)xproto::CoCreateProto()->queryObject(CONFIG_MANAGER_OBJ);
	if (config_manager) {
		IConfig* conf = config_manager->getConfig(_T("link-historys"));
		if (conf) {
			std::vector<xstring> iplist;
			conf->getList(0, iplist);
			if (iplist.size()) {
				for(int i = 0; i < iplist.size(); ++i) {
					PingServerManager::inst()->BeginPing(app2net(iplist[i]).data(), this, 5000);
					//PingServerManager::inst()->BeginPing("8.8.8.8", this, 5000);
					//break;
				}
			}
		}
	
	}
}

void Login::OnPingCallBack(const CWinPingTask& task) {
	if (__linkIp.empty() && task.ping_ != -1) {
		nsox::nu_auto_ptr<xproto::IConfigManager> config_manager = (xproto::IConfigManager*)xproto::CoCreateProto()->queryObject(CONFIG_MANAGER_OBJ);
		if (config_manager) {
			IConfig* conf = config_manager->getConfig(_T("link-historys"));
			if (conf) {
				uint16_t port = conf->getInt(1, 81);
				std::vector<uint16_t> ports;
				ports.push_back(port);
				doLbs(task.host_, ports);
			}
		}
	}
}

void Login::SetAddrConfig(xproto::IAddrConfig* config) {
	addr_config_ = config;
}

xproto::IAddrConfig* Login::GetAddrConfig() {
	return addr_config_;
}


#endif