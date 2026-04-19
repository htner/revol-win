#ifndef IUInfo_Protocol_H
#define IUInfo_Protocol_H
#include "common/int_types.h"
#include "common/packet.h"
#include "common/unistr.h"
#include "const.h"

namespace protocol {

namespace uinfo {
struct IUInfoWatcher;

struct MiniInfo{
	UniString account;//	帐号：邮箱（注册不验证邮箱）长度64的带@字符的字符串
	UniString nick; //昵称：255位
	SEX sex; //性别：男、女
};

struct UInfo : public MiniInfo {
	UniString passwd;//密码 6-32
	long birthday; //生日：年/月/日
	int area;//国家
	int province; //省份
	int city; //城市：序号数字
	bool validate;
	UniString resume;//个人简介：1024位
	long updateTime;//上次登录时间
	uint32_t updateIp;//上次登录IP
};

struct IUInfoWatchBase{
	virtual void watch(IUInfoWatcher *) = 0;
	virtual void revoke(IUInfoWatcher *) = 0;
};

struct IUInfo: public virtual IUInfoWatchBase{
	
	virtual UniString getAccount() const = 0;
	
	virtual UniString getNick() const = 0;
	virtual void setNick(const UniString &) = 0;

	virtual void setPasswd(const UniString &) = 0;

	virtual SEX getSex() const = 0;
	virtual void setSex(SEX) = 0;

	virtual long getBirthday() const = 0;
	virtual void setBirthday(long) = 0;

	virtual int getArea() const = 0;
	virtual void setArea(int) = 0;

	virtual int getProvince() const = 0;
	virtual void setProvince(int) = 0;

	virtual int getCity() const = 0;
	virtual void setCity(int) = 0;

	virtual bool getValidate() const = 0;

	virtual UniString getResume() const = 0;
	virtual void setResume(const UniString &) = 0;

	virtual long getUpdateTime() const = 0;
	virtual uint32_t getUpdateIp() const = 0;

	virtual uint32_t getUserJF() const = 0;

	//主动refresh
	virtual void refresh() = 0;
	virtual void update() = 0;

	virtual void release() = 0;

	virtual UniString getSign() = 0;

	virtual void setSign(const UniString &s) = 0;

	virtual UniString getPassport() const= 0;

	virtual UniString getIntro() const = 0;

	virtual void setIntro(const UniString &intro) = 0; 
};


}
}
#endif
