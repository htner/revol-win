#include <common/nsox/default_proto_handler.h>
#include <common/nsox/nuautoptr.h>
#include <common/nsox/proto_cast.h>
#include <protocol/const.h>
//#include <protocol/preport.h>
#include "WsTcpSocket.h"
#include "WsUdpSocket.h"
#include "timer/TimerWrap.h"

//定义数据报文格式	
//"cmd=login;id=0;uid=0;dns=10;lsc=20;lsd=30;lkc=40;lke=50;lkr=70;res=200"  登陆
//"cmd=login;id=%d;uid=%d;dns=%d;lsc=%d;lsd=%d;lkc=%d;lke=%d;lkr=%d;res=%d"  登陆
//"cmd=joins;id=0;uid=0;sid=0;sm=20;sc=30;sek=40;sj=50;res=200"
//"cmd=voice;id=%d;uid=%d;

class ReportManager : public nproto::default_proto_handler
{
public:
	static ReportManager* GetInst();
	virtual ~ReportManager();

	BEGIN_REQUEST_MAP(ReportManager)		
//		REQUEST_HANDLER(XCast<protocol::preport::PReportDataRes>, OnReportDataRes)			
	END_REQUEST_MAP()

private:
	ReportManager();

public:
	void Report(int type, const std::string&);
	void Report(int type, const char*, size_t len);
	void Report(int type, int id,  const char* fmt, ...);

	void StartReportSocket();
	void OnReportTimeout();

	void Stop();

protected:
	void onclose(int err, nproto::connection* c);		
	void oncreate(nproto::connection* c);

protected:
	//void OnReportDataRes(XCast<protocol::preport::PReportDataRes>& cmd, nproto::connection* c);

protected:
	nsox::nu_auto_ptr<CWsTcpSocket> tcp_socket_;
	nsox::nu_auto_ptr<CWsUdpSocket>	udp_socket_;
	TimerHandler<ReportManager>		report_timer_;			//LBS定时器
};
