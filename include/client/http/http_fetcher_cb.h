#pragma once
#include "neohttp.h"
#include <json/json.h>

extern std::string GetRcAuthorizationString();
extern bool isOfficialHost(const std::string& host);

inline bool isOfficialUrl(const std::string& url) {
	std::string host = url;
	size_t pos = host.find("://");
	if (pos != std::string::npos) {
		host = host.substr(pos + 3); // 移除协议部分
	}
	pos = host.find("/");
	if (pos != std::string::npos) {
		host = host.substr(0, pos); // 移除路径部分
	}
	return isOfficialHost(host);
}


struct http_fetcher_cb {
	virtual void on_http_data(int context, int type, const std::string& data) = 0;
	virtual void on_http_error(int context, int type, const std::string& data, int status) = 0;
};


struct IHttp_fetcher
{
	virtual int  get(LPCTSTR url,int type, http_fetcher_cb*,bool bIgnoreCache = false) = 0;
	virtual int  post(LPCTSTR url, http_post_data& params, int type, http_fetcher_cb*) = 0;
	virtual int  postJson(LPCTSTR url, Json::Value& params, int type, http_fetcher_cb*) = 0;
	virtual void remove(http_fetcher_cb * cb)										   = 0;
	virtual void remove(int nContext)												   = 0;
	virtual void clear()															   = 0;
	virtual ~IHttp_fetcher(){};
};



