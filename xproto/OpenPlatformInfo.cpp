#include "OpenPlatformInfo.h"
#include "RaidUser.h"

#include <json/json.h>
#include "helper/utf8to.h"
#include "helper/base64.h"
#include "SessionLiteInfo.h"
#include "client/proto/ismanager.h"
#include "client/proto/isession.h"


OpenPlatformInfoManager::OpenPlatformInfoManager() {
	get_open_platform_info_proxyer_.init(this, &OpenPlatformInfoManager::OnGetOpenPlatformInfo, _T("platform.get_bind_by_rc"));
	set_open_platform_info_proxyer_.init(this, &OpenPlatformInfoManager::OnSetOpenPlatformInfo, _T("platform.set_bind"));
	remove_open_platform_info_proxyer_.init(this, &OpenPlatformInfoManager::OnRemoveOpenPlatformInfo, _T("platform.rm_bind"));
}

OpenPlatformInfoManager* OpenPlatformInfoManager::Instance() {
	static OpenPlatformInfoManager open;
	return &open;
}


OpenPlatformInfoManager::~OpenPlatformInfoManager() {
	//get_open_platform_info_proxyer_.uninit();
	//set_open_platform_info_proxyer_.uninit£¨);
	//remove_open_platform_info_proxyer_.uninit();
}

void OpenPlatformInfoManager::GetUserOpenPlatformInfo(uint32_t uid) {
	GetOpenPlatformInfo(1, uid, 10);
}

void OpenPlatformInfoManager::SetUserOpenPlatformInfo(uint32_t uid, uint32_t platform_id,  const std::string& platform_user_id,  const std::string & platform_user_name,const std::string& pf_token, const std::string& pf_email, const std::string& pf_url) {
	SetOpenPlatformInfo(1, uid, platform_id, platform_user_id, platform_user_name, pf_token, pf_email, pf_url);
}

void OpenPlatformInfoManager::RemoveUserOpenPlatformInfo(uint32_t uid, uint32_t platform_id) {
	RemoveOpenPlatformInfo(1, uid, platform_id);
}

void OpenPlatformInfoManager::GetSessionOpenPlatformInfo(uint32_t sid) {
	GetOpenPlatformInfo(2, sid, 10);
}

void OpenPlatformInfoManager::SetSessionOpenPlatformInfo(uint32_t sid, uint32_t platform_id,  const std::string& platform_user_id,  const std::string & platform_user_name,const std::string& pf_token, const std::string& pf_email, const std::string& pf_url) {
	SetOpenPlatformInfo(2, sid, platform_id,platform_user_id, platform_user_name,  pf_token, pf_email, pf_url);
}

void OpenPlatformInfoManager::RemoveSessionOpenPlatformInfo(uint32_t sid, uint32_t platform_id) {
	RemoveOpenPlatformInfo(2, sid, platform_id);
}

void OpenPlatformInfoManager::GetOpenPlatformInfo(uint32_t type, uint32_t id, int time) {
	try {
		uint64_t id_real = (uint64_t)type << 32 | id;
		if (time && ids_.find(id_real) != ids_.end() && timeGetTime() - ids_[id_real] < time *1000) {
			return;
		}
		ids_[id_real]  = timeGetTime();
		Json::Value param;
		Json::Value node;
		node["type"] = type;
		node["id"] = id;
		param.append(node);
		Json::FastWriter fast_writer;
		std::string strJson = fast_writer.write(param);
		get_open_platform_info_proxyer_.request(0, _T(""), net2app(strJson));
	} catch(...) {
	}
}

void OpenPlatformInfoManager::SetOpenPlatformInfo(uint32_t type, uint32_t sid, uint32_t platform_id, const std::string & platform_user_id,
												  const std::string& platform_user_name, const std::string& pf_token, const std::string& pf_email, const std::string& pf_url) {
	try {
		Json::Value param;
		param["rc_type"] = type;
		param["rc_id"] = sid;
		param["pf_id"] = platform_user_id;
		param["pf_username"] = platform_user_name;
		param["pf_type"] = platform_id;
		param["pf_token"] = pf_token;
		param["pf_email"] = pf_email;
		param["pf_url"] = pf_url;
		Json::FastWriter fast_writer;
		std::string strJson = fast_writer.write(param);
		set_open_platform_info_proxyer_.request(0, _T(""), net2app(strJson));
	} catch(...) {
	}
}

void OpenPlatformInfoManager::RemoveOpenPlatformInfo(uint32_t type, uint32_t sid, uint32_t platform_id) {
	try {
		Json::Value param;
		param["rc_type"] = type;
		param["rc_id"] = sid;
		param["pf_type"] = platform_id;
		Json::FastWriter fast_writer;
		std::string strJson = fast_writer.write(param);
		remove_open_platform_info_proxyer_.request(0, _T(""), net2app(strJson));
	} catch(...) {
	}
}

