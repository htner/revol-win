
#define POSTFIX							_T(_POSTFIX_)
//#define REX_REGKEY_FOLDER			_T("software\\RC") POSTFIX
#define REX_PREFIX						_T("@") USER_ACCOUNT_POSTFIX

#define REX_INST_KEY_60			_T("gateway.rex.") POSTFIX _T(".instance.6.0")

#define MAIN_REX_DOMAINWND_FLAG			_T("rex_main_window.") POSTFIX

//#define REX_INST_KEY				_T("gateway.rex.instance") POSTFIX
#define ARGS_PIPE_NAME					("\\\\.\\pipe\\rex_args_pipe_" _POSTFIX_)

#define RC_EXIST_FLAG					_T("EXIST_FLAG") POSTFIX

#define LANG_POSTFIX1					boost::str(boost::wformat(_T("&lang=%s")) % (nxctrl::getLang()->getCurrentLang()).c_str())
#define LANG_POSTFIX2					boost::str(boost::wformat(_T("?lang=%s")) % (nxctrl::getLang()->getCurrentLang()).c_str())
#define PUT_LANG_POSTFIX1(url)			(xstring(url) + LANG_POSTFIX1)
#define PUT_LANG_POSTFIX2(url)			(xstring(url) + LANG_POSTFIX2)
#define VALID_URL_TAILS					{_T("rex.com"),_T("raidtalk.com"),_T("rtalkdns.com")}

/************************************************************************/

#define URL_SESSION_CHECK					_T("http://") API_REX_DOMAIN _T("/session/v1/precheck?name=")
#define URL_CRASH_REPORT					_T("http://") API_REX_DOMAIN _T("/crash.php?version=%s&type=%s")
#define REPORT_TYPE_REX				_T("Rex")
#define REPORT_TYPE_RCPLUGIN				_T("rcplugin")
#define URL_SEARCH_GROUP					_T("http://") REX_SEARCH _T("/") REX_SEARCH_PAGE _T("?ver=%s&q=%s")
#define URL_INSTANT_SEARCH					_T("http://") API_REX_DOMAIN _T("/search.php?keyword=%s&ver=%s&lang=%s")


//////////////////////////////////////////////////////////////////////////

