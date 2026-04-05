#pragma once

#include "infobase.h"
#include "proto/islist.h"

namespace protocol
{
	namespace session{

		#define BIT_QUEUELIMIT 0xfffffffe
		#define BIT_DISCULIMIT 0xfffffffd

		#define BIT_SEARCHBYIDLIMIT 0xfffffffe
		#define BIT_MEMBERLISTLIMIT 0xfffffffd

		class SessionInfoBase :	public virtual ISessionInfoBase,
								public xproto::InfoBase,
								public XConnPoint<ISInfoEvent>
		{
		public:
			SessionInfoBase();
			virtual ~SessionInfoBase(void);

			virtual SID_T getSid() const;							//頻道ID
			virtual SID_T getPid() const;							//頻道父ID
			virtual SID_T getAliaseId() const;						//短位ID
			virtual UID_T getCreator(void) const;					//創建者

			virtual bool hasPass(void) const;						//是否有密码
			virtual bool isPub() const;								//是否設為公共頻道
			virtual bool isLimit() const;							//是否限制游客進入
			virtual bool isMicLimitQueue() const;					//排麦模式游客限制
			virtual bool isMicLimitFree() const;					//自由模式游客限制

			virtual bool isSearchByIDLimit() const;					//私人頻道
			virtual bool isMemberListLimit() const;					//
			virtual bool isChannelDisable() const;					//沒有人能進入
			
			virtual bool isChangeQuality() const;

			virtual int getArea(void) const;						//國家或地區
			virtual int getProvince(void) const;					//省、州
			virtual int getCity(void) const;						//城市
			virtual int getMicTime() const;							//麦序时长

			virtual int getMemberLimited(void) const;			//人數上限
			
			virtual int getTxtInterval() const;					//发言间隔
			virtual int getTxtInitWait() const;					//发言初始等待
			virtual int getTxtMaxLength() const;				//最大长度
			virtual int getJoinJifenLimit() const;				//會員積分限制
			virtual int getAutoApply() const;					//申請自動處理
			virtual int getApplyUjifen() const;					//申請積分限制
			virtual int getApplyUsjifen() const;				//申請貢獻值限制

			
			virtual xstring getName() const;						//頻道名
			virtual xstring getBulletin(void) const;				//公告
			
			virtual xstring	getTypeString() const;					//類型描述
			virtual xstring getApplyInfo() const;					//申請信息

			virtual time_t				getBulletinTimeStamp() const;	//公告修改時間
			virtual session::GuildType	getType() const;				//類型
			virtual ChannelStyle		getStyle() const;				//频道模式（排麦，自由，指挥）
			virtual session::VoiceQC	getVoiceQC() const;				//音质
			virtual TxtFormatLimit		getTxtLimitFormat() const;		//文字格式限制

			virtual std::vector<BanUserMiniInfo>	getBanIDs() const;			//封锁ID的人
			virtual std::vector<BanIpMiniInfo>	getBanIPs() const;			//封锁的IP

			virtual std::vector<MiniInfo>	getManager() const;			//管理员
			//virtual std::vector<MiniInfo>	getMember() const;			//伺服器人

			virtual ISPType getIsp() const;								//ISP所在
			virtual uint64_t getJifen()  const;							//積分	
			virtual uint32_t getPaiming() const;						//排名
			virtual xstring getIntroduce() const;						//群簡介
			virtual SessGrade getGrade() const;							//频道等级
			virtual xstring getOrder() const;							//order的原始样式
			virtual uint32_t getHall() const;							//大厅
			
			virtual uint32_t getReception() const;						//接待頻道
			virtual uint32_t getMemberCount() const;					//會員人數
			virtual uint32_t getOnlineCount() const;					//在線人數
			virtual uint32_t getMaxMember() const;						//最高會員數
			virtual uint32_t getCollectionCount() const;				//收藏数量(非即時數據)

			virtual xstring getLables()  const;
			virtual xstring getSlogan()  const;							//簽名
			virtual xstring getCreateTime() const;						//創建時間
			virtual uint32_t getFondCount() const;						//喜欢的人数

			virtual xstring getFlashUrl() const;							//flash url
			virtual CAccessProperty getAccessProperty() const;					//訪問控制
			virtual xstring	getNavigation() const;						//導航
			virtual xstring	getOpenPlugin() const;

			virtual bool isShowFlower() const;									//是否顯示花

			virtual void AddPlatformInfo(xproto::OpenPlatformInfoVector);



			
			virtual void  getKickOffs(const SID_T &sid, std::vector<SKickData>& kicks, std::map<UID_T, MiniInfo>& kinfos) const;	
																			//临时被T名單

			virtual const xproto::OpenPlatformInfoVector& getOpenPlatformInfo();

			xproto::OpenPlatformInfoVector __open_platform_infos;
		};


		class SessionListInfo : public SessionInfoBase
		{
		public:
			SessionListInfo(void);
			virtual ~SessionListInfo(void);
			void setProperty(sox::Properties &p);
			void initSid(uint32_t sid, uint32_t asid);

			//uint32_t getLastTime();
			//void getLastTime();
		private:
			uint32_t last_time_;
		};

}
}

