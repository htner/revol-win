#ifndef ISESSIONMANAGER_H_
#define ISESSIONMANAGER_H_
#include "common/xstring.h"
#include "protocol/psessionbase.h"
#include "iproto.h"

#include <vector>

namespace protocol{
namespace session{

struct SearchResult{
	SID_T	id;
	xstring name;
	xstring owner;
	bool bLimit;
	bool bPub;
};
struct ISession;
struct TextChat;
struct VoiceChat;

struct ISessionHandler : public xproto::IXObject
{
	enum Status{
		JOIN1,       //正在连接，获取list服务器地址
		JOIN2,		 //通过join的地址，去同步列表 步骤二
		CONNECTED,   //连接成功
		NETBROKEN,   //网络错误
		CLOSING,     //正在关闭
		INVALID,	 //session 打开错误,
		KICKOFF,
		JOIN_KICKOFF,
		RECONNECTING,
		TOOMANYUSR,		//频道满了
		PASSWDERROR,
		NOTMEMBER,     //不是会员，不能进入
		NOTENOUGHJIFEN,    //积分不足，不能进入
		NOTEXIST,                   // session不存在
		ERR_RETYR,                 //內部錯誤，建議重試
		RECONNECTED   //
	};

	virtual void onConnected(ISession *) = 0;
	virtual void onValid(ISession *) = 0;
	virtual void onStatus(Status st, ISession *) = 0;
	virtual void onClose(int reason, ISession *, int err) = 0;
	virtual void onReEnterSucc(ISession *) = 0;
};

struct SessionCreateInfo;
struct ISessMgrEvent : public xproto::IActionEvent
{
	virtual void onCreate(const SessionCreateInfo& info, uint16_t res) = 0;
};

struct IFindEvent : public xproto::IXObject{
	virtual void onResult(int resCode, const std::vector<SearchResult> &res) = 0;
};


struct ISessionManager: public xproto::IProtoObject
					  , public virtual IConnPoint<ISessMgrEvent>
					  , public virtual IConnPoint<IFindEvent>
					  , public virtual IConnPoint<xproto::IActionEvent>
{
	enum Operate{
		CREATE,JOIN,DISMISS,
	};
	//virtual void create(const SessionCreateInfo &)									 = 0;
	virtual ISession *join(const SID_T &sid,const xstring& passwd,ISessionHandler*) = 0;
	virtual ISession *GetJoined() = 0;
	//virtual void dismiss(const SID_T &sid)										 = 0;
	//virtual void findByChannelId(const SID_T &sid)						= 0;
	//virtual void findByChannelName(const xstring &name)					= 0;
	//virtual void findByLocate(int area, int province, int city)			= 0;
	//virtual void findByType(GuildType type, const xstring &typeName)	= 0;
	virtual void changeSign(xstring& sign)								= 0;
};

}
}
#endif /*ISESSIONMANAGER_H_*/
