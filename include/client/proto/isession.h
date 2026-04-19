#pragma  once
#include "common/misc.h"
#include "common/packet.h"
#include <protocol/psessionbase.h>
#include <protocol/authcode.h>
#include "islist.h"
#include "iproto.h"
#include "iwboard.h"
#include <set>

namespace protocol{
	namespace session{
		//会员状态信息
		enum MemberStatus {
			ms_offline = 0,
			ms_online,
			ms_InSession
		};

		enum MemberCardLimit{
			mbcl_none = 0,
			mbcl_self,
			mbcl_admin
		};

		enum CxDevice {
			dev_winpc = 0,
			dev_android = 1,
			dev_ios = 2,
			dev_macbook = 3,
			dev_unkown
		};

		enum VipLevel {
			vip_null = 0,
			vip_1 = 1,
			vip_2 = 2 ,
			vip_3 = 3 ,
			vip_4 = 4 ,
			vip_5 = 5 ,
			vip_6 = 6 ,
			vip_max = 10
		};

		//语音包管理
		struct  IPacketNotifyer{
				virtual void on_packet_loss(const UID_T& user, int seq) = 0;
				virtual void OnPacketLossChangeBad() = 0;
				virtual void OnPacketLossTooMuch() = 0;
		};
		
		//创建频道信息结构
		struct SessionCreateInfo{
		public:
			SessionCreateInfo(const xstring& n, bool lim, bool pub, 
				const xstring& h, const std::string& pa, const SID_T& pi,
				int tp, const xstring &tStr, VoiceQC vq = 0, ISPType sp = ISP_US, const SID_T& pre = 0)
			{
				name	  = n;
				islimit   = lim;
				isPub     = pub;
				hello	  = h;
				password = pa;
				pid		 = pi;
				type	= tp;
				typeStr = tStr;
				qc		= vq;
				isp		= sp;
				preCh		= pre;
			}

			SessionCreateInfo(){}
		public:
			
			bool islimit;
			bool isPub;
			SID_T		sid;	//服务器填写
			int			type; 
			SID_T		pid;
			SID_T		preCh;
			xstring		hello;
			xstring		name;
			xstring		typeStr;
			std::string password;

			VoiceQC		qc;
			ISPType		isp;

		};

		struct ISessMgrWatcher : public xproto::IXObject{
			virtual void onRefresh() = 0;
			/*
				err define:
				用户不存在 404
				你没有权限进行该操作（不是该频道管理员,或者权限对抗失败） 403
			*/
			virtual void onError(int err) = 0;
		};


		//
		struct IBanWatcher : public xproto::IXObject{
			virtual void onRefresh() = 0;
		};
		
		//
		struct IKickOffWatcher : public xproto::IXObject{
			virtual void onRefresh() = 0;
			virtual HULI getSid() = 0;
		};

		struct IMemberCardEvent  : public xproto::IXObject{
			virtual void onChange(const uint32_t &uid, const xstring &name, const uint32_t& modifyBy) = 0;
		};

		struct IMedalEvent  : public xproto::IXObject{
			virtual void onMedalRefresh() = 0;
			virtual void onMedalChange(const uint32_t &uid) = 0;
		};

		typedef bool (*MedalCompareFunction)(const PSimpleEffect&, const PSimpleEffect&); 

		struct IVIPEvent  : public xproto::IXObject{
			virtual void onVIPRefresh() = 0;
			virtual void onVIPChange(const uint32_t &uid) = 0;
		};

		struct XVoiceData
		{
					char*			payload;
					uint32_t			payloadsize;
					uint32_t			user;
					int				seq;
					XVoiceData*		next;
					VoiceType		vt;
					uint8_t			type;
					VoiceQC			qc;
		};

