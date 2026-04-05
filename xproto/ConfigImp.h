#pragma once
#include "proto/iconfig.h"
#include "common/iproperty.h"

namespace xproto{
		struct IInnerConfigManager;

		class ConfigImp : public IConfig
						, public XConnPoint<IConfigEvent>
		{
			sox::Properties			config;
			std::string				uid;
			IInnerConfigManager*	manager;

		public:
			ConfigImp();
			~ConfigImp(void);

			void	SetUid(const std::string& u);
			void	SetManager(IInnerConfigManager* m);

			virtual int saveConfig();
			// 整数
			virtual int getInt(int tag, int _default);
			virtual void setInt(int tag, int value);
			// 字符串
			virtual void getStr(int tag, xstring & out);
			virtual void setStr(int tag, const xstring& str);
			// 接口没有提供默认值的，在使用前需要自己初始化
			// RECT
			virtual void getRect(int tag, RECT & rect);
			virtual void setRect(int tag, const RECT & rect);
			// POINT
			virtual void getPoint(int tag, POINT & point);
			virtual void setPoint(int tag, const POINT & point);
			// 二进制
			virtual std::string getBin(int tag, const std::string & _default);
			virtual void setBin(int tag, const std::string & bin);

			virtual bool getList(int tag, std::vector<xstring>& strlist);
			virtual bool setList(int tag, const std::vector<xstring>& strlist);

			virtual bool getMap(int tag, std::map<xstring,std::string>& map);
			virtual bool setMap(int tag, std::map<xstring,std::string>& map);

			virtual void release();

			void replaceProperty(const sox::Properties &p);
		};
}