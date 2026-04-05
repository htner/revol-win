#include "sessionliteinfo.h"
#include "protocol/pkg_tag.h"

#include "OpenPlatformInfo.h"

using namespace xproto;
using namespace protocol::session;

SessionInfoBase::SessionInfoBase()
{
}

SessionInfoBase::~SessionInfoBase(void)
{
}

xstring SessionInfoBase::getName() const{
	return getString(SIT_NAME);
}
SID_T SessionInfoBase::getSid() const{
	return getInt(SIT_SID);
}
SID_T SessionInfoBase::getPid() const{
	return getInt(SIT_PID);
}

bool SessionInfoBase::isPub() const{
	return getBool(SIT_ISPUB);
}

bool SessionInfoBase::isLimit() const{
	return getBool(SIT_ISLIMIT);
}

SID_T SessionInfoBase::getAliaseId() const{
	SID_T mid = getInt(SIT_ALIASESID);
	return mid ? mid : getSid();
}

xstring SessionInfoBase::getBulletin(void) const{
	return getString(SIT_BULLETIN);
}

UID_T SessionInfoBase::getCreator(void) const{
	return getInt(SIT_OWNER);
}

int  SessionInfoBase::getMemberLimited(void) const{
	return getInt(SIT_MEMBERLIMIT);
}

int SessionInfoBase::getArea(void) const{
	return getInt(UIT_AREA);
}

int SessionInfoBase::getProvince(void) const{
	return getInt(UIT_PROVINCE);
}

int SessionInfoBase::getCity(void) const{
	return getInt(UIT_CITY);
}

GuildType SessionInfoBase::getType() const{
	return (GuildType)getInt(SIT_TYPE);
}

ChannelStyle SessionInfoBase::getStyle() const
{
	return (ChannelStyle)getInt(SIT_STYLE);
}

xstring SessionInfoBase::getTypeString() const{
	return getString(SIT_TYPESTRING);
}

uint32_t SessionInfoBase::getPaiming() const{
	return getInt(SIT_PAIMING);
}

time_t SessionInfoBase::getBulletinTimeStamp() const{
	return (time_t)getInt(SIT_BULLETINTIMESTAMP);
}

xstring SessionInfoBase::getIntroduce() const{
	return getString(SIT_INTRODUCE);
}

int	SessionInfoBase::getMicTime() const{
	return getInt(SIT_MICROTIME);
}

bool SessionInfoBase::hasPass(void) const{
	return getBool(SIT_BHASPASSWD);
}

VoiceQC SessionInfoBase::getVoiceQC() const{
	return (VoiceQC)getInt(SIT_VOICEQC);
}

std::vector<BanUserMiniInfo> SessionInfoBase::getBanIDs() const{
	return std::vector<BanUserMiniInfo>();
}
std::vector<BanIpMiniInfo> SessionInfoBase::getBanIPs() const{
	return std::vector<BanIpMiniInfo>();
}

std::vector<MiniInfo> SessionInfoBase::getManager() const{
	return std::vector<MiniInfo>();
}

//std::vector<MiniInfo> SessionInfoBase::getMember() const{
//	return std::vector<MiniInfo>();
//}

uint64_t SessionInfoBase::getJifen() const{
	return getInt64(SIT_JIFEN);
}

ISPType SessionInfoBase::getIsp() const{
	return (ISPType)getInt(SIT_ISPTYPE);

}



/************************************************************************/
/*                                                                      */
/************************************************************************/

SessionListInfo::SessionListInfo(void)
{
}

SessionListInfo::~SessionListInfo(void)
{
}

void SessionListInfo::setProperty(sox::Properties &p){
	sox::properties::replace(__getter, p);
	forEachWatcher0(&ISInfoEvent::onChange);
}

void SessionListInfo::initSid(uint32_t sid, uint32_t asid){
	sox::properties::setint(__getter, SIT_SID, sid);
	sox::properties::setint(__getter, SIT_ALIASESID, asid);
}


void  SessionInfoBase::getKickOffs(const SID_T &sid, std::vector<SKickData>& kicks,  std::map<UID_T, MiniInfo>& kinfos) const
{
}

TxtFormatLimit SessionInfoBase::getTxtLimitFormat()  const
{
	return (TxtFormatLimit)getInt(SIT_VTXTFORMAT);
}


int SessionInfoBase::getTxtInterval() const
{
	return getInt(SIT_VTXTINTERVAL);
}