		struct ISessionInfo: public virtual ISessionInfoBase
						   , public virtual IConnPoint<ISessMgrWatcher>
						   , public virtual IConnPoint<IBanWatcher>
						   , public virtual IConnPoint<IKickOffWatcher>
		{
			virtual void setName(const xstring &n) = 0;
			virtual void changeMemberRight(const UID_T &uid, ChannelRoler nr) = 0;
			virtual void setPub(bool p) = 0;
			virtual void setLimit(bool l) = 0;
			virtual void setPass(const std::string &md5Pass) = 0;
			virtual void setBulletin(const xstring &hello) = 0;
			virtual void setMicTime(int t) = 0;
	

			virtual void setType(GuildType ) = 0;
			virtual void setTypeString(const xstring &) = 0;

			virtual void setMemberLimited(uint32_t ml) = 0;

			virtual void setArea(int n) = 0;
			virtual void setProvince(int n) = 0;
			virtual void setCity(int n) = 0;
			virtual void SetIsp(ISPType ntype) = 0;
			virtual void setStyle(ChannelStyle style) = 0;
			//走 list 通道
		
			virtual void unbanID(const UID_T &uid) = 0;
			virtual void unbanIP(uint32_t ip) = 0;

			virtual void batchUnbanID(const std::vector<UID_T> &uids) = 0;
			virtual void batchUnbanIP(const std::vector<uint32_t> &ips) = 0;

			virtual void update() = 0;
			virtual void setIntroduce(const xstring&) = 0;

			virtual void setVoiceQC(VoiceQC qc) = 0;

			virtual void setTxtInterval(int n) = 0;
			virtual void setTxtInitWait(int n) = 0;
			virtual void setTxtMaxLength(int n) = 0;
			virtual void setTxtLimitFormat(TxtFormatLimit tfl) = 0;
			virtual void setMicLimitQueue(bool b) = 0;
			virtual void setMicLimitFree(bool b) = 0;
			virtual void setApplyInfo(const xstring &info) = 0;
			virtual void setApplyUjifen(int jifen) = 0;
			virtual void setApplyUsjifen(int jifen) = 0;
			virtual void setSearchByIDLimit(bool b) = 0;
			virtual void setMemberListLimit(bool b) = 0;
			virtual void setJoinJifenLimit(int n ) = 0;
			virtual void setChannelDisable(bool b) =0;
			virtual void setAutoApply(int n) = 0;
			virtual void setChangeQuality(bool b) = 0;
			virtual void setHall(int hall) = 0;	
			virtual void setReception(int reception) = 0;	

			virtual void setLables(const xstring& lables) = 0;
			virtual void setSlogan(const xstring& slogan) = 0;

			virtual void setFlashUrl(const xstring& slogan) = 0;
			virtual void setAccessProperty(CAccessProperty) = 0;

			virtual void setNavigation(const xstring& na) = 0;

			virtual void setShowFlower(bool) = 0;	 //開通花的顯示屬性
			virtual void setOpenPlatformInfo(xproto::OpenPlatformInfo) = 0;
			virtual void removeOpenPlatformInfo(xproto::OpenPlatformInfo) = 0;
		};

		typedef ISessionInfo IChannelInfo;
		
		struct TextChat;

		struct IChatWatch : public xproto::IXObject{
			virtual void onText(const UID_T &from, const HULI &to, const TextChat &) = 0;
			virtual void onVoice(const UID_T &from, const HULI &to, uint32_t seq, uint32_t timeStamp, const std::string &) = 0;
			virtual void onStopVoice(const UID_T &who) = 0;
			virtual void onEvent(const UID_T &from, const HULI &to, uint32_t code, const xstring &) = 0;
			virtual void onBroadcasText(const UID_T &from, const HULI &to, const TextChat &) = 0;
		};

		struct IMemberChatWatch : public xproto::IXObject{
			virtual void onMemberText(const UID_T& from, uint32_t timeStamp, const TextChat &) = 0;
			virtual void onBatchMemberText(const std::vector<PHistoryData>&){};
		};

		struct INoticeWatch : public xproto::IXObject{
			virtual void onNotice(uint32_t type, uint32_t resources , const xstring& info) = 0;
		};

		struct IUserGuild{
		};