#define URL_PRELOADER						_T("http://") REX_API_DOMAIN _T("/preload/index.php?ver=")
#define URL_GAMELIST						_T("http://") REX_API_DOMAIN _T("/server/gamelist.xml")
#define URL_GAMEAPPLIST						_T("http://") ADS_REX_DOMAIN _T("/nads/App/app.php?ver=%s&uid=%d")
#define URL_GAMEAPPINTRO					_T("http://") GAME_REX_DOMAIN	_T("/openapi/get_gameinfo?ver=%s&uid=%d&id=%d")
#define URL_GAMEAPP_SEARCH					_T("http://") GAME_REX_DOMAIN _T("/openapi/searchWebGame?ver=%s&uid=%d&wd=%s")
#define URL_GAMEAPP_NAVIGATE				_T("http://") GAME_REX_DOMAIN _T("/openapi/searchNavigate?ver=%s&uid=%d")
#define URL_GAMEAPP_NAVGAMES				_T("http://") GAME_REX_DOMAIN _T("/openapi/searchTypeGames?ver=%s&uid=%d&type=%s")
#define URL_CREATE_GROUP					_T("http://") REX_API_DOMAIN _T("/session/v1/create?authorization=%s&type=%s&subtype=%s&slogan=%s&name=%s&owner=%d&isp=%d")
#define URL_ERROR_REPORT                    _T("http://") REX_API_DOMAIN _T("/server/operate.php?account=%s&mac_addr=%s&operation=%d&result=%d&ver=%s")
#define URL_APPLY_SETMEMBER					_T("http://") REX_API_DOMAIN _T("/server/setMember.php?authorization=%s&account=%s&sid=%s&operator=%s&act=%d&op=%d")
#define URL_RELIEVE_GROUP_STATUS			_T("http://") REX_API_DOMAIN _T("/server/removeGroup.php?authorization=%s&account=%s&sid=%s")
#define URL_INFO_REPORT                     _T("http://") REX_API_DOMAIN _T("/server/operates.php")
#define URL_AVATAR_MD5_LIST					_T("http://") REX_API_DOMAIN _T("/user/getFriendImgMd5.php?authorization=%s&account=%s")
#define URL_GET_SERVER_GAME_LIST			_T("http://") REX_API_DOMAIN _T("/server/getGameList.php?authorization=%s&owner=%d&sid=%d&type=%d")
#define URL_SET_SERVER_GAME_LIST			_T("http://") REX_API_DOMAIN _T("/server/setGameList.php?authorization=%s&owner=%d&sid=%d&type=%s")
#define URL_LANGUAGE_UPDATE					_T("http://") REX_API_DOMAIN _T("/client/LanguageIni/index.php?type=language&ver=%s")
#define URL_CHECKFACEBOOKBIND				_T("http://") REX_API_DOMAIN _T("/user/fb_check.php?fb_uid=%s&uid=%ld")
#define URL_GET_SESSION_INFO				PUT_LANG_POSTFIX1(_T("http://") REX_API_DOMAIN _T("/server/getsinfo.php?authorization=%s&account=%s&sid=%s"))
#define URL_GET_USER_INTRO_INFO				PUT_LANG_POSTFIX1(_T("http://") REX_API_DOMAIN _T("/user/get_user_new.php?account=%s"))
#define URL_FLASH							PUT_LANG_POSTFIX1(_T("http://") REX_API_DOMAIN _T("/user/loadFlash.php?ver=%s&name=%s"))
#define URL_GET_RANKLIST					PUT_LANG_POSTFIX1(_T("http://") REX_API_DOMAIN _T("/session/v1/navlist?authorization=%s&uid=%d&ver=%s"))
#define URL_GET_IM_ADS						PUT_LANG_POSTFIX1(_T("http://") REX_API_DOMAIN _T("/server/getimads.php?authorization=%s&uid=%d"))
#define URL_GET_MAIN_INFO					PUT_LANG_POSTFIX1(_T("http://") REX_API_DOMAIN _T("/home/v1/summary?uid=%d&ver=%s&timestamp=%s&t=%ld"))
#define URL_GET_BULLETIN_VIDEO				_T("http://") REX_API_DOMAIN _T("/server/bulletin_video.php?sid=%d&cid=%d&url=%s")
#define URL_AUTOADD_FBRCFRIEND				_T("http://") REX_API_DOMAIN _T("/user/add_fb_rc_friend.php")
//////////////////////////////////////////////////////////////////////////

#define URL_GET_USER_LOGO_DEF				_T("http://")  _T("/user/getimg.php?uid=%d&type=100")
#define URL_GET_SERVER_LOGO_DEF				_T("http://") REX_API_DOMAIN_2 _T("/server/getimg.php?sid=%d&uid=%d&type=100")
#define URL_GET_USER_LOGO_TYPE				_T("http://") WWW_REX_DOMAIN _T("/web/fetch/user/avatar?uid=%2%&type=%1%")
#define URL_GET_SERVER_LOGO_TYPE			_T("http://") WWW_REX_DOMAIN _T("/web/fetch/session/logo?sid=%d&uid=%d&type=%d")
#define URL_PUT_USER_LOGO					_T("http://") WWW_REX_DOMAIN _T("/www/upload/user/avatar.html?uid=%d&authorization=%s")
#define URL_PUT_SERVER_LOGO					_T("http://") WWW_REX_DOMAIN _T("/www/upload/session/logo.html?sid=%d&uid=%s&authorization=%s")
#define URL_SERVER_AVATAR_MD5_LIST			_T("http://") REX_API_DOMAIN_2 _T("/server/getSidMd5.php")
#define URL_UPDATE_FRIEND_FEED				_T("http://") REX_API_DOMAIN_2 _T("/feed/im_feed.php")
#define URL_QUERY_CHAT_IMAGE				_T("http://") REX_API_DOMAIN_2 _T("/user/querychatimg.php?md5=%s")
#define URL_PUT_CHAT_IMAGE					_T("http://") REX_API_DOMAIN_2 _T("/user/uploadchatimg.php?filename=%s&uid=%d&authorization=%s&md5=%s")
#define URL_FRIEND_FEED						PUT_LANG_POSTFIX1(_T("http://") WWW_REX_DOMAIN _T("/web/v1/feeds?authorization=%s&uid=%d&ver=%s"))

