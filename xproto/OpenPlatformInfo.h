#ifndef OPEN_PLATFORM_INFO_fwaefwaefweaf
#define OPEN_PLATFORM_INFO_fwaefwaefweaf

#include <string>
#include <map>
#include <vector>
#include "xstring.h"
#include "common/int_types.h"
#include "proxyManager.h"
#include "client/proto/IProto.h"

/****************************************
* 参数说明：
* platform.get_bind_by_pf		// 获取绑定 根据平台信息
* postdata=[{"id":100000179227156,"type":1},{"id":100000187445370,"type":1}]
* 字段说明：
* id 代表 第三方平台的唯一ID，如facebook ID
* type 代表 第三方平台的类型，目前：type=1 facebook ，其它的暂无定义 		（类型： TINYINT(4) UNSIGNED）

*****************************************/	


/****************************************
* get_bind_by_rc.php		// 获取绑定 根据RC信息
* 
* postdata=[{"id":241332,"type":1},{"id":241379,"type":1}]
* 字段说明：
* id 代表 RC ID，如 uid，sid
* type 代表 id的类型，1 代表 uid， 2代表 sid
*****************************************/	

/****************************************
* set_bind.php		// 新增绑定 或 修改绑定

* postdata={"rc_id":1124584,"rc_type":1,"pf_id":"1231123124234123","pf_type":1,"pf_token":"asdfgsd-fwefwe-fsfwe","pf_email":"","pf_url":""}
* 字段说明：
* rc_id  代表 sid 或 uid 等
* rc_type  代表 rc_id的类型，1 代表 uid， 2代表 sid
* pf_id   第三方平台的唯一ID，如facebook ID
* pf_type  代表 第三方平台的类型，目前：type=1 facebook ，其它的暂无定义 		（类型： TINYINT(4) UNSIGNED）
* pf_token
* pf_email
* pf_url
*****************************************/	

/****************************************
* rm_bind.php			// 解除绑定

* postdata={"rc_id":241276,"rc_type":1,"pf_type":1}
* 字段说明：
* rc_id  代表 sid 或 uid 等
* rc_type  代表 rc_id的类型，1 代表 uid， 2代表 sid
* pf_type  代表 第三方平台的类型，目前：type=1 facebook ，其它的暂无定义 		（类型： TINYINT(4) UNSIGNED）
*****************************************/	

class OpenPlatformInfoManager {
	OpenPlatformInfoManager();
public:
	static OpenPlatformInfoManager* Instance();
	~OpenPlatformInfoManager();

	void GetUserOpenPlatformInfo(uint32_t uid);
	void SetUserOpenPlatformInfo(uint32_t uid, uint32_t platform_id, const std::string& platform_user_id,   
			const std::string & platform_user_name, const std::string& pf_token, const std::string& pf_email, const std::string& pf_url);
	void RemoveUserOpenPlatformInfo(uint32_t uid, uint32_t platform_id);


	void GetSessionOpenPlatformInfo(uint32_t sid);
	void SetSessionOpenPlatformInfo(uint32_t sid, uint32_t platform_id, const std::string& platform_user_id,  
			const std::string & platform_user_name, const std::string& pf_token, const std::string& pf_email, const std::string& pf_url);
	void RemoveSessionOpenPlatformInfo(uint32_t sid, uint32_t platform_id);

	void OnGetOpenPlatformInfo(const xstring& method, uint32_t resCode, const xstring& res, uint32_t idd);
	void OnSetOpenPlatformInfo(const xstring& method, uint32_t resCode, const xstring& res, uint32_t idd);
	void OnRemoveOpenPlatformInfo(const xstring& method, uint32_t resCode, const xstring& res, uint32_t idd);

private:
	void GetOpenPlatformInfo(uint32_t type, uint32_t sid, int time);
	void SetOpenPlatformInfo(uint32_t type, uint32_t sid, uint32_t platform_id, const std::string& platform_user_id,  
			const std::string & platform_user_name, const std::string& pf_token, const std::string& pf_email, const std::string& pf_url);
	void RemoveOpenPlatformInfo(uint32_t type, uint32_t sid, uint32_t platform_id);
private:
	xproto::ProxyHanler<OpenPlatformInfoManager> get_open_platform_info_proxyer_;
	xproto::ProxyHanler<OpenPlatformInfoManager> set_open_platform_info_proxyer_;
	xproto::ProxyHanler<OpenPlatformInfoManager> remove_open_platform_info_proxyer_;
	std::map<uint64_t, std::vector<xproto::OpenPlatformInfo> > open_platform_infos_;
	std::map<uint64_t, uint32_t> ids_;
};

	


#endif