		template<typename T = UID_T>
		struct ITempMemberListEvent : public xproto::IXObject
		{
			virtual void onMemberRefresh() = 0;
			virtual void onMemberChange(const T &id) = 0;
			virtual void onMemberAdd(const T &id) = 0;
			virtual void onMemberRemove(const T &id) = 0;
		};


		//基本会员结构体
		template<typename T = UID_T>
		struct ITempMemberListBase : public virtual IConnPoint<ITempMemberListEvent<T> >
			, public xproto::IXObject
		{
				virtual   void getMember(std::vector<T>& ret) const = 0;
				virtual   int getMemberCount() const = 0;
		};

		typedef ITempMemberListEvent<> IMemberListEvent;
		typedef ITempMemberListBase<> IMemberListBase;

		struct IMemberCreditEvent  : public xproto::IXObject{
			virtual void onRefresh(const std::vector<std::pair<uint32_t, uint32_t> >& ) = 0;
		};

		struct IUserInfoEvent  : public xproto::IXObject{
			virtual void onRefresh(const std::set<uint32_t>& uids) = 0;
		};


		


		struct ISessionMemberList: public xproto::IRoomBase
								 , public virtual IConnPoint<xproto::IActionEvent>
								 , public virtual IConnPoint<IMemberCardEvent>
								 , public virtual IConnPoint<IMedalEvent>
								 , public virtual IConnPoint<IVIPEvent>
								 , public virtual IConnPoint<IMemberCreditEvent>
								 , public virtual IConnPoint<IUserInfoEvent>
								 , public IMemberListBase
		{
			virtual bool		isLimit() = 0;
			virtual bool		hasPasswd(const HULI &subId) const = 0;
			virtual uinfo::SEX	getSex(const UID_T &uid) const = 0;
			virtual LPCWSTR		getSign(const UID_T &uid) const = 0;
			virtual LPCWSTR		getStatus(const UID_T &uid) const = 0;
			virtual uint32_t 	getOrder(const UID_T &uid) const =0 ;
			virtual LPCWSTR		getClientInfo(const UID_T &uid) const = 0;
			virtual CxDevice	getClientDevice(const UID_T &uid) const = 0;

			virtual void		updateMemberList() const = 0;
			virtual void		changeFolder(const UID_T &from, const HULI &to, const std::string &passwd) = 0;
			virtual void		changeTo(const HULI &to, const std::string &passwd) = 0;
			virtual				ChannelRoler getRoler(const HULI &hid, const SID_T &channelId) const = 0;
			virtual void		changeMemberRight(const UID_T &uid, ChannelRoler nr, const HULI &channelId) = 0;

			//對帳號進行管理,手動轉入時用
			virtual void		changeMemberRight(const ACCOUNT_T &account, ChannelRoler nr, const SID_T &channelId) = 0;

			virtual void		kickOff(const UID_T & sid, const HULI &uid, const xstring& reason, uint32_t minutes) = 0;
			virtual void		tuoren(const UID_T &who, const HULI &from, const HULI &to) = 0;

			virtual void		enableText(const UID_T &uid, bool bEnable) = 0;
			virtual void		enableVoice(const UID_T &uid, bool bEnable) = 0;
			

			virtual bool		isUserTextable(const UID_T &uid) const = 0;
			virtual bool		isUserVoiceable(const UID_T &uid) const = 0;

			virtual void		mutedSelf(bool muted) = 0;  //muted = true 表示禁音
			virtual bool		isSelfMuted() const = 0;
			
			virtual bool		canVoiceOpt(const UID_T &who,const HULI &target) const = 0;
			virtual void		pushToMicrophoneList(const UID_T& uid) = 0;
			virtual void		setKeyActive(const UID_T &uid) = 0;

			virtual uint32_t	getUserJF(const UID_T &uid) const = 0;
			virtual uint32_t	getUserSessJF(const UID_T &uid) = 0;
			virtual uint32_t	getAddTime(const UID_T &uid) const = 0;
			virtual int	getUserSessContribute(const UID_T &uid) = 0;  // getUserSessJF，如果还没有读取到， 返回-1

			virtual void		enableAllText(const HULI &channelId, bool bEnable) = 0;

			virtual void		banAccount(const ACCOUNT_T &account) = 0;
			virtual void		banID(const UID_T &uid) = 0;
			virtual void		banIP(const UID_T &uid) = 0;

			virtual void		setAllKeyActive(const SID_T &channelId) = 0;
			virtual void		ignoreText(const UID_T &uid,bool ignore) = 0;
			virtual void		ignoreVoice(const UID_T &uid,bool ignore) = 0;
			virtual void		leaveGuild() = 0;
			virtual void		directKickoff(const HULI & sid, const HULI &uid, const xstring& reason, uint32_t minutes) = 0;
			virtual IUserGuild*	getUserGuilds(const UID_T &uid) = 0;
			virtual bool		isIgnoreText(const UID_T &uid) const = 0 ;
			virtual bool		isIgnoreVoice(const UID_T &uid) const = 0 ;
			virtual bool		isUserTextableInCh(const UID_T &uid) const = 0;
			virtual				ChannelRoler getMemberRoler(const UID_T &uid) const = 0 ;
			virtual				MemberStatus getMemberStatus(const UID_T &uid) const = 0;
			virtual bool		isSameParent(const UID_T &u1,const UID_T &u2) const = 0;
			///////////////////member card
			virtual LPCWSTR		getCardNick(const UID_T &hid) const  = 0;
			virtual LPCWSTR		getRealNick(const UID_T &hid) const = 0 ;
			virtual void		updateMemberCards(const UID_T&, const xstring &name, uint32_t nlimit)  = 0;
			virtual bool		isSelfUpdateMemberCards(const UID_T&) = 0;
			virtual bool		isAdminUpdateMemberCards(const UID_T&) = 0;
			virtual bool		isCardsNameExist(const xstring &name) = 0; // return the uid if exist, else return ""
			//virtual bool isUserTextableInCh(const xstring &uid, const xstring &chId) = 0;

			virtual SID_T		innerGetUserPid(const UID_T &uid) const = 0;
			virtual SID_T		innerGetChannelPid(const SID_T &pid) const = 0;
			virtual simeffect_const_t& getEffect(const UID_T &uid) const = 0;
			virtual VipLevel getVipLevel(const UID_T& uid) const  = 0;
			virtual void setMedalCompareFunction(MedalCompareFunction)  = 0;
			virtual void setDisplayNames(const std::vector<uint32_t>&) = 0;
			virtual void getNamesFromServer(const std::vector<uint32_t>&) = 0;
			
			//virtual bool isUserTextableInCh(const xstring &uid, const xstring &chId) = 0;
		};