//////////////////////////////////////////////////////////////////////////
#define URL_WEB_LOAD_RC					_T("http://") WWW_REX_DOMAIN _T("/group.html?sid=%d")
#define REX_URL_OFFICIAL			_T("http://") WWW_REX_DOMAIN _T("/rcgo.php?itype=1&uid=%d&cookie=%s")
#define REPORT_CREATE_SERVER			_T("http://") WWW_REX_DOMAIN _T("/rpcsvr.php?authorization=%s")
#define REX_URL_DIRECTVISTLINK     _T("http://") WWW_REX_DOMAIN _T("/direct.php?url=%s&uid=%d&authorization=%s")
#define REX_URL_LEVEL_INTRODUCE	_T("http://") WWW_REX_DOMAIN _T("/faq.php#1")
#define REX_URL_SHARE_GROUP		_T("http://") WWW_REX_DOMAIN _T("/getcode.php?sid=%s&sname=%s")
#define REX_URL_MANAGER_GROUP		_T("http://") WWW_REX_DOMAIN _T("/user/group/index.php?sid=%d&uid=%d&authorization=%s")
#define REX_URL_FLOWER             _T("http://") WWW_REX_DOMAIN _T("/user/group/flower.php?sid=%d&uid=%d&authorization=%s")
#define REX_URL_CHANNELEVENT       _T("http://") WWW_REX_DOMAIN _T("/user/group/channel_app.php?sid=%d&uid=%d&authorization=%s")
#define REX_URL_CHANNELWIDGET_APPLY       _T("http://") WWW_REX_DOMAIN _T("/user/group/app.php?sid=%d&pid=%d&uid=%d&authorization=%s")
#define REX_URL_APPLY				_T("http://") WWW_REX_DOMAIN _T("/apply.php")
#define REX_URL_OVERGAMELIST		_T("http://") WWW_REX_DOMAIN _T("/support_gamelist.php")
#define REX_MEDAL_CONFIG			PUT_LANG_POSTFIX1(_T("http://") REX_API_DOMAIN _T("/summary/v1/medal/config?ver=%s"))
#define REX_URL_FAQ_DIRECT		    PUT_LANG_POSTFIX1(_T("http://") WWW_REX_DOMAIN _T("/faqhelp.php?type=%s"))
#define REX_URL_POLICIES			PUT_LANG_POSTFIX2(_T("http://") WWW_REX_DOMAIN _T("/policies.php")).c_str()
#define REX_URL_PRIVACY			PUT_LANG_POSTFIX2(_T("http://") WWW_REX_DOMAIN _T("/agreement.php")).c_str()
#define REX_URL_HELPCENTER         PUT_LANG_POSTFIX2(_T("http://") WWW_REX_DOMAIN _T("/help.php")).c_str()
#define REX_URL_FAQ				PUT_LANG_POSTFIX2(_T("http://") WWW_REX_DOMAIN _T("/faq.php")).c_str()
#define REX_URL_REGNEW             PUT_LANG_POSTFIX2(_T("http://") WWW_REX_DOMAIN _T("/register.php"))
#define REX_URL_SIGNUP             PUT_LANG_POSTFIX2(_T("http://") WWW_REX_DOMAIN _T("/user/signup"))
#define REX_URL_GETPASS            PUT_LANG_POSTFIX2(_T("http://") WWW_REX_DOMAIN _T("/getpass.php"))
#define REX_URL_DOWNLOAD			PUT_LANG_POSTFIX2(_T("http://") WWW_REX_DOMAIN _T("/download.php"))

