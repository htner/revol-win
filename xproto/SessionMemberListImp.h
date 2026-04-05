#pragma once

#include "common/core/ibase.h"
#include "common/core/ilink.h"
#include "proto/isession.h"
#include "proto/iproto.h"
#include "common/nsox/default_proto_handler.h"
#include "common/nsox/nu_request.h"
#include "common/nsox/proto_cast.h"
#include "protocol/psession.h"
#include "timer/TimerWrap.h"
#include "memMisc.h"
#include "client/helper/ustring.h"
#define MAX_REASON_SIZE 1024


namespace protocol{
	namespace session{

		static const LPCWSTR filterDisplayString(LPCWSTR lpstr) {
			static std::wstring str;
			str = lpstr;
			str = helper::ustring::TrimChar(str,_T("\n\t\r"));
			helper::ustring::TrimUnicodeCtrlChar(str);
			return str.c_str();
		}

		struct PeopleItem{
			SID_T pid;
			CxDevice device;
			protocol::uinfo::SEX sex;

			uint32_t user_jifen;  //積分
			uint32_t usess_jifen;	//貢獻值

			localStringW nick;
			localStringW sign;
			localStringW status;
			localStringW client_info;
			VipLevel vip;
			PeopleItem() {
				pid = 0;
				device = dev_unkown;
				vip = vip_null;
			}
			void setClientInfo(LPCWSTR ci) {
				//set device from client_info
				client_info = ci;
				localStringW tmp = client_info;
				transform(tmp.begin(), tmp.end(), tmp.begin(), tolower);
				if (tmp.find(_T("android")) !=  localStringW::npos) {
					device = dev_android;
				} else {
					device = dev_winpc;
				}
			}


			~PeopleItem() {
				pid = 0;
			}
		};

		struct ChannelItem{
		private:
			bool bPasswd;
			SID_T pid;
			localStringW nick;
			
		public:
			SID_T getPid() const{ return pid;}
			LPCWSTR getNick() const{ return nick.c_str();}
			bool hasPasswd() const {return bPasswd;}

			void setPid(const SID_T &p){ pid = p;}
			void setNick(LPCWSTR n){ nick = filterDisplayString(n);}
			void setPasswd(bool b){ bPasswd = b;}

			ChannelItem(const SID_T &p, const localStringW &n, bool bP): pid(p), nick(n), bPasswd(bP){}
			ChannelItem(){}
		};

		struct MemberItem{
			localStringW						nick;
			uint32_t					user_jifen;
			uint32_t					usess_jifen;
			protocol::uinfo::SEX	gender;
			localStringW						sign;
			ChannelRoler			roler;
			uint32_t				addtime;
		};

		struct CachePeopleItem{
			protocol::uinfo::SEX sex;
			localStringW account;
			localStringW nick;
		};

		struct CardItem{
				uint32_t uid;
				localStringW account;
				localStringW cname;
				localStringW tempname;
				uint32_t cidentity;
		};