/*


void OpenPlatformInfoManager::OnGetOpenPlatformInfo(const xstring& method, uint32_t resCode, const xstring& res, uint32_t idd) {
	try {
		if (resCode != 200) {
				return;
		}
		Json::Reader reader;
		Json::Value root;
		std::string str = helper::Base64::decode(app2net(res));
		if( !reader.parse(str, root) ) {
			assert(FALSE);
			return;
		}
		Json::Value elem = root["platform"];
		if (!elem.isArray()) {
			return;
		}

		uint32_t platform_id = 0;
		std::string pf_user_id;
		std::string pf_token;
		std::string pf_email;
		std::string pf_url;
		
		for (Json::UInt i = 0; i < elem.size(); ++i) {
			Json::Value node = elem[i];
			Json::UInt j = 0;
			platform_id = node["pf_type"].asInt();
			pf_user_id = node["pf_id"].asString();
			pf_token = node["pf_token"].asString();
			pf_email = node["pf_email"].asString();
			pf_url = node["pf_url"].asString();
		}

	} catch(...) {
	}	
}
*/


void OpenPlatformInfoManager::OnGetOpenPlatformInfo(const xstring& method, uint32_t resCode, const xstring& res, uint32_t idd) {
	try {
		if (resCode != 200) {
				return;
		}
		Json::Reader reader;
		Json::Value root;
		std::string str = helper::Base64::decode(app2net(res));
		if( !reader.parse(str, root) ) {
			assert(FALSE);
			return;
		}
		Json::Value elem = root["platform"];
		if (!elem.isArray()) {
			return;
		}

		uint32_t platform_id = 0;
		std::string pf_user_id;
		std::string pf_token;
		std::string pf_email;
		std::string pf_url;

		xproto::OpenPlatformInfo platform_info;
		xproto::OpenPlatformInfoVector infos;
		uint32_t id = 0;
		for (Json::UInt i = 0; i < elem.size(); ++i) {
			Json::Value node = elem[i];
			id = node["rc_id"].asInt();
			platform_info.id_type = node["rc_type"].asInt();
			platform_info.id = id;
			int pf_type =  node["pf_type"].asInt();
			if (pf_type == -1) {
				break;
			}
			
			platform_info.platform_id = node["pf_type"].asInt();
			platform_info.platform_user_id =  net2app(node["pf_id"].asString());
			platform_info.pf_token = net2app(node["pf_token"].asString());
			platform_info.pf_email = net2app(node["pf_email"].asString());
			platform_info.pf_url = net2app(node["pf_url"].asString());
			platform_info.platform_user_name = net2app(node["pf_username"].asString());


			infos.push_back(platform_info);
		}
		if (id) {
			if (platform_info.id_type == 1) {
				if (xproto::IUserManager* pUser = (xproto::IUserManager *)xproto::CoCreateProto()->queryObject(USER_MANAGER_OBJ)) {
					xproto::IUserInfo* m_userinfo = pUser->getUserInfo(platform_info.id);
					if (m_userinfo) {
						((xproto::CRaidUser*)m_userinfo)->AddPlatformInfo(infos);
					}
				}
			} else if (platform_info.id_type == 2) {
				protocol::slist::ISessionList* spSessionList = (protocol::slist::ISessionList*)xproto::CoCreateProto()->queryObject(SESSION_LIST_OBJ);
				if (spSessionList) {
					protocol::session::ISessionInfoBase* base_info = spSessionList->getSessionInfo(platform_info.id, false);
					if (base_info) {
						(base_info)->AddPlatformInfo(infos);
					}
				}
				protocol::session::ISessionManager* sessMgr = (ISessionManager*)xproto::CoCreateProto()->queryObject(SESSION_MANAGER_OBJ);
				if (sessMgr) {
					protocol::session::ISession* sess = sessMgr->GetJoined();
					if (sess && sess->getSid() == platform_info.id) {
						ISessionInfo* sess_info = sess->getSessionInfo();
						if (sess_info) {
							sess_info->AddPlatformInfo(infos);
						}
					}
				}
			}

		}
		//open_platform_infos_[]

	} catch(...) {
	}	
}


void OpenPlatformInfoManager::OnSetOpenPlatformInfo(const xstring& method, uint32_t resCode, const xstring& res, uint32_t idd) {
	try {
		if (resCode != 200) {
				return;
		}

		Json::Reader reader;
		Json::Value root;
		std::string str = helper::Base64::decode(app2net(res));
		if(!reader.parse(str, root) ) {
			assert(FALSE);
			return;
		}
		uint32_t id = root["rc_id"].asInt();
		uint32_t type = root["rc_type"].asInt();
		GetOpenPlatformInfo(type, id, 0);
	} catch(...) {
	}	
}


void OpenPlatformInfoManager::OnRemoveOpenPlatformInfo(const xstring& method, uint32_t resCode, const xstring& res, uint32_t idd) {
	try {
		if (resCode != 200) {
			return;
		}
		Json::Reader reader;
		Json::Value root;
		std::string str = helper::Base64::decode(app2net(res));
		if( !reader.parse(str, root) ) {
			assert(FALSE);
		}
			
		uint32_t id = root["rc_id"].asInt();
		uint32_t type = root["rc_type"].asInt();
		GetOpenPlatformInfo(type, id , 0);
	} catch(...) {
	}	
}

