#pragma once
#include "proto/isession.h"
#include "proto/ismanager.h"
#include "protocol/psessionbase.h"
#include "protocol/plogin.h"
#include "common/iproperty.h"
#include "common/core/ibase.h"
#include "common/core/iclient.h"
#include "protocol/psession.h"
#include "timer/TimerWrap.h"
#include "common/nsox/nuautoptr.h"
#include "common/nsox/proto_cast.h"
#include "common/nsox/nu_request.h"
#include "common/nsox/default_proto_handler.h"
#include "helper/RSAKey.h"
#include "sitem.h"
#include "voice_queue.h"
#include "front_connection.h"

#ifdef WIN32
#include "atlwin.h"
#endif

#include <set>
#include <list>
#include <fstream>

#include "memMisc.h"

#define QUEUE_SIZE 60
#define JSE_TIME 20
#define MAX_TEXT_SEQ_ROMM 1000

class CEncryTcpSocket;
class CWsUdpSocket;

namespace protocol{

	namespace session{
		class MicrophoneListImp;
		class SessionImp;
		class VoiceDataManager;
		class WhiteBoard; 
		class CPlugInManager;
		class SessionInfo;
		class SessionImp;
		class SessionManager;
		class SessionMemberListImp;
		class RulerChecker;


		struct RolerPair{
			ChannelRoler roler;
			SID_T channelId;
		};
	
		struct IVoicePumper {	
			virtual void OnPumpData(const PChatVoice& voice) = 0;
			virtual void OnUserVoiceStatus(XCast<PUserVoiceStatus>&) = 0;
		};

		class IVoiceQuality {
		public:
			virtual uint32_t GetVoiceQuality() = 0;
			virtual uint8_t GetSegmentInPack() = 0;
			virtual double GetDefaultPackTime() = 0;
		};

		enum ReconnectStat{
			INIT_STAT,
			SESS_CONNECTED,
			LITTLE_RECONNECT,
			BIG_RECONNECT,
			CONNECT_ERROR
		};

