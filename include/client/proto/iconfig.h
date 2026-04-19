#ifndef IConfig_Protocol_H
#define IConfig_Protocol_H
#include <string>
#include "iproto.h"
#include <vector>
#include <map>

//struct RECT;
//struct POINT;

#define SYSTEM_USER L"@system"

namespace xproto
{
	struct IConfigEvent : public xproto::IXObject
	{
		virtual void	onConfigChanged() = 0;
	};
	struct IConfig : public IXObject
				   , public virtual IConnPoint<IConfigEvent>
	{
		virtual int		saveConfig() = 0;
		virtual int		getInt(int tag, int def) = 0;
		virtual void	setInt(int tag, int	 value) = 0;
		virtual void	getStr(int tag, xstring & out) = 0;
		virtual void	setStr(int tag, const xstring & str) = 0;
		// 接口没有提供默认值的，在使用前需要自己初始化
		// RECT
		virtual void getRect(int tag, RECT & rect) = 0;
		virtual void setRect(int tag, const RECT & rect) = 0;
		// POINT
		virtual void getPoint(int tag, POINT & point) = 0;
		virtual void setPoint(int tag, const POINT & point) = 0;
		// 二进制
		virtual std::string getBin(int tag, const std::string & _default) = 0;
		virtual void setBin(int tag, const std::string & bin) = 0;

		virtual bool getList(int tag, std::vector<xstring>& strlist)		= 0;
		virtual bool setList(int tag, const std::vector<xstring>& strlist)	= 0;

		
		virtual bool getMap(int tag, std::map<xstring,std::string>& map)	= 0;
		virtual bool setMap(int tag, std::map<xstring,std::string>& map)	= 0;

		virtual void release() = 0;
};

struct IConfigManager : public IProtoObject{
	virtual IConfig* getConfig(const xstring &uid) = 0;
	virtual void	 delConfig(const xstring& uid)	= 0;
	virtual void	 setPath(const xstring& path) = 0;
};
}
#endif