int SessionInfoBase::getTxtInitWait() const
{
	return getInt(SIT_VTXTINITALWAIT);
}

int SessionInfoBase::getTxtMaxLength() const
{
	return getInt(SIT_VTXTLENGTH);
}


bool SessionInfoBase::isMicLimitFree()  const
{
	return getBool(SIT_VMICLIMITFREE);
}
bool SessionInfoBase::isMicLimitQueue()  const
{
	return getBool(SIT_VMICLIMITQUEUE);
}

SessGrade SessionInfoBase::getGrade()  const
{
	return (SessGrade)getInt(SIT_GRADE);
}

xstring SessionInfoBase::getApplyInfo()  const
{
	return getString(SIT_APPLYINFO);
}

int SessionInfoBase::getApplyUjifen()  const
{
	return getInt(SIT_APPLYUJIFEN);
}

int SessionInfoBase::getApplyUsjifen()  const
{
	return getInt(SIT_APPLYUSJIFEN);
}
bool SessionInfoBase::isSearchByIDLimit()  const
{
	return getBool(SIT_SEARCHLIMIT);
}

bool SessionInfoBase::isMemberListLimit()  const
{
	return getBool(SIT_VMLISTLIMIT);
}

int SessionInfoBase::getJoinJifenLimit() const 
{
	return getInt(SIT_VJOINJIFEN);
}


bool SessionInfoBase::isChannelDisable() const
{
	return getBool(SIT_DISABLE);
}

int SessionInfoBase::getAutoApply() const
{
	return getInt(SIT_APPLYAUTO);
}

bool SessionInfoBase::isChangeQuality() const 
{
	return getBool(SIT_CHANGEQUALITY);
}

xstring SessionInfoBase::getOrder() const
{
	return getString(SIT_ORDER);
}

uint32_t SessionInfoBase::getHall() const
{
	return getInt(SIT_HALL);
}

uint32_t SessionInfoBase::getReception() const 
{
	return getInt(SIT_RECEPTION);
}

uint32_t SessionInfoBase::getMemberCount() const
{
	//·Ç¼´•r”µ“þ
	return getInt(SIT_MEMBERCOUNT);
}

uint32_t SessionInfoBase::getOnlineCount() const
{
	//·Ç¼´•r”µ“þ
	return getInt(SIT_ONLINECOUNT);
}

uint32_t SessionInfoBase::getMaxMember() const 
{
	return 5000;
	//return getInt(SIT_MAXMEMBER);
}

uint32_t SessionInfoBase::getCollectionCount() const {
	return getInt(SIT_COLLECTIONCOUNT);
}

xstring SessionInfoBase::getLables()  const
{
	return getString(SIT_LABLES);
}


xstring SessionInfoBase::getSlogan()  const
{
	return getString(SIT_SLOGAN);
}

xstring SessionInfoBase::getCreateTime() const
{
	return getString(SIT_CREATETIME);
}

uint32_t SessionInfoBase::getFondCount() const
{
	return getInt(SIT_FONDCOUNT);
}

xstring  SessionInfoBase::getFlashUrl() const
{
	return getString(SIT_FLASHURL);
}

CAccessProperty SessionInfoBase::getAccessProperty() const
{
	if (getBool(SIT_ISPUB) == false) {
		return e_private;
	}
	if (getBool(SIT_ISLIMIT) == true) {
		return e_protected;
	}
	return e_public;
}


xstring	SessionInfoBase::getNavigation() const					//Œ§º½
{
	return getString(SIT_NAVIGATION);
}


xstring	SessionInfoBase::getOpenPlugin() const
{
	return getString(SIT_OPENPLUGIN);
}

bool SessionInfoBase::isShowFlower() const
{
	return !getBool(SIT_SHOWFLOWER) || !getBool(SIT_NAVIGATION);
}

const xproto::OpenPlatformInfoVector& SessionInfoBase::getOpenPlatformInfo() {
	if (__open_platform_infos.empty()) {
		OpenPlatformInfoManager::Instance()->GetSessionOpenPlatformInfo(getSid());
	}
	return __open_platform_infos;
}

void SessionInfoBase::AddPlatformInfo(xproto::OpenPlatformInfoVector info) {
	__open_platform_infos = info;
	forEachWatcher0(&ISInfoEvent::onOpenPlatformRefresh);
}