#ifndef ISLIST_H_
#define ISLIST_H_
#include <map>
#include <vector>
#include <string>
#include "client/proto/IProto.h"
#include "protocol/psessionbase.h"
#include "protocol/const.h"

namespace protocol
{
	namespace session
	{


		enum SessGrade{
			sl_normal = 0,
			sl_advanced = 1
		};

		enum TxtFormatLimit{
			tfl_nolimit = 0,				// 没有限制
			tfl_urllimit = 1,				//游客不能发送 URL
			tfl_vislimit = 2,				//游客不能说话
			tfl_onlymanager	= 4				//游客与会员不能说话
		};

		enum CAccessProperty{
			e_default = 0,				//私有群
			e_public = 1,				//公開群
			e_protected = 2,			//半公開群
			e_private = 4				//私有群
		};
			

		struct MiniInfo{
			UID_T uid;									//用戶ID
			xstring nick;								//用戶昵稱
			protocol::uinfo::SEX gender;				//性別
			protocol::session::ChannelRoler role;		//角色
			MiniInfo(const UID_T &u, const xstring &n, protocol::uinfo::SEX s):uid(u), nick(n), gender(s){}
			MiniInfo(){role = protocol::session::NORMAL;};
		};
		struct BanUserMiniInfo :public MiniInfo{
			xstring account;
			xstring reason;
		};
		struct BanIpMiniInfo :public BanUserMiniInfo{
			uint32_t ip;
		};

		struct SKickData{
				SID_T sid;								//頻道ID
				UID_T uid;								//用戶ID
				UID_T aid;								//管理員
				uint32_t op_time;						//kickoff時間 
				uint32_t tick;							//時長
				xstring reason;							//原因
		};


		struct ISInfoEvent : public xproto::IXObject{
			virtual void onChange() = 0;				//
			virtual void onOpenPlatformRefresh() = 0;				//
		};


		//頻道信息
		struct ISessionInfoBase: public virtual IConnPoint<ISInfoEvent>	
								, public xproto::IXObject
		{
			enum MEMBER_APPLY_AUTO{
				NO_AUTO = 0,
				AUTO_ACC,
				AUTO_REF
			};

			virtual SID_T getSid()			const = 0;							//頻道ID
			virtual SID_T getPid()			const = 0;							//頻道父ID
			virtual SID_T getAliaseId()		const = 0;							//短位ID
			virtual UID_T getCreator(void)	const = 0;							//創建者

			virtual bool hasPass(void)		const = 0;								//是否有密码
			virtual bool isPub()			const = 0;								//是否設為公共頻道
			virtual bool isLimit()			const = 0;								//是否限制游客進入
			virtual bool isMicLimitQueue()	const = 0;								//排麦模式游客限制

			virtual bool isMicLimitFree()	const = 0;								//自由模式游客限制


			virtual bool isSearchByIDLimit()	const = 0;					//私人頻道
			virtual bool isMemberListLimit()	const = 0;					//
			virtual bool isChannelDisable()		const = 0;					//沒有人能進入
			
			virtual bool isChangeQuality()		const = 0;

			virtual int			getArea(void)		const = 0;					//國家或地區
			virtual int			getProvince(void)	const = 0;					//省、州
			virtual int			getCity(void)		const = 0;					//城市
			virtual int			getMicTime()		const = 0;					//麦序时长
			virtual int getMemberLimited(void)		const = 0;					//人數上限
			
			virtual int getTxtInterval()		const = 0;						//发言间隔
			virtual int getTxtInitWait()		const = 0;						//发言初始等待
			virtual int getTxtMaxLength()		const = 0;						//最大长度
			virtual int getJoinJifenLimit()		const = 0;						//會員積分限制
			virtual int getAutoApply()			const = 0;						//申請自動處理
			virtual int getApplyUjifen()		const = 0;						//申請積分限制
			virtual int getApplyUsjifen()		const = 0;						//申請貢獻值限制


			virtual xstring getName()			const = 0;						//頻道名
			virtual xstring getBulletin(void)	const = 0;						//公告
			
			virtual xstring	getTypeString()		const = 0;					//類型描述
			virtual xstring getApplyInfo()		const = 0;					//申請信息

			virtual time_t				getBulletinTimeStamp()	const = 0;				//公告修改時間
			virtual session::GuildType	getType()				const = 0;				//類型
			virtual ChannelStyle		getStyle()				const = 0;				//频道模式（排麦，自由，指挥）
			virtual session::VoiceQC	getVoiceQC()			const = 0;				//音质
			virtual TxtFormatLimit		getTxtLimitFormat()		const = 0;				//文字格式限制

			virtual std::vector<BanUserMiniInfo>	getBanIDs()			const = 0;				//封锁ID的人
			virtual std::vector<BanIpMiniInfo>	getBanIPs()			const = 0;				//封锁的IP

			virtual std::vector<MiniInfo>	getManager()		const = 0;				//管理员
			//virtual std::vector<MiniInfo>	getMember()			const = 0;				//伺服器人

			virtual ISPType getIsp()			const = 0;								//ISP所在
			virtual uint64_t getJifen()			const = 0;								//積分	
			virtual uint32_t getPaiming()		const = 0;								//排名
			virtual xstring getIntroduce()		const = 0;								//群簡介
			virtual SessGrade getGrade()		const = 0;								//频道等级

			
			virtual void  getKickOffs(const HULI &sid, std::vector<SKickData>& kicks, std::map<UID_T, MiniInfo>& kinfos) const = 0;	
																			//临时被T名單
			virtual uint32_t getHall() const = 0;							//大厅
			virtual uint32_t getReception() const = 0;						//接待頻道
			virtual uint32_t getMemberCount() const = 0;					//會員人數(非即時數據)
			virtual uint32_t getOnlineCount() const = 0;					//當前在線人數(非即時數據)
			virtual uint32_t getMaxMember() const = 0;						//最高會員數

			virtual xstring getLables()  const = 0;
			virtual xstring getSlogan()  const = 0;							//簽名
			virtual xstring getCreateTime() const = 0;						//創建時間
			virtual uint32_t getFondCount() const = 0;						//喜欢的人的总数
			virtual xstring getFlashUrl() const = 0;							//flash url]
			virtual CAccessProperty getAccessProperty() const = 0;					//訪問控制

			virtual xstring	getNavigation() const = 0;						//導航
			virtual xstring	getOpenPlugin() const = 0;						//開通的插件功能

			virtual bool isShowFlower() const = 0;									//是否顯示花的顯示
			virtual const xproto::OpenPlatformInfoVector& getOpenPlatformInfo() = 0;\
			virtual void AddPlatformInfo(xproto::OpenPlatformInfoVector) = 0;



		};

	}
	namespace slist
	{
	
