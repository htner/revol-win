#pragma once
#include "common/xstring.h"
#include "common/iproperty.h"
#include "proto/iconfig.h"

namespace xproto
{
		struct IInnerConfigManager: public IConfigManager{
			virtual void releaseConfig(IConfig *config) = 0;
			virtual int saveConfig(const xstring &uid, const sox::Properties& config) = 0;
		};
}