		struct IMicrophoneListEvent;
		struct IMicrophoneList: public xproto::IListBase
							  , virtual public IConnPoint<IMicrophoneListEvent>
		{
			virtual void joinQueue(void) = 0;
			virtual void leaveQueue() = 0;
			virtual void mute(bool mute) = 0;
			virtual void kickOff(const UID_T &uid) = 0;
			virtual void doubleTime(void) = 0;
			virtual void move(const UID_T &uid, bool down) = 0;
			virtual void disableQueue(void) = 0;
			virtual void allowQueue(void) = 0;
			virtual int	 getTime(void) = 0;
			virtual bool getMute(void) = 0;
			virtual bool getDisable(void) = 0;
			virtual uinfo::SEX getSex(const UID_T& uid) = 0;
			virtual ChannelRoler getRoler(const HULI &hid, const HULI &channelId) = 0;

			virtual void addGuest(const UID_T &uid) = 0; 
			virtual void removeGuest(const UID_T &uid) = 0; 
			virtual void clearGuests() = 0;

			virtual void kickAllOff() = 0;
			virtual void move2top(const UID_T &uid) = 0;
			virtual void tuorenQueue(const UID_T& uid) = 0;
			virtual void onChange(const UID_T& id, const SID_T& p) = 0;

			virtual bool CanJoin() = 0;
			virtual bool CanLeave() = 0;
			virtual bool CanMove(const UID_T& target,bool down) = 0;
			virtual bool CanSay(const UID_T& target) = 0;
			virtual bool CanDoubleTime(const UID_T& target) = 0;
			virtual bool CanTuoRenQueue(const UID_T& target) = 0;
			virtual bool CanTopQueue(const UID_T& target) = 0;

			virtual bool canAddGuest(const UID_T &uid) = 0; 
			virtual bool canRemoveGuest(const UID_T &uid) = 0; 
			virtual bool canClearGuests() = 0;

			
			virtual bool isGuest(const UID_T &uid) = 0;
			virtual const std::set<uint32_t>& getGuests() = 0;
		};