	struct ISessListEvent : public xproto::IXObject
	{
		virtual void onFondResult(const SID_T& sid) = 0;
	};

	//用戶相關頻道列表信息
	struct ISessionList: public xproto::IListBase
					   , virtual public IConnPoint<xproto::IActionEvent>
					   , virtual public IConnPoint<ISessListEvent>
					   , public xproto::IProtoObject
	{
		enum operate{
			ADDFAVOR,
			DELFAVOR,
			ADDFOLDER,
			DELFOLDER,
			SYNC
		};
		//virtual uint32_t  getCount(const HULI& cid) = 0;										//獵取總數
		virtual UID_T	getOwner(const SID_T &fid)							const = 0;			//獲取owner	
		virtual void	addFolder(const SID_T &pid, const xstring &fdNick)	= 0;				//增加類型
		virtual void	delFolder(const SID_T &fid, const SID_T &pid)		= 0;				//刪除類型
		virtual void	addFavor(const SID_T &sid, const SID_T &pid)		= 0;				//增加頻道
		virtual void	removeFavor(const SID_T &fid, const SID_T &pid)		= 0;				//刪除頻道
			
		
		virtual void	sync() = 0;																//同步
		virtual bool	isLimitChannel(const SID_T &cid)				const= 0;				//是否為限制類
		virtual bool	IsExistSess(const SID_T &sid)					const= 0;				//是否存在
		virtual bool	isPublic(const SID_T& hid )						const = 0;				//是否為公開頻道
		virtual xstring  getIntro(const SID_T& cid)						const = 0;				//獲取介紹
		virtual uint32_t	getOnline(const SID_T &fid)					const = 0;				//獲取總人數
		virtual int  getCollect(const HULI &hid) const = 0;										//获取收藏总数
		virtual int  getMember(const HULI &hid) const = 0;										//获取会员总数

		virtual session::ISessionInfoBase *getSessionInfo(const SID_T &sid,bool update_force = false) = 0;	//

		virtual xstring  getLables(const HULI& hid)						const = 0;				//
		virtual xstring  getSlogan(const HULI& hid)						const = 0;				//簽名

		//virtual void checkSub(const HULI $pid, const HULI& sid);
		virtual int isFond(const HULI &sid) = 0;												// // -1等待更新, 0 unFond, 1 Fond
		virtual void setFond(const HULI &sid, bool bFond) = 0;
		virtual void syncSub(const HULI &pid) = 0;
		virtual void updateFonds() = 0;
		virtual void	updateSinfoLikes(const std::vector<uint32_t>& sids) = 0;

	};

	}}




#endif /*ISLIST_H_*/
