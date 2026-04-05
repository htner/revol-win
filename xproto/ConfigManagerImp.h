#pragma once
#include "iconfigmanager.h"
#include "helper/TServiceManager.h"
#include "proto/proto_obj_name.h"

#include <map>

namespace xproto{
			class ConfigManagerImp :
			public IInnerConfigManager,
			public TServiceManager<IConfig>,
			public TServiceFactory<IConfig>
		{
			std::string own;
			xstring __path;
		public:
			ConfigManagerImp(void);
			~ConfigManagerImp(void);

			DECLARE_PROTO_OBJECT_NAME(CONFIG_MANAGER_OBJ);

			virtual void destroy(IConfig *config);
			virtual IConfig		*getConfig(const xstring& uid);
			virtual int			removeConfig(const xstring& uid);
			virtual void		releaseConfig(IConfig *config);
			virtual int			saveConfig(const xstring &uid, const sox::Properties& config);

			virtual IConfig		*create(const xstring &uid);
			void setOwner(const UID_T& uid){own = uid;};
			virtual void	 delConfig(const xstring& uid);
			virtual void	 setPath(const xstring& path);
		};
}