#define FACEBOOK_DIRECTURI				_T("http://") WWW_REX_DOMAIN _T("/client/api/fb_bind_succ.php")
#ifdef REX_RU
#define REX_URL_CONTACTUS				_T("http://")WWW_REX_DOMAIN _T("/contactus.html")
#else
#define REX_URL_CONTACTUS				_T("http://") WWW_REX_DOMAIN _T("/contactus.php")
#endif

#define FACEBOOK_INVITE_GUIDE			_T("http://") WWW_REX_DOMAIN _T("/client/facebook/index.php?uid=%d&cookie=%s")
#define FACEBOOK_USERC_COUNT			_T("http://") WWW_REX_DOMAIN_SL _T("/v2/api/get_fb_friends.php")

//////////////////////////////////////////////////////////////////////////
#define REX_URL_USER_MEDAL			PUT_LANG_POSTFIX1(_T("http://") WWW_REX_DOMAIN_SL _T("/www/medal/user_medal.php?uid=%d&authorization=%s&ver=%s"))
#define URL_GET_USEREFFECT				_T("http://") WWW_REX_DOMAIN_SL _T("/www/medal/user_medal_list.php?uid=%d")
#define REX_URL_LOG				_T("http://") WWW_REX_DOMAIN_SL _T("/statics.php")
//#define REX_URL_TOPGAME			_T("http://") WWW_REX_DOMAIN_SL _T("/myprivilege/topgames.php?uid=%d&authorization=%s&uname=%s")
#define REX_URL_TOPGAME			_T("http://") GAME_REX_DOMAIN_GIFT _T("/rc_mini?uid=%d&authorization=%s&uname=%s")
#define REX_URL_GAMEGIFT			_T("http://") GAME_REX_DOMAIN_GIFT _T("/rc_mini/feed_num?uid=%d&authorization=%s&ver=%s")

#define REX_URL_GAMEFEED			_T("http://") WWW_REX_DOMAIN_SL _T("/www/dynamic/index.php?type=GetDynamic&uid=%s")


#define REPORT_CHANNEL_CHAT 			PUT_LANG_POSTFIX2(_T("http://")	WWW_REX_DOMAIN_SL	_T("/summary/v1/rc_client/report"))
#define REX_URL_FEEDBACK			PUT_LANG_POSTFIX2(_T("http://") WWW_REX_DOMAIN_SL _T("/summary/v1/feedback"))
#define REX_URL_GAMEBOX			PUT_LANG_POSTFIX2(_T("http://") WWW_REX_DOMAIN_SL _T("/client/top_games/GameBox.php"))
/************************************************************************/

#define URL_PRODUCT_STATUS					_T("http://") REX_UPLOAD_1 _T("/statics.php?authorization=%s&uid=%d")
#define URL_PRODUCT_STATUS_EX				_T("http://") REX_UPLOAD_1 _T("/statistics.php?authorization=%s")
#define URL_PRODUCT_ACTION_STATUS			_T("http://") REX_UPLOAD_1 _T("/actstate.php?authorization=%s")
#define URL_CHAT_IMAGE						_T("http://") REX_UPLOAD_2 _T("/images/%s")

#define REX_URL_CHECKVER				_T("http://") REX_UPDATE _T("/check.php?ver=")
#define REX_URL_CHECKVER_NEW			_T("http://") REX_UPDATE _T("/web/v1/update/check?ver=")


#define REX_FORUM					_T("http://") REX_FORUM_DM _T("/rcgo.php?itype=1&uid=%d&cookie=%s")

#define REX_ADS_FEED				PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/api/product_ads.php?uid=%d&authorization=%s&ver=%s"))
#define REX_URL_GAME_LOBBY			PUT_LANG_POSTFIX2(_T("http://") ADS_REX_DOMAIN _T("/gamebox/game.php"))
#define REX_URL_NEWS				PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/api/news.php?ver=%s"))
#define REX_URL_BANNER				PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/api/main_banner.php?ver=%s"))
#define UPLOAD_FOLD_TIME				PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/nads/MainFootAds/mainfoot.php?type=html&uid=%d&ver=%s&time=%d"))