		class SessionImp;
		class SessionMemberListImp :
			public ISessionMemberList,
			public XConnPoint<xproto::IRoomEvent>,
			public XConnPoint<IMemberListEvent >,
			public XConnPoint<xproto::IActionEvent>,
			public XConnPoint<IMemberCardEvent>,
			public XConnPoint<IMedalEvent>,
			public XConnPoint<IVIPEvent>,
			public XConnPoint<IMemberCreditEvent>,
			public XConnPoint<IUserInfoEvent>,
			public nproto::default_proto_handler
		{
			OBJECT_LEAK_CHECK(SessionMemberListImp);
			typedef std::map<UID_T, PeopleItem,std::less<UID_T>, PairAllocator(UID_T,PeopleItem) > people_doc_t;
			typedef std::map<SID_T, ChannelItem,std::less<UID_T>, PairAllocator(SID_T,ChannelItem) > channal_doc_t;
			typedef std::map<UID_T, MemberItem,std::less<UID_T>, PairAllocator(UID_T,MemberItem) > member_doc_t;
			typedef std::map<UID_T, CachePeopleItem,std::less<UID_T>, PairAllocator(UID_T,CachePeopleItem) > cache_people_doc_t;
			typedef std::map<UID_T, CardItem,std::less<UID_T>, PairAllocator(UID_T,CardItem) > card_doc_t;
			typedef std::map<UID_T, simeffect_t,std::less<UID_T>, PairAllocator(UID_T,simeffect_t) > simeffect_doc_t;
			
		private:
			std::set<uint32_t> fast_sync_uinfo_;
			std::map<uint32_t, uint8_t> uinfo_requests_map_;

			std::set<uint32_t> usjifen_requested_;
			std::set<uint32_t> usjifen_requests_;
			std::map<uint32_t, uint32_t> user_session_jifen_;
			bool						initiative_sync_user_info_; //主动同步uinfo
			bool						selfMuted;  // 是否在服務器請求不需要語音數據
			bool						bSelfAdd;   //自己是否已經到達
			bool						bGetMember;  //是否已經同步會員數
			int							bSyncMemberTime;
			uint32_t					memberCount; //當前會員數,只有列表不存在時才生效
			uint32_t					maxOrder;	//當前最大的樹索引
			uint32_t					last_session_jifen_;
			people_doc_t				people;		//當前的人, 注意不要随意使用 operator[], 防止增加新的people,形成隐身单位
			people_doc_t				get_people_from_servers_;
			channal_doc_t				channels;	//當前頻道
			member_doc_t				members;	//當前的會員
			card_doc_t					memberCards; //群名片			
			simeffect_doc_t				userMedals; //效果

			SessionImp					*pSession;
			
			protocol::session::is_map_t		disablevoice;
			protocol::session::is_map_t		disabletext;

			typedef std::set<SID_T,std::less<SID_T>,TempAllocator(SID_T) > SidSet;
			SidSet					disableTextChs;
			SidSet					disableVoiceChs;
			typedef std::map<SID_T, uint32_t,std::less<SID_T>, PairAllocator(SID_T,uint32_t) > ChannelOrlerMap;
			ChannelOrlerMap		channelOrders;  // add by ivan 
			typedef std::set<localStringW,std::less<localStringW>, TempAllocator(localStringW) > MemberNameSet;
			MemberNameSet				memberNames;

			TimerHandler<SessionMemberListImp>	__effectTimer;
			TimerHandler<SessionMemberListImp>	uinfo_timer_; 
			TimerHandler<SessionMemberListImp>	member_uinfo_timer_; 
			MedalCompareFunction				medal_compare_;
					
		public:
			SessionMemberListImp();
			virtual ~SessionMemberListImp(void);

			DECLARE_REQUEST_MAP

			//Inherit from IRoomBase
			virtual bool isFolder(const xproto::ITreeNode  &hid) const;
			virtual bool isExisit(const xproto::ITreeNode &hid, const xproto::ITreeNode &pid) const;
			virtual void getParent(const xproto::ITreeNode &hid, std::vector<xproto::ITreeNode > &parents) const;
			virtual LPCWSTR getNick(const xproto::ITreeNode &hid) const;
			virtual xproto::ITreeNode getAliaseId(const  xproto::ITreeNode &hid) const;
			virtual xproto::IRoomBase::PlainTree_t getTreeData() const;
			virtual void fillTreeData(IListFiller*,int) const;
			

			virtual void onclose(int err, nproto::connection* c);
			virtual void oncreate(nproto::connection* c);

			

			//session Interface
			virtual void leaveGuild();
			virtual bool isLimit();
			virtual uint32_t getOnlineCount() const; 

			//channel Interface
			virtual uint32_t getOrder(const SID_T &subId) const;
			virtual void enableAllText(const SID_T &channelId, bool bEnable);
			
			virtual bool hasPasswd(const SID_T &subId) const;

			//user Interface
			virtual bool isSameParent(const UID_T &u1,const UID_T &u2) const;
			
			
			virtual uinfo::SEX getSex(const UID_T& uid) const;

			virtual void changeFolder(const SID_T &from, const SID_T &to, const std::string &passwd);
			virtual void changeTo(const SID_T &to, const std::string &passwd);

			virtual ChannelRoler getRoler(const UID_T &hid, const SID_T &channelId) const;

			virtual void changeMemberRight(const UID_T &uid, ChannelRoler nr, const SID_T &channelId);
			virtual void changeMemberRight(const ACCOUNT_T &account, ChannelRoler nr, const SID_T &channelId);

			virtual void kickOff(const SID_T & sid, const UID_T &uid, const xstring& reason, uint32_t minutes);
			virtual void tuoren(const UID_T &who, const SID_T &from, const SID_T &to);

			virtual void enableText(const UID_T &uid, bool bEnable);
			virtual bool isUserTextable(const UID_T &uid) const;

			virtual void enableVoice(const UID_T &uid, bool bEnable);
			virtual bool isUserVoiceable(const UID_T &uid) const;

			virtual void mutedSelf(bool bEnable);
			virtual bool isSelfMuted() const;

			virtual bool canVoiceOpt(const UID_T &who,const UID_T &target) const;
			virtual void pushToMicrophoneList(const UID_T& uid);

			
			virtual void directKickoff(const HULI & sid, const HULI &uid, const xstring& reason, uint32_t minutes);

			virtual IUserGuild *getUserGuilds(const UID_T &uid);

			virtual LPCWSTR getSign(const UID_T &uid) const;
			virtual LPCWSTR getStatus(const UID_T &uid) const;

			virtual bool isUserTextableInCh(const UID_T &uid) const;
			virtual uint32_t getUserJF(const UID_T &uid) const;
			virtual uint32_t getUserSessJF(const UID_T &uid);
			virtual uint32_t getAddTime(const UID_T &uid) const;
			virtual int	getUserSessContribute(const UID_T &uid);

			virtual void banAccount(const ACCOUNT_T &account);
			virtual void banID(const UID_T &uid);
			virtual void banIP(const UID_T &uid);
			virtual void setAllKeyActive(const SID_T &channelId);

			virtual void ignoreText(const UID_T &uid, bool ignore);
			virtual void ignoreVoice(const UID_T &uid, bool ignore) ;
			virtual bool isIgnoreText(const UID_T &uid) const;
			virtual bool isIgnoreVoice(const UID_T &uid) const;

			//member Interface
			virtual void		 getMember(std::vector<UID_T>& ret) const;
			virtual ChannelRoler getMemberRoler(const UID_T& uid) const ;

			virtual	MemberStatus getMemberStatus(const UID_T& uid) const;

			virtual SID_T innerGetChannelPid(const SID_T &pid) const;
			virtual  int	getMemberCount() const;
			virtual void	updateMemberList() const;

			virtual simeffect_const_t& getEffect(const UID_T &uid) const;

			virtual LPCWSTR		getClientInfo(const UID_T &uid) const;

			virtual CxDevice getClientDevice(const UID_T &uid) const;

			virtual VipLevel getVipLevel(const UID_T& uid) const;

			virtual void setMedalCompareFunction(MedalCompareFunction) ;

			virtual void setDisplayNames(const std::vector<uint32_t>&);

			virtual void getNamesFromServer(const std::vector<uint32_t>&);

			virtual void UpdateUserInfo(uint32_t uid);
			
		public:
			//session
			void	setSession(SessionImp* s);
		
			
			void	onSync(std::vector<Partner> &partners, 
						std::vector<SubChannel> &subs, 
						const protocol::session::is_map_t &refbanvoice,
						const protocol::session::is_map_t &refbantext, 
						const std::vector<SID_T> &txtDisabledCh,
						bool bReConnect = false);

			void OnInitUserChannel(const std::vector<std::pair<uint32_t, uint32_t> > &partners);  
			void UpdateDisableVoice(const protocol::session::is_map_t &refbanvoice);
			void UpdateDisableText(const protocol::session::is_map_t &refbantext);
			void UpdateOrder(const std::vector<uint32_t>& orders);
			void UpdateChannelTreeInfo(const std::vector<SubChannel>&);
			void UpdateUserInfos(const std::vector<UserInfo>&);


			void RefreshSessionTree();
			void endSync(bool);

			//channel
			void	setTextAble(const UID_T &uid,bool bAble);
			void	setVoiceAble(const UID_T &uid,bool bAble);
			bool	isChTextable(const SID_T &ch) const;
			bool	isChVoiceable(const SID_T &ch) const;
			void	notifyChannelChange(const SID_T &chId);
	
			//user
			void	infoAffectList(int id, const sox::Properties &prop);
			bool	isOnline(const UID_T &uid) const;

			void	setKeyActive(const UID_T &uid);
			void	onPInfoChange(PPInfoChanged *pp, uint32_t resCode);			
			void	onStatusChange(const UID_T& uid,const xstring& status, uint32_t resCode);
			virtual SID_T	innerGetUserPid(const UID_T &uid) const;
			void	notifyUserChange(const UID_T &uid);

			// member
			void	onSyncMembers(const std::vector<PMemberInfo>& );
			void	addMember(const UID_T &uid, const xstring &nick, const xstring &sign,
								protocol::uinfo::SEX gender, uint32_t user_jifen, uint32_t usjifen, ChannelRoler roler);
			void	removeMember(const UID_T &uid);
			void	syncMemberInfo(const UID_T &uid, ChannelRoler roler, bool notice = true);
			void	removeAllMember();
			void	updateMemberCount(uint32_t c);

			void	onUpdateMemberCard(const std::vector<MemberCard>& memberCards);
			void	onMemberCards(const std::vector<MemberCard>& memberCards);
			void	onUpdateMemberCard(const UID_T&,   const	xstring& cname, uint32_t cidentity);
			void	onRemoveMember(const UID_T&);

			virtual LPCWSTR		getCardNick(const UID_T &hid) const;
			virtual LPCWSTR		getRealNick(const UID_T &hid) const;

			virtual void updateMemberCards(const UID_T&, const xstring &name, uint32_t nlimit);
			virtual bool isSelfUpdateMemberCards(const UID_T&);
			virtual bool isAdminUpdateMemberCards(const UID_T&);
			virtual bool isCardsNameExist(const xstring &name); // return the uid if exist, else return ""

			void onEffectTimeout();
			void remvoeUserEffect(uint32_t uid);
			void setSelfMuted(bool);

			void OnGetUserInfo();
			void OnGetMemberUserInfo();
			void RealAddSubChannel(POnSubChannelAdd*);
			void RealMemberChanges(PPushSessData* cmd, nproto::connection* c);


			

			//packets
			void onMemberAdd(POnMemberAdd* cmd, nproto::connection* c);
			void onChannelAdd(XCast<POnSubChannelAdd>& cmd, nproto::connection* c);
			void onChannelRemove(XCast<POnSubChannelRemove>& cmd, nproto::connection* c);
			void onTuoren(XCast<POnTuoren>& cmd, nproto::connection* c);
			void onMemberChanges(XCast<PPushSessData>& cmd, nproto::connection* c);
			void onEnableAllText(XCast<PEnableAllText>& cmd, nproto::connection* c);
			void onEnableAllVoice(XCast<PEnableAllVoice>& cmd, nproto::connection* c);
			void onUpdateMemberCard(XCast<PUpdateMemberCard>& cmd, nproto::connection* c);
			void onChangeFolder(XCast<PChangeFolder>& cmd, nproto::connection* c);
			void onSyncEffect(XCast<PSyncEffects>& cmd, nproto::connection* c);
			void onAddEffect(XCast<PAddEffects>& cmd, nproto::connection* c);
			void onRemoveEffect(XCast<PRemoveEffect>& cmd, nproto::connection* c);
			void onUpdateClientInfos(XCast<PUpdateClientInfos>& cmd, nproto::connection* c);
			void OnGetUserSessionCreditRes(XCast<PGetUserSessionCreditRes>& cmd, nproto::connection* c);
//			void OnGetSessionMemberCreditRes(XCast<PGetSessionMemberCreditRes>& cmd, nproto::connection* c);

	//		void OnSeqSubChannelAdd(XCast<PSeqOnSubChannelAdd>& cmd, nproto::connection* c);
	//		void OnSeqMemberChanges(XCast<PSeqPushSessData>& cmd, nproto::connection* c);
			
			

			void RefreshMemberList();
			


private:
			SID_T getUserPid();
			void updateChannelOrder(uint32_t sid, uint32_t pre);
			

		};

	}
}