		class SessionImp :	public protocol::session::ISession,
			public IPacketNotifyer,
			public IVoicePumper,
			public IVoiceQuality,
			public XConnPoint<xproto::IActionEvent>,
			public XConnPoint<ISessionEvent>,
			public XConnPoint<IChatWatch>,
			public XConnPoint<IMemberChatWatch>,
			public XConnPoint<INoticeWatch>
		{
		public:
			OBJECT_LEAK_CHECK(SessionImp);
			typedef std::vector<RolerPair> RolerPairVertor;
			typedef std::map<UID_T, RolerPairVertor, std::less<UID_T>>	UserRolers;
			typedef std::list<int64_t,TempAllocator(int64_t) >	StampList;
			typedef nsox::nu_auto_ptr<SessionInfo> SessionInfoPtr;
			typedef std::map<SID_T, SessionInfoPtr,std::less<UID_T>,PairAllocator(SID_T,SessionInfoPtr) > ChannelInfo;
			typedef std::set<uint32_t,std::less<uint32_t>, TempAllocator(uint32_t) > UserIDSet;

		public: 
			/************************************************
			 *  Base SessionImp
			 */
			DECLARE_REQUEST_MAP
			SessionImp();
			~SessionImp(void);
			void InitData();
			
			

			/************************************************
			 * for SessionManager
			 */
			void Init(uint32_t s, SID_T uid, const xstring& passwd, const xstring& chPasswd, ISessionHandler* h,  uint32_t version);
			void InitByToken(uint32_t s, SID_T uid, const xstring& token, const xstring& chPasswd, ISessionHandler* h,  uint32_t version);
			void SetManager(SessionManager *m);
			void ChangeNick(const std::string &newNick, uinfo::SEX g, const std::string &sign);
			void SetAliasId(SID_T asid);
			void SetSessionId(SID_T asid);
			SID_T GetSessionId() const;
			SID_T GetAliasId() const;
			UID_T GetUid() const;
			bool Start(const std::string &ip, const std::vector<uint16_t> &ports);
			void SetToken(const xstring&);
			void SetRtt(uint32_t);
			void InitReportTimes(); //初始化进群，开始进群
			void SetTime(int index); //更新进群时间


			/************************************************
			 * ISession, 
			 * from ISession interface, using old Naming Style.
			 */
			virtual HULI getSid(); 
			virtual HULI getCur();
			virtual bool setInitChannel(uint32_t channelId);

			virtual void setReception(SID_T channelId);
			virtual void setHall(SID_T channelId);

			virtual SID_T getHall();
			virtual SID_T getReception();

						
			virtual ISessionMemberList *getSessionList();
			virtual ISessionPluginManager *getSessionPluginManager();
			virtual IChannelInfo *			getChannelInfo(const SID_T &sid);
			virtual ISessionInfo *			getSessionInfo();
			virtual IMicrophoneList *getMicrophoneList();
			virtual IAuthChecker *getAuthChecker();
			virtual void changeStyle(ChannelStyle s);
			virtual ChannelStyle getCurrentStyle();

			virtual void createSubChannel(const SessionCreateInfo& info);
			virtual void dismissSubChannel(const HULI &sid);
			
			virtual int say(const TextChat &text);
			virtual int say(const std::string &voice, uint32_t seq);
			virtual int say(const std::string &voice, uint32_t seq, uint8_t type);
			virtual int say(const std::string &voice, uint32_t seq, uint32_t qc);

			virtual void leave();

			virtual void invite(const UID_T &uid) {};
			virtual void accept() {};
			virtual xstring getLastError() const;

			virtual void stopVoice(bool broadcast);

			virtual void guangboVoice(const std::string &voice, const HULI &chId, uint32_t seq);
			virtual void guangboText(const TextChat &text, const HULI &chId);


			virtual int calcNetLatence(int timeStamp){return 0;};

			virtual uint32_t getPing(void);

			virtual IWBoard* getWBoard(){return NULL;};
			virtual void	 restart();

			virtual void	ChangeStatus(const xstring& status);

			virtual void	changeOrder(const std::vector<UID_T>& order);
			virtual int		sayInMember(const TextChat &text);
			virtual void	notice(NoticeType notice_type, const xstring& exp, uint32_t type, uint32_t resources , const xstring& info);
			virtual void	getKickOffList(const HULI &ch);
			virtual void	removeKickOffList(const UID_T &uid, const HULI &sid);
			virtual XVoiceData*	fetchVoices(VoiceType vt=using_chaninfo);
			virtual void 	releaseVoice(XVoiceData*);
			virtual XVoiceData*	FetchBroadcastVoices();  //// Use fetchVoices(using_broadcast) instead in the lastest version
			virtual void	BroadcastsEvent(uint32_t sid, uint32_t code, const xstring& str);

			virtual void	initVoice(bool init);

			virtual void setSegmentInPack(uint8_t s);
			virtual void setDefaultPackTime(double t);

			/************************************************
			 * for connection
			 * connection events
			 */
			bool Login(nsox::nu_auto_ptr<FrontConnection> c);
			bool RegisterUdp(nsox::nu_auto_ptr<FrontConnection> c);

			bool Relogin();
			
			bool UdpClose(nsox::nu_auto_ptr<FrontConnection> );


			/************************************************
			 * IPacketNotifyer
			 * connection events
			 */
			virtual void on_packet_loss(const UID_T& user, int seq);
			virtual void OnPacketLossChangeBad();
			virtual void OnPacketLossTooMuch();


			/************************************************
			 * IVoicePumper
			 * connection events
			 */
			virtual void OnPumpData(const PChatVoice& voice){};
			virtual void OnUserVoiceStatus(XCast<PUserVoiceStatus>&);


			/************************************************
			 * IVoiceQuality
			 * connection events
			 */
			virtual uint32_t GetVoiceQuality();
			virtual uint8_t GetSegmentInPack();
			virtual double GetDefaultPackTime();

			/************************************************
			 * IVoiceStatus
			 * connection events
			 */
			virtual void getImmediateStatus(std::list<VoiceStatisticsData>* status);             //获取语音即时状态





		public:
			/************************************************
			 * 登陆成功后的操作
			 * （1）需要后台自动更新的资料
			 * （2）连接到其他前端
			 */
			void UpdateMemberList();		//獲取會員列表  
			void GetSessionBaseInfo();		//获取基本资料
			bool GetBackUpFronts(int count);			//获取其他前端的信息
			void UpdateVoiceToken();



			/************************************************
			 * 向外部提供一些基本的资料，有点奇怪 (xproto内部）
			 */
			CRSAKey* GetRsaKey();
			bool IsLimit();
			ChannelRoler GetMaxRoler(const UID_T &uid);
			ChannelRoler GetRoler(const UID_T &uid, const SID_T &channelId);
			ChannelRoler GetChannelRoler(const UID_T &uid, const SID_T &channelId);
			bool IsChannelTextable(SID_T ch);
			bool IsChannelVoiceable(SID_T ch);

			bool IsVip(UID_T uid);
			bool IsTVip(UID_T uid);
			void IgnoreText(UID_T uid, bool bi);
			void IgnoreVoice(UID_T uid, bool bi);

			bool IsIgnoreText(UID_T uid);
			bool IsIgnoreVoice(UID_T uid);

			bool IsRecurePid(const SID_T& subid, const SID_T& pid);	
			VoiceQC GetUserChannelQC(const UID_T& uid);
			uint32_t GetServerVserion();




			/************************************************
			 * 向外部提供向服务器请求的接口 (xproto内部）
			 */
			void UpdateChannelInfoRequest(SID_T sid, sox::Properties &prop); //更新频道资料请求
			void UpdateSessionInfoRequest(sox::Properties &prop); //更新群资料请求
			void StopVoiceRequest(uint32_t sid);

			//void UpdateChannelInfo(uint32_t cid, sox::Properties props);

			/************************************************
			 * 外部更改成员变量的接口 (xproto内部）
			 */
			void UpdateChannelInfo(uint32_t cid, sox::Properties props);
			void UpdateSessInfo( sox::Properties &prop);
			void UpdateLastChannelPasswd(const std::string& channel_passwd_);
			void ResetVoiceManager();
			void CurrectChannelChanged(SID_T from , SID_T to);
			void AddRoler(UID_T uid, std::vector<MRolerAndCh> &rolers);
			void ReplaceRoler(SID_T uid, const SID_T &strSid, ChannelRoler nr);
			void RemoveRoler(const SID_T& uid,  const SID_T &channelId);
			

			
			//void SyncUStatus(); 同步用户的状态，暂时不做


			/************************************************
			 * 通知watcher，告诉它们内部资料已经更改 (xproto内部）
			 */
			void UpdateSList();
			void ChannelInfoKickOffAdded(const UID_T& uid, const SID_T sid); // 更新到channelinfo 的kickoff list

			void NotifyResult(int op, int res);						// 通知客户端操作结果, 一般在失败时使用
			void NotifyMemberChange(const UID_T& uid);
			void NotifyEnableAllText(PEnableAllText *et);
			void NotifyTuoren(uint32_t admin, SID_T from, SID_T to);
			void NotifyLoginRescode(int err);						//通知客户端登陆的结果
			void NotifySessionStatus(ISessionHandler::Status st);	//通知道客户端服务器的状态
			void NotifySessionLbsError(ISessionHandler::Status st);

			bool ReSessionLbs();

			virtual void GetChannelExtendInfo(uint32_t channel_id);
	

			/************************************************
			 * 发包器
			 */
			template<typename T>
			void SendNprotoRequest(T& req){
				conn_manager_->Send(req);
			};

			template<typename T>
			void DirectSendRequest(nproto::connection* c, T& req){
				nproto::write_request(c, &req, T::uri);
				//c->Send(req, T::uri)
			};

			void onVoice(XCast<PChatVoice>& cmd, nproto::connection* c);

		private:
			bool CanSay();
			void OnPacketLoss(const UID_T &user, int seq);
			int CheckSeqStatus(uint32_t seq);
			void ReseqTextSeq();

		protected:

			/************************************************
			 * 消息处理函数
			 */
			
			void onText(XCast<PChatText>& cmd, nproto::connection* c);
			void onSeqText(XCast<PSeqChatText>& cmd, nproto::connection* c);
			void onBroadcastText(XCast<PBroadcastText>& cmd, nproto::connection* c);
			void onBroadcastVoice(XCast<PBroadcastVoice>& cmd, nproto::connection* c);
			
			void onExpandTypeVoice(XCast<PExpandTypeVoice>& cmd, nproto::connection* c);
			void onExpandQcVoice(XCast<PExpandQcVoice>& cmd, nproto::connection* c);
			void onReSendVoices(XCast<PReSendVoiceRes>& cmd, nproto::connection* c);
			void onUdpConnected(XCast<PUdpLoginRes>& cmd, nproto::connection* c);
			void OnVoiceRes(XCast<POnVoiceRes>& cmd, nproto::connection* c);

			void onReSendTypeVoices(XCast<PReSendTypeVoiceRes>& cmd, nproto::connection* c);
			void onReSendQcVoices(XCast<PReSendQcVoiceRes>& cmd, nproto::connection* c);
			void onReSendBcVoices(XCast<PReSendBcVoiceRes>& cmd, nproto::connection* c);

			void onChannelInfoChanged(XCast<POnChannelInfoUpdated>& cmd, nproto::connection* c);
			void onRolerChanged(XCast<POnMemberUpdated>& cmd, nproto::connection* c);
			void onRingBusy(XCast<PRingBusy>& cmd, nproto::connection* c);

			void onSetKeyActive(XCast<PSetKeyActive>& cmd, nproto::connection* c);
			void onSetRoomKeyActive(XCast<PSetRoomKeyActive>& cmd, nproto::connection* c);
			void onKickOff(XCast<POnSessKickOff>& cmd, nproto::connection* c);

			void onSyncManager(XCast<POnSyncManagers>& cmd, nproto::connection* c);
			//void onPing(XCast<login::PPlus>& cmd, nproto::connection* c);
			//void onTcpPing(XCast<PTcpPingRes>& cmd, nproto::connection* c);

			void onStop(XCast<PStopVoice>& cmd, nproto::connection* c);
			void onGetBans(XCast<POnGetBans>& cmd, nproto::connection* c);

			void onBanUser(XCast<PUpdateBanIDRes>& cmd, nproto::connection* c);
			void onBanIp(XCast<PUpdateBanIpRes>& cmd, nproto::connection* c);

			void onBatchBanUser(XCast<PBatchUpdateBanIDRes>& cmd, nproto::connection* c);
			void onBatchBanIp(XCast<PBatchUpdateBanIpRes>& cmd, nproto::connection* c);

			void onNotifySession(XCast<PNotifySession> &cmd, nproto::connection* c);
			void onDisableVoice(XCast<PDisableVoice> &cmd, nproto::connection* c);
			void onDisableText(XCast<PDisableText> &cmd, nproto::connection* c);

			void onStatusChanged(XCast<PStatusChanged>& cmd, nproto::connection* c);
			void onSyncUStatus(XCast<PSyncUStatusRes>& cmd, nproto::connection* c);

			void onNickChange(XCast<PPInfoChanged>& cmd, nproto::connection* c);
			void onChangeFolder(XCast<PChangeFolder>& cmd, nproto::connection* c);
			//void onReconnect(XCast<PReconnenct>& cmd, nproto::connection* c);

			void onSync(XCast<PRealJoinChannelRes>& cmd, nproto::connection* c);
			//void onExchangeKey(XCast<login::PExchangeKeyRes>& cmd, nproto::connection* c);
			void onResendVoice(XCast<PReSendVoice>& cmd, nproto::connection* c);

			void onSynOrder(XCast<PSynOrder>& cmd, nproto::connection* c);
			void onTcpConnect(XCast<PTcpConnect>& cmd, nproto::connection* c);

			void onUpdateKickOffList(XCast<PUpdateKickOffRes>& cmd, nproto::connection* c);
			void onMemberText(XCast<PMemberText>& cmd, nproto::connection* c);
			void onMemberHistory(XCast<PZMemberHistory>& cmd, nproto::connection* c);

			void onMemberList(XCast<PZMemberList>& cmd, nproto::connection* c);
			void  onNotice(XCast<PNotice>& cmd, nproto::connection* c);
			void onRemoveKickOff(XCast<PRemoveKickOffRes>& cmd, nproto::connection* c);

			void onZSync(XCast<PZRealJoinChannelRes>& cmd, nproto::connection* c);
			void _onSync(PRealJoinChannelRes* cmd, nproto::connection* c );

			void onBroadCasts(XCast<PBroadcasts>& cmd, nproto::connection* c);

			void onUpdateSessInfo(XCast<PUpdateSessInfoRes>& cmd, nproto::connection* c);
			void onSessionLastError(XCast<PSessionLastError>& cmd, nproto::connection* c);

			void onMemberCards(XCast<PZMemberCards>& cmd, nproto::connection* c);
			//void OnGetMemberCards(XCast<PMemberCards>& cmd, nproto::connection* c);

			void onBroadCastEvent(XCast<PBroadcastEvent>& cmd, nproto::connection* c);

			void onEmptyVoices(XCast<PEmptyChatVoice>& cmd, nproto::connection* c);

			void onMutedSelf(XCast<PMutedSelfRes> &cmd, nproto::connection* c);

			void OnChangeUserInfo(XCast<PChangePInfo>&, nproto::connection* c);

			//void onJoinSessionRes(XCast<PRealJoinSessionRes>&, nproto::connection* c);
			//void onSyncSession(XCast<PSyncSessionRes>&, nproto::connection* c);

			
			//void OnGetVoiceTokenRes(XCast<PGetVoiceTokenRes>&, nproto::connection* c);

			void  OnJoinSesionRes(XCast<PJoinSessionRes>&, nproto::connection* c);
			void  OnGetSessionMainRes(XCast<PGetSessionMainRes>&, nproto::connection* c);
			void  OnGetSessionAuthRes(XCast<PGetSessionAuthRes>&, nproto::connection* c);
			void  OnGetDisableVoiceRes(XCast<PGetDisableVoiceRes>&, nproto::connection* c);
			void  OnGetDisableTextRes(XCast<PGetDisableTextRes>&, nproto::connection* c);
			void  OnGetUserRolesRes(XCast<PGetUserRolesRes>&, nproto::connection* c);
			void  OnGetUserInfoRes(XCast<PGetUserInfoRes>&, nproto::connection* c);
			void  OnGetSeqListRes(XCast<PGetSeqListRes>&, nproto::connection* c);
	

			//void OnGetExtendChannelInfoRes(XCast<PGetExtendChannelInfoRes>&, nproto::connection* c);
			void OnGetBackUpProtocolFrontsRes(XCast<PGetBackUpProtocolFrontsRes>& cmd, nproto::connection* c);

			//void OnGetStreamAddr(XCast<protocol::sstream::GetStreamAddrResPacket>& cmd, nproto::connection* c);



		protected:

		private:		
			/************************************************
			 * sessionlbs allocate ipv4 addrs
			 */
			uint16_t port_;
			std::string ip_;

			/************************************************
			 * 當前用戶、客户端信息
			 */
			UID_T uid_;				
			xstring user_password_;			// 用户的密码
			xstring user_token_;			// 用户的token

			xstring user_key_;				// 暂时没用
			uint32_t client_version_;
			xstring last_error_;
			static CRSAKey rsakey_;
			uint32_t client_id_;			// 作为客户端多连接认证码
			uint32_t text_sequence_;		// 文字聊天的序列号，防止重发
			uint32_t server_version_;		//
			uint32_t times_[JSE_TIME];  //
			bool is_relogin_; //是否为重新登陆
			bool is_onvaild_; //是否通知过
			uint32_t voice_no_res_;


			/************************************************
			 * 當前频道信息
			 */
			SID_T alias_id_;						// 短號或引導號
			SID_T session_id_;						// session號
			SID_T currect_channel_;					// 當前所在的頻道

			
			UserRolers user_rolers_;				// 用戶角色
			UserIDSet text_ignores_;				//	被當前用戶忽略文字的人
			UserIDSet voice_ignores_;				//	被當前用戶忽略語音的人
			SessionInfoPtr session_info_;			// 当前伺服器信息
			ChannelInfo channel_infos_;				//頻道信息

			std::string last_input_channel_password_;

			


			/************************************************
			 * 语音包相关信息
			 */
			uint8_t voice_packer_segments_;					// 每個語音包中包含的語音段數
			double default_packet_duration_;				//一般的包播放時長
			OutputVoiceQueue output_voice_queue_;			//(自己产生的语音)输出語音隊列
			VoiceDataManager* voice_mgr_;					//(其他人产生的语音)输入語音管理隊列
			std::set<uint32_t> voice_res_;					// 最近收到的ChatVoice的回包seq队列

			uint32_t sequence_cache_[MAX_TEXT_SEQ_ROMM];

			ISessionHandler* session_handler_;		
			
			RulerChecker* rule_checker_;					//權限檢查
			SessionManager* session_manager_;				//sessionlbs, 与其他非session相关类交互
			FrontConnectionManager* conn_manager_;			//连接管理中心
			//xproto::VoiceFront	voice_front_;
			
			nsox::nu_auto_ptr<CPlugInManager> plugin_manager_;				//插件管理器
			nsox::nu_auto_ptr<MicrophoneListImp> microphone_list_imp_;		//排麥模式時管理隊列
			
			nsox::nu_auto_ptr<SessionMemberListImp> session_member_list_;

			TimerHandler<SessionImp>	voice_token_timer_;

			friend class SessionManager;
			friend class SessionMemberListImp;
			friend class SessionInfo;
			friend class RulerChecker;
			friend class MicrophoneListImp;
			friend class Report;
			friend struct DataQueue;
		};
	}
}