#define REX_LOGIN_AWARD			PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/nads/LoginWindowAds/loginwin.php?uid=%d&ver=%s"))
#define REX_LOGIN_ADS              _T("http://") ADS_REX_DOMAIN _T("/nads/LoginAds/logintheme.php?type=login&uid=%d&ver=%s")

#define REX_FRIEND_ADS             PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/nads/FriendAds/friend.php?type=friend2&uid=%d&ver=%s"))
#define REX_CHANNEL_ADS            PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/nads/SessTalkAds/sesstalk.php?type=sesstalk2&sess=%d&ver=%s"))
#define REX_IM_ADS                 PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/nads/ImTalkAds/imtalk.php?type=imtalk2&uid=%d&ver=%s"))
#define REX_CHATLOGO_ADS			PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/nads/ImTalkAds/imtalk.php?type=imtalkimg&uid=%d&ver=%s"))

#define URL_SKIN_UPDATE					PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/nads/SkinsAds/skin.php?type=skins&ver=%s&current=%ld"))
#define	FRAME_AD_VIEW					PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/nads/get_config.php?modules=mainfoot&uid=%d&ver=%s"))

#define URL_IDLE_ADS_CONFIG				PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/nads/get_config.php?modules=Config&uid=%d&ver=%s"))
#define URL_IDLE_ADS_AFK				PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/nads/LeisureReg/trigger.php?uid=%d&ver=%s&type=intime"))
#define URL_IDLE_ADS_OFFPC				PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/nads/LeisureAlert/hangup.php?uid=%d&ver=%s&type=intime"))
#define URL_IDLE_ADS_LAYER				PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/nads/Leisure/mask.php?uid=%d&ver=%s&type=intime"))
#define REX_VERITY                 PUT_LANG_POSTFIX1(_T("http://") ADS_REX_DOMAIN _T("/nads/VerifyPassAds/verifypass.php?sid=%d"))

#define URL_UPDATE_FLASH				_T("http://get.adobe.com/flashplayer/")
#define URL_UPDATE_IE					_T("http://windows.microsoft.com/en-us/windows/home")


//////////////////////////////////////////////////////////////////////////sign	
#define URL_SIGNIN_AGREEMENT			_T("http://") WWW_REX_DOMAIN _T("/agreement.php")
#define URL_GET_CAPTCODE _T("http://") REX_API_DOMAIN _T("/user/imgcode.php")
#define URL_CHECK_CAPTCODE _T("http://") REX_API_DOMAIN _T("/user/checkcode.php?imgcode=")
#define URL_CHECK_ACCOUNTEXITED _T("http://") REX_API_DOMAIN _T("/user/v1/check_account?account=")
#define URL_CHECK_EMAIL _T("http://") REX_API_DOMAIN _T("/user/v1/check_email?email=%s")
#define URL_SEND_SIGNINFO _T("http://") REX_API_DOMAIN _T("/user/v1/signup")

//////////////////////////////////////////////////////////////////////////
#define URL_GET_VIPSMILE_PACKLIST	_T("http://") REX_API_DOMAIN _T("/smilePacks/cfg.xml")
#define URL_GET_VIPSMILE_PACK		_T("http://") REX_API_DOMAIN _T("/smilePacks/BigEmoji.zip")

///////////////////// raidpay /////////////////////////////////////////////////////

#define REX_VIP_LINK                 _T("http://") PAY_REX_DOMAIN _T("/index.php?c=home&a=autoLogin&goto=service.vip&uid=%d&authorization=%s")

/************************************************************************/


//////////////////////////////////////////////////////////////////////////




//korea
#define REX_INST_PORT1			7777
#define REX_INST_PORT2			7123
#define REX_INST_PORT3			7222

//taiwan
#define REX_INST_PORT4			4759
#define REX_INST_PORT5			4321
#define REX_INST_PORT6			4333

//enus
#define REX_INST_PORT7			5759
#define REX_INST_PORT8			5333
#define REX_INST_PORT9			5346

//Vietnam
#define REX_INST_PORT10		6666
#define REX_INST_PORT11		6667
#define REX_INST_PORT12		6668

