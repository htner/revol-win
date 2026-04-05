#include <common/core/request.h>
#include "client/proto/iconfig.h"
#include "client/proto/proto_obj_name.h"
#include "common/xstring.h"
#include "helper/ufile.h"
#include "helper/utf8to.h"
#include "report.h"

ReportManager* ReportManager::GetInst() {
	static ReportManager report_manager;
	return &report_manager;
}

ReportManager::~ReportManager() {
	Stop();
}

void ReportManager::Stop() {
	tcp_socket_ = NULL;
	udp_socket_ = NULL;
}
ReportManager::ReportManager() {
	report_timer_.init(this, &ReportManager::OnReportTimeout);
	report_timer_.start(2*60*1000);
}

void ReportManager::Report(int type, const std::string& data) {
	nsox::nu_auto_ptr<xproto::IProtoLink> spLink = (xproto::IProtoLink*)xproto::CoCreateProto()->queryObject(PROTO_LINK_OBJ);
	
}

void ReportManager::Report(int type, const char* data, size_t len) {
	std::string str(data, len);
	Report(type, str);
}

void ReportManager::Report(int type, int id,  const char* fmt, ...) {
	char info[2560];
	va_list ap;

    va_start(ap, fmt);
	int len = VSPRINTF(info, 2560 - 1, fmt, ap);
	if (len)
		Report(type, info, len);
	va_end(ap);
}


void ReportManager::StartReportSocket() {
	xstring xx = getAppPath();
	xx += _T("\\rcEnv.ini");
	wchar_t host[1024];
	xstring report_addr = L"report.rex.com";
	nsox::nu_auto_ptr<xproto::IProtoLink> spLink = (xproto::IProtoLink*)xproto::CoCreateProto()->queryObject(PROTO_LINK_OBJ);
	if (spLink != NULL) {
		if (spLink->GetAddrConfig()) {
			report_addr = spLink->GetAddrConfig()->ReportServerAddr();
		}
	}
	GetPrivateProfileStringW(L"xproto-report", L"host", report_addr.data(), host, 1024-1, xx.c_str());
	uint16_t port = GetPrivateProfileIntW(L"xproto-report", L"port", 4001, xx.c_str());
	if(tcp_socket_){
		tcp_socket_ = NULL;
	}
	tcp_socket_ = nsox::nu_create_object<CWsTcpSocket>::create_inst();
	tcp_socket_->Create();
	tcp_socket_->SetProtoHandler(this);
	std::string host_str = app2net(host);
	tcp_socket_->AsynConnect(host_str.c_str(), port);

	if(udp_socket_){
		udp_socket_->close_conn();
		udp_socket_  = NULL;
	}
	udp_socket_ = nsox::nu_create_object<CWsUdpSocket>::create_inst();
	udp_socket_->setProtoHandler(this);
	udp_socket_->create();
	udp_socket_->connect(host_str.c_str(), port);

}

void ReportManager::onclose(int err, nproto::connection* c) {
	tcp_socket_ = NULL;
}

void ReportManager::oncreate(nproto::connection* c) {
	nsox::nu_auto_ptr<xproto::IConfigManager> config_manager = (xproto::IConfigManager*)xproto::CoCreateProto()->queryObject(CONFIG_MANAGER_OBJ);
	if (config_manager) {
		xproto::IConfig* conf = config_manager->getConfig(_T("link-historys"));
		if (conf) {
			std::vector<xstring> iplist;
			conf->getList(0, iplist);
			if (iplist.size()) {
				for(int i = 0; i < iplist.size(); ++i) {
					//PingServerManager::inst()->BeginPing(app2net(iplist[i]).data(), this, 5000);
				}
			}
		}
	}
}



void ReportManager::OnReportTimeout() {
	if (tcp_socket_)
		tcp_socket_->close_conn();
	tcp_socket_ = NULL;
}