#ifndef CROSS_PLANTFORM_CONST_H_
#define CROSS_PLANTFORM_CONST_H_

#define MAX_SVID_NUM 100
#define SOCKET_TIMEOUT 60000
#define MYCOLLECTION_FOLDER			1000000000		//我收藏的列表
#define PUBLICCHANNEL_FOLDER		1000000001
#define RECEMMONDCHANNEL_FOLDER		1000000002
#define HOT_FOLDER					1000000003
#define MYCREATED_FOLDER			1000000004		//我加入的列表
#define RECENT_FOLDER				1000000005		//最近訪問列表
#define MYFOND_FOLDER				1000000010      //我喜歡的列表
#define MYGROUPS_FOLDER				9999999999		//個人專屬



#define UDP_OFFSET 10000

#define MAX_CHANNEL_SIZE 3
#define MAX_SUBCHANNEL_SIZE 100
#ifdef WIN32  	 //===========================================
#define FAV_FOLDER		_T("1000000000")
#define FAV_PUBLIC			_T("1000000001")
#define FAV_RECOMMEND		_T("1000000002")
#define FAV_HOT			_T("1000000003")
#define FAV_MYSERVER		_T("1000000004")
#define FAV_RECENT		_T("1000000005")
#define FAV_GUILD		_T("1000000006")
#define KICKOUT_PID 		0
#else	 	//===================MAC VERSION ===============
#define FAV_FOLDER		L"1000000000"
#define FAV_PUBLIC		L"1000000001"
#define FAV_RECOMMEND	L"1000000002"
#define FAV_HOT			L"1000000003"
#define FAV_MINE        L"1000000004"
#define FAV_RECENT      L"1000000005"
#define FAV_GUILD		L"1000000006"
#endif 		//============================================

#define SEQPAGE					1000
#define PACKET_SEQ_INCREMENT		2

#define DEF_SESSIONKEY_LENGTH 16

#define ADMIN_CHANNEL 0

namespace protocol {
	namespace uinfo {
		enum SEX {
			female,
			male
		};

	}
}

namespace protocol{
	namespace login{
	enum LinkStatus {
		//sync_uinfo = 100,
		//sync_ulist = 180,
		LINK_INIT,
		LINK_LBS,
		EXCHANGE_PASSWORD,
		LBS_ERROR,
		LINK_AUTH,
		LOGIN_SUCCESS,
		PASSWD_ERROR,
		SERVER_ERROR,
		NET_BROKEN,
		TIMEOUT,
		KICKOFF,
		LOGOUT,
		UNKNOWN,
		PROTOCOL_OLD,  //協議版本
		LINK_GETMAIL,
		NON_EMAIL,
		LBS_SHUTDOWN,
		RELOGIN_SUCCESS,
		SERVER_REDIRECT,
		USER_NONEXIST,
		LBS_ERROR_DNS, //20
		LINK_KEY_ERROR,
		LBS_NET_ERROR,
		LINK_NET_ERROR,
		LOGIN_TIMEOUT,
		CLIENT_OLD,		//客戶端版本
		BLACK_ACCOUNT,	//帳號被封
		BLACK_MACHINE,	//機器被封
		LINK_OVERTIME,
		LOCK_OVERTIME //連接次數過多,所以帳號被鎖一段時間
		//success = 200,
		//	passwd_err = 401,
		//	
		// timeout	  = 405,
		//	unknow	  = 406
	};
	}}

enum ISPType{
	AUTO_DETECT = 0,
	ISP_US = 1,	//US
	ISP_EU = 2,	//EU
	ISP_TW = 3, //TW
	ISP_CN = 4  //CN
};

#define ISP_DEFAULT	ISP_US

#define LINK_PING_TIME 20000
#define LOGIN_TRY_TIME  20000


#endif /*CROSS_PLANTFORM_CONST_H_*/
