#ifndef BASE_SVID_PROTOCOL_H_
#define BASE_SVID_PROTOCOL_H_
#include <string>
namespace protocol{
		enum svid {
			BASEAPP_SVID  = 0,  // 基本服务的编号,基本服务全部都回被重定向到这个服务处理
			////////////////////////////////////////////////////////////////////////////
			// basic            // 数量 说明               locate-hash
			LBS_SVID   = 1,  // lbs
			SESSION_SVID  = 2,  // 10 会话服务，包括群     eSession
			UINFO_SVID    = 3,  // 10 用户资料，           eHashUid：操作对应的uid
			LINKD_SVID = 4,		//02 登陆注销   eHashUid
			CLIENT_SVID =5,
			SLIST_SVID  = 6,  
			SDB_SVID = 7,    //session db
			SMANAGER_SVID = 8,
			ONLINE_SVID = 9,
			UDB_SVID = 10,
			JIFEN_SVID = 11,
			SNAP_SVID = 12,
			CATALOG_SVID = 13,
			HEART_SVID = 15, // 管理服务器
			DAEMON_SVID = 18,
			ECHO_SVID = 19,	//echod
			DATACENTER_SVID = 20,
			IM_SVID = 21,
			WB_SVID	  = 22, // white board
			PLUGIN_SVID = 23,
			SESS_LOGS_DC_SVID = 24,
			SESS_INIT_DC_SVID = 25,
			SESS_JOIN_DC_SVID = 26,
			LINK_JOIN_DC_SVID = 27,
			BASEAPP_MAX_SVID = 255, // 保留的基本服务编号范围

		};
		
		static std::string names[] = {"", "login", "session", "uinfo", "lbs"};
		
		enum HashType {
			ROUND_ROBIN,
			UID_HASH,
			SESSION_HASH,
			FIX,
			CLIENT_SESSION_HASH,
			UNKNOWN,
			AUTO,
		};
}
#endif /*BASE_SVID_H_*/