		enum SessionOperate{
			CREATE_SUBCHANNEL,
			DISSMISS_SUBCHANNEL,
			CHANGE_FOLDER,
			UPDATE_SINFO,
			UPDATE_MEMBERROLER,
			CHANGE_STYLE,
			KICKOFF,
			JOIN_SYNC,
			SETKEYACTIVE,
			CHANGE_SERVER
		};


		struct SessOperation
		{	
				enum OpCode
				{
					NONE = 0,
					CREATE_SUBCHANNEL_CHK,		// 创建子频道
					DISSMISS_SUBCHANNEL_CHK,	// 解散子频道
					CHANGE_INFO,				// 更改子频道信息
					ADD_MANAGER,				// 添加管理员
					REMOVE_MANAGER,				// 删除管理员
					ADD_SUB_MANAGER,			// 删除子频道管理员
					REMOVE_SUB_MANAGER,			// 添加子频道管理员
					ADD_MEMBER,					// 添加成员
					REMOVE_MEMBER,				// 删除会员
					JOIN_CHANNEL,				// 加入限制及频道
					JOIN_WITHOUTPASSWD,			// 加入没有密码频道
					KICKOFF,					// 踢人
					VOICE,						// 语音
					TEXT,						// 文字
					CHANGESTYLE,				// 更改模式
					TUOREN,						// 拖人
					SETKEYACTIVE,				// 键盘激活
					DISABLE_VOICE,				// 禁止语音
					ENABLE_VOICE,				// 启用语音
					ENABLE_TEXT,				// 禁止文本
					DISABLE_TEXT,				// 启用文本
					BROADCAST,					// 广播
					DRAGETO_QUEUE,				// 拖进卖序
					JOIN_QUEUE,					// 抢麦	
					LEAVE_QUEUE,				// 退出麦序
					MOVEUP_QUEUE,				// 上移麦序
					MOVEDOWN_QUEUE,				// 下移麦序
					KICK_QUEUE,					// 踢出麦序
					MUTE_QUEUE,					// 禁止抢麦
					UNMUTE_QUEUE,				// 启用抢麦
					DOUBLETIME_QUEUE,			// 双倍麦序时间
					DISABLE_QUEUE,				// 禁用麦序
					ALLOW_QUEUE,				// 启用麦序
					DRAG_PEOPLE,				// 拖人
					DROP_PEOPLE,				// 放人

					EDIT_CHANNEL_INFO,			// 编辑频道信息，chenzhou 2008/4/8
					SHOW_AS_MANAGER,			// 是否显示在管理员列表
					DISABLE_ALL_TEXT,
					ENABLE_ALL_TEXT,
					SETALLKEYACTIVE,
					ADD_VIP,
					REMOVE_VIP,
					ADD_TMPVIP,
					REMOVE_TMPVIP,
					ADD_BAN_ID,
					REMOVE_BAN_ID,
					ADD_BAN_IP,
					REMOVE_BAN_IP,

					MOVETOP_QUEUE,
					CLEAR_QUEUE,
					IGNORE_TEXT,
					IGNORE_VOICE,
					QUIT_GUILD,				//退出工会
					DRAG_ROOM,				//拖频道
					DROP_ROOM,				//放频道
					DIRECT_KICKOFF,
					EDIT_SESSION_INFO,			//改變伺服器一般属性
					MEMBER_TEXT,
					EDIT_SESSION,				//改變伺服器
					UPDATE_MEMBER_CARD,
					CHANGE_SESS_ORDER,		//修改頻道排序

