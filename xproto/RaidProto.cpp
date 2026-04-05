#include "raidproto.h"
#include "protolink.h"
#include "proto/proto_obj_name.h"
#include "timer/extratimer.h"
#include "raiduser.h"
#include "sessionmanager.h"
#include "configmanagerimp.h"

using namespace xproto;

CRaidProto::CRaidProto(void)
{
		__protoLink = nsox::nu_create_object<CProtoLink>::create_inst();	
		__configMgr = nsox::nu_create_object<ConfigManagerImp>::create_inst();	
		init();
		
}
CRaidProto::~CRaidProto(void)
{
	
}
void CRaidProto::setLogTime()
{
		nsox::logger::inst()->set_logger_time();
}
void CRaidProto::init()
{
		CTimer::Instance()->SetTick(20);	
		__logTimer.init(this, &CRaidProto::setLogTime);
		__logTimer.start(500);
}
IProtoObject*	CRaidProto::queryObject(LPCSTR objName)
{		
		if(strcmp(PROTO_LINK_OBJ,objName) == 0)
		{
				return __protoLink;
		}
		if(strcmp(USER_MANAGER_OBJ,objName) == 0)
		{
				return __protoLink->getUserMgr();
		}
		if(strcmp(SESSION_LIST_OBJ,objName) == 0)
		{
			return __protoLink->getSessList();
		}
		if(strcmp(SESSION_MANAGER_OBJ, objName) == 0){
				return (IProtoObject*)(__protoLink->__sessManager);
		}
		if(strcmp(CONFIG_MANAGER_OBJ, objName) == 0){
				return 	__configMgr;
		}
		if(strcmp(IM_OBJ, objName) == 0){
				return 	__protoLink->getImBase();
		}
		if(strcmp(PROXY_MANAGER_OBJ, objName) == 0){
			return 	__protoLink->getProxyMgr();
		}
		

		ASSERT(FALSE);
		return NULL;
}

void CRaidProto::set_logger_func(output_function_t func){
			nsox::logger::inst()->set_logger_func(func);
}


void CRaidProto::set_logger_level(int nlevel)
{
	nsox::logger::inst()->set_logger_level(nlevel);
}

namespace xproto
{
	IRaidProto* CoCreateProto()
	{
			static CRaidProto proto;
			return &proto;
	}
}

