#pragma once

#include "infobase.h"
#include "proto/islist.h"
#include "sessionliteinfo.h"
#include "proto/isession.h"
#include "protocol/psession.h"
#include "common/core/request.h"
#include "memMisc.h"
namespace protocol
{
	namespace session{

		class SessionImp;

		

		class SessionInfo : public ISessionInfo
						  , public SessionInfoBase
						  , public XConnPoint<ISessMgrWatcher>
						  , public XConnPoint<IBanWatcher>
						  , public XConnPoint<IKickOffWatcher>
		{
			OBJECT_LEAK_CHECK(SessionInfo);
			typedef std::vector<SKickData,TempAllocator(SKickData) > KickDataVector;
			typedef std::map<UID_T, KickDataVector,std::less<UID_T>,PairAllocator(UID_T,KickDataVector) > kickoff_map_t;			
			typedef std::map<UID_T, MiniInfo, std::less<UID_T>,PairAllocator(UID_T,MiniInfo) > MemberMap;
			typedef std::vector<PBanUserMini,TempAllocator(PBanUserMini) > BanIdVecotr;
			typedef std::vector<PBanIpMini,TempAllocator(PBanIpMini) > BanIpMinVector;
			typedef std::map<UID_T, PListMini, std::less<UID_T>,PairAllocator(UID_T,PListMini) > KickOffMap;

			SessionImp *pSession;
			MemberMap members;
			BanIdVecotr banIds;
			BanIpMinVector banIps;
			kickoff_map_t kickOffs;
			KickOffMap kickOffInfos;
			bool					isSess;
			bool					get_extend_;
		public:
			SessionInfo();
			~SessionInfo();
			void	setSession(SessionImp* sess);

			virtual void setName(const xstring &n);
			virtual void setPub(bool p);
			virtual void setLimit(bool l);
			virtual void setBulletin(const xstring &hello);
			virtual void setMicTime(int t);
			virtual void setMemberLimited(uint32_t ml);
			virtual void setPass(const std::string &md5Pass);
			virtual void setArea(int n);
			virtual void setProvince(int n);
			virtual void setCity(int n);
			virtual void SetIsp(ISPType ntype);

			virtual void setTxtInterval(int n);
			virtual void setTxtInitWait(int n);
			virtual void setTxtMaxLength(int n);
			virtual void setTxtLimitFormat(TxtFormatLimit tfl);
			virtual void setMicLimitQueue(bool b);
			virtual void setMicLimitFree(bool b);
			virtual void setApplyInfo(const xstring &info);
			virtual void setApplyUjifen(int jifen);
			virtual void setApplyUsjifen(int jifen);
			virtual void setSearchByIDLimit(bool b);
			virtual void setMemberListLimit(bool b);
			virtual void setJoinJifenLimit(int n);
			virtual void setChannelDisable(bool b);
			virtual void setAutoApply(int n);
			virtual void setChangeQuality(bool b);
			virtual void setHall(int hall);	
			virtual void setReception(int reception);

			virtual void setLables(const xstring& lables);
			virtual void setSlogan(const xstring& slogan);

			virtual void setFlashUrl(const xstring& url);
			virtual void setAccessProperty(CAccessProperty);

			virtual void setNavigation(const xstring& na);

			virtual void setShowFlower(bool);	


			virtual void setOrder(const xstring& order);

			virtual std::vector<MiniInfo> getManager() const;
			//virtual std::vector<MiniInfo> getMember() const;

			virtual void changeMemberRight(const HULI &uid, ChannelRoler nr);
			virtual HULI getAliaseId() const;
			virtual void setType(GuildType type);
			virtual void setTypeString(const xstring &ts);
			virtual void setIntroduce(const xstring& intro);

			void setOperator(const UID_T &uid);
			void update();
			void onInfoChange();
			virtual void Watch(ISessMgrWatcher *w);
            virtual void Watch(IBanWatcher* w);

			void onGetManagers(POnSyncManagers *sm);
			void onRolerAdd(const UID_T &uid, uint32_t ch, ChannelRoler r, const xstring &nick, protocol::uinfo::SEX gender);
			void onRolerRemove(const UID_T &uid, uint32_t ch);
			void onInfoChanged(POnChannelInfoUpdated *res, uint32_t resCode, core::Request &req);
			void onRolerChanged(POnMemberUpdated * cmd, uint32_t resCode, core::Request &req);
			void onRolerChange(const UID_T &uid, uint32_t ch, ChannelRoler r, const xstring &nick, protocol::uinfo::SEX gender);
			void refreshManager(POnMemberUpdated *upd);
			void refreshManager();
			void updateManagerErr(uint32_t resCode);
			virtual std::vector<BanUserMiniInfo> getBanIDs() const;
			virtual std::vector<BanIpMiniInfo> getBanIPs() const;
			void  getKickOffs(const SID_T &sid, std::vector<SKickData>& kicks,  std::map<UID_T, MiniInfo>& kinfos) const;

			virtual void unbanID(const UID_T &uid);
			virtual void unbanIP(uint32_t ip);
			virtual void batchUnbanID(const std::vector<UID_T> &uids);
			virtual void batchUnbanIP(const std::vector<uint32_t> &ips);

			void updateBans(std::vector<PBanUserMini> &ids, std::vector<PBanIpMini> &ips);

			void banUser(const HULI &uid, bool isBan, const ACCOUNT_T& account);
			void banIp(uint32_t ip, bool isBan);

			void batchBanUser(const std::vector<HULI> &uid, bool isBan);
			void batchBanIp(const std::vector<uint32_t> &ip, bool isBan);

			virtual void setVoiceQC(VoiceQC qc);
			virtual void setStyle(ChannelStyle style);

			void removeKickOff(const UID_T &uid, const SID_T &sid);
			void addKickOff(const UID_T &uid, const SID_T &sid);
			void updateKickOffs(const HULI& sid, std::vector<PListMini> &ids, PUpdateKickOffRes::kuser_map_t&);
			virtual void Watch(IKickOffWatcher* w);

			void setProperty(sox::Properties &p);

			void setIsSess(bool b);
			bool getIsSess() const;

			void setOpenPlatformInfo(xproto::OpenPlatformInfo);
			void removeOpenPlatformInfo(xproto::OpenPlatformInfo);

			bool GetExtend();
			void SetExtend(bool);
		private:
			uint32_t innerGetSid() const;
		};

}}