					ADD_QUEUE_GUEST,			// 增加臨時連麥用戶
					REMVOE_QUEUE_GUEST,		// 刪除臨時連麥用戶
					CLEAR_QUEUE_GUEST		// 清除臨時連麥用戶
				};
				enum RetCode
				{
					R_NONE = 0,
					R_INNER_ERROR = 1,           //內部錯誤,不用處理
					R_VOICE_CHAIR_MAN_UNABLE = 2,	//指揮模式,非管理員不能說話
					R_VOICE_ORDER_UNABLE = 3,		//排麥模式,只有第一位能說話
					R_VOICE_USER_UNABLE  = 4,		//用戶不能說話
					R_VOICE_CHANNEL_UNABLE = 5,		//此頻道被設置為不能說話
					R_VOICE_MEMBER_UNABLE = 6,		//遊客不能說話
					R_VOICE_CONTROL_UNABLE = 7, // 排麥情況下,管理員控麥

					R_USER_QUEUN_UNABLE = 8,	// 因為被禁止語音,不能排麥
					R_VISITOR_QUEUN_UNABLE = 9, // 遊客不能排麥
					R_PROHIBIT_QUEUN_UNABLE = 10 // 禁止排麥下,所有人不能排麥
					

				};

				SessOperation(){ retCode = R_NONE;}
				SessOperation(OpCode opcode)
				{
						retCode = R_NONE;
						this->opcode = opcode;
				}
				SessOperation(OpCode opcode,const HULI& room){
					retCode = R_NONE;
					this->opcode = opcode;
					this->room = room;
				}
				SessOperation(OpCode opcode,const HULI& room, const HULI& user){
						this->opcode = opcode;
						this->room = room;
						this->user = user;
						retCode = R_NONE;
				}
				OpCode opcode;
				UID_T   user;	//被操作的人
				SID_T   room;	//所处的频道
				RetCode retCode;
		};
		struct SessDropOperation : public  SessOperation{
			SessDropOperation(OpCode opcode,const HULI& target,const HULI& room) : SessOperation(opcode,target){
				this->room = room;
			}
			HULI	room;
		};


		struct IAuthChecker{
				virtual bool canDo(SessOperation* operation) = 0;
		};



		struct IMicrophoneListEvent : public xproto::IXObject{
			virtual void onSync(void) = 0;
			virtual void onAllow(const UID_T &manipulator) = 0;
			virtual void onDisable(const UID_T &manipulator) = 0;
			virtual void onKickOffQueue(const UID_T &manipulator,const UID_T &uid) = 0;
			virtual void onDoubleTimeQueue(const UID_T &manipulator,const UID_T &uid, int sec) = 0;
			virtual void onTurn(const UID_T& uid, int sec) = 0;
			virtual void onTimeout(const UID_T& uid) = 0;
			virtual void onLeave(const UID_T& uid) = 0;
			virtual void onDrag(const UID_T &uid,const UID_T& to) = 0;
			virtual void onMute(const UID_T &uid,bool mute) = 0;
			virtual void onClearQueue(const UID_T &manipulator) =0;
			virtual void onMove(const UID_T &manipulator,const UID_T &uid, bool down) = 0;
			virtual void onMove2top(const UID_T &manipulator,const UID_T &uid) = 0;
			
			virtual void onGuestAdd(const UID_T &admin, const UID_T &uid) = 0;  // 仅作事件通知
			virtual void onGuestRemove(const UID_T &admin, const UID_T &uid) = 0;  // 仅作事件通知
			virtual void onGuestClear(const UID_T &admin) = 0;  // 仅作事件通知
			virtual void onGuestSync() = 0;
		};

		

