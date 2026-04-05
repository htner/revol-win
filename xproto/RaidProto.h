#pragma once
#include "proto/iproto.h"
#include "common/nsox/nuautoptr.h"
#include "timer/TimerWrap.h"


namespace xproto
{
	class CProtoLink;
	class CRaidUserManager;
	class ConfigManagerImp;

	class CRaidProto : public IRaidProto
	{
	public:
		CRaidProto(void);
		virtual ~CRaidProto(void);
		virtual IProtoObject* queryObject(LPCSTR objName);

		void	setLogTime();

		void init();

		virtual void set_logger_func(output_function_t func);
		virtual void set_logger_level(int);

	protected:
		nsox::nu_auto_ptr<CProtoLink>			__protoLink;
		nsox::nu_auto_ptr<ConfigManagerImp>		__configMgr;
		TimerHandler<CRaidProto>				__logTimer;
	};
}