		struct ISessionEvent : public xproto::IXObject
		{
			enum AnnounceType
			{
				EMERGENCY = 0,
				ANNOUNCEMENT,
				AD,
				NOTICE,
				WARNING,
				ERR,
				RESTART
			};
			virtual void onPosChange(const SID_T &from) = 0;
			virtual void onStyleChange(const UID_T& uid, bool isNewChannel) = 0;
			virtual void onRolerChange(const UID_T& op, const UID_T& user, const SID_T& sid, ChannelRoler newRoler, ChannelRoler oldRoler) = 0;
			virtual void onKickOff(const UID_T &who, const SID_T &fromCh, const SID_T &toCh, const UID_T &admin, const xstring &reason, uint32_t sec) = 0;
			virtual void onRingBusy() = 0;
			//target is you
			virtual void onSetKeyActive(const UID_T & manipulator) = 0;
			virtual void onNotifySession(AnnounceType type, const xstring &msg) = 0;
			//xxx bool
			virtual void onDisableVoice(const SID_T &channel, const UID_T &manipulator, const UID_T &uid, bool bDisable) = 0;
			//xxx bool
			virtual void onDisableText(const SID_T &channel, const UID_T &manipulator, const UID_T& uid, bool bDisable) = 0;
			virtual void onTuoren(const UID_T &manipulator,const SID_T &from,const SID_T &to) = 0;
			virtual void onSetRoomKeyActive(const UID_T &manipulator) = 0;
			virtual void onDisableAllText(const UID_T &manipulator, const SID_T &channelId, bool bDisable) = 0;
			virtual void onTxtLimit(const UID_T &manipulator, const SID_T &channelId, bool bLimit) = 0;
			virtual void onReceptionChange(const SID_T &from, const SID_T &to) = 0;
			virtual void onNavigateChange(const SID_T &cid) = 0;
		};

		struct ILeaveWatcher{
			virtual void onLeave(const SID_T &sid) = 0;
		};

		

		struct VoiceStatisticsData{
			int	 total;					//该轮统计的总数
			int	 no_init;				//没有初始化的数量
			int	 loss;					//丢包总数	
			int	 error;					//出错总数
			int	 first;					//正常，该用户真接通过服务器广播获得数据
			int	 resend_succ;			//该用户通过请求服务器重传获得数据
			int	 resend_late;			//该用户通过请求服务器重传获得数据, 但该数据已经过期
			int	 resend_error;			//失败，该用户企图通过请求服务器重传获得数据, 但该数据没有得到重传
			int	 resend_excess;			//该用户企图通过请求服务器重传获得数据, 但该数据在重传前，广播包已经到达
			int	 succ;					//成功，通过广播或重传得到了数据
			int  speak_loss_maybe;		//语音广播源丢包（语音源客户端）
			int  emptys;				//补空包
			int  gets;					//得到服务器的数据通知，包含语音数据或空包
			int	 total_percentage;
			int	 no_init_percentage;
			int	 loss_percentage;
			int	 error_percentage;
			int	 first_percentage;
			int	 resend_succ_percentage;
			int	 resend_late_percentage;
			int	 resend_error_percentage;
			int	 resend_excess_percentage;
			int	 succ_percentage;
			int	 gets_percentage;
		};

		struct IVoiceStatus{
			virtual void getImmediateStatus(std::list<VoiceStatisticsData>* status) = 0;             //获取语音即时状态
		};

		struct ISessionPluginManager;

		struct ISession: public virtual IConnPoint<ISessionEvent>
					   , public virtual IConnPoint<xproto::IActionEvent>
					   , public virtual IConnPoint<IChatWatch>
					   , public virtual IConnPoint<IMemberChatWatch>
					   , public virtual IConnPoint<INoticeWatch>
					   , public xproto::IXObject
					   , public IVoiceStatus
		{
			enum ChatResult{
				SENDED,
				TEXT_OVERFLOW,
				VOICE_OVERFLOW
			};

			enum ChatType
			{
				TEXT,
				VOICE
			};

			enum NoticeType
			{
				allUser = 1,
				allMember = 2,
				allAdmin = 3,
				onlyexp = 4,
				onlyexp_sess = 4,
				onlyexp_auto = 5,
				onlyexp_link = 6
			};
			//获取UI显示的SID
			virtual HULI getSid() = 0; 
			virtual HULI getCur() = 0;
			virtual bool setInitChannel(uint32_t channelId) = 0;

			virtual void setReception(SID_T channelId) = 0;
			virtual void setHall(SID_T channelId) = 0;

			virtual SID_T getHall() = 0;
			virtual SID_T getReception() = 0;

						
			virtual ISessionMemberList *getSessionList() = 0;
			virtual ISessionPluginManager *getSessionPluginManager() = 0;
			virtual IChannelInfo *			getChannelInfo(const SID_T &sid) = 0;
			virtual ISessionInfo *			getSessionInfo() = 0;
			virtual IMicrophoneList *getMicrophoneList() = 0;
			virtual IAuthChecker *getAuthChecker() = 0;
			virtual void changeStyle(ChannelStyle s) = 0;
			virtual ChannelStyle getCurrentStyle() = 0;
			




			virtual void createSubChannel(const SessionCreateInfo& info) = 0;
			virtual void dismissSubChannel(const HULI &sid) = 0;
			
			virtual int say(const TextChat &text) = 0;
			virtual int say(const std::string &voice, uint32_t seq) = 0;
			virtual int say(const std::string &voice, uint32_t seq, uint8_t type) = 0;
			virtual int say(const std::string &voice, uint32_t seq, uint32_t qc) = 0;

			//virtual void sayTo(const UID_T& uid, ChatType type) = 0;

			//virtual xstring getToUser(ChatType type) = 0;

			virtual void leave() = 0;

			virtual void invite(const UID_T &uid) = 0;
			virtual void accept() = 0;
			virtual xstring getLastError() const = 0;

			virtual void stopVoice(bool broadcast) = 0;

			virtual void guangboVoice(const std::string &voice, const HULI &chId, uint32_t seq) = 0;
			virtual void guangboText(const TextChat &text, const HULI &chId) = 0;


			virtual int calcNetLatence(int timeStamp) = 0;

			virtual uint32_t getPing(void) = 0;

			virtual IWBoard* getWBoard()	= 0;
			virtual void	 restart()		= 0;

			virtual void	ChangeStatus(const xstring& status) = 0;

			virtual void	changeOrder(const std::vector<UID_T>& order)=0;
			virtual int		sayInMember(const TextChat &text) = 0;
			virtual void	notice(NoticeType notice_type, const xstring& exp, uint32_t type, uint32_t resources , const xstring& info) = 0;
			virtual void	getKickOffList(const HULI &ch) = 0;
			virtual void	removeKickOffList(const UID_T &uid, const HULI &sid) = 0;
			virtual XVoiceData*	fetchVoices(VoiceType vt=using_chaninfo) = 0;
			virtual void 	releaseVoice(XVoiceData*) = 0;
			virtual XVoiceData*	FetchBroadcastVoices() = 0;  //// Use fetchVoices(using_broadcast) instead in the lastest version
			virtual void	BroadcastsEvent(uint32_t sid, uint32_t code, const xstring& str) = 0;

			virtual void	initVoice(bool init) = 0;

			virtual void setSegmentInPack(uint8_t s) = 0;
			virtual void setDefaultPackTime(double t) = 0;

			//异步获取群公告等资料 在打开频道资料查看公告时候需要调用
			virtual void GetChannelExtendInfo(uint32_t channel_id) = 0;
		};

		struct ISessionPlugEvent : public xproto::IXObject
		{
			virtual void onData(uint32_t pi, const  std::string& data) = 0;
			virtual void onPlugChange(uint32_t pi, bool add) = 0;
		};

		struct ISessionPlugBase : public xproto::IXObject
						 , virtual public IConnPoint<ISessionPlugEvent>
		{
			virtual void send(const std::string& data) = 0;
		};

		struct ISessionPluginManager : public xproto::IXObject
		{
			virtual ISessionPlugBase* getPlugIn(uint32_t type) = 0;
			virtual bool isPlugInOpen(uint32_t type) const = 0;
		};
		

	}
}
