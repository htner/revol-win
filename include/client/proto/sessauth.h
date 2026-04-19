#pragma once
#ifndef __INC_SESSION_ROLER_H
#define __INC_SESSION_ROLER_H

#include "common/core/request.h"
#include "protocol/psessionbase.h"
#include "protocol/authcode.h"
#include "common/xstring.h"
#include "iproto.h"

namespace protocol {
	namespace session {
		struct clonable
		{
				virtual clonable*   clone() = 0;
				virtual void		 destroy() = 0;
				virtual ~clonable(){};
		};

		struct sess_auth_checker{
				virtual ~sess_auth_checker(){}
				virtual bool check(ChannelRoler from, ChannelRoler to, session_auth::auth_code ac) = 0;
		};

		struct client_role_source {
				virtual ChannelRoler	get_user_role(const UID_T& uid, const SID_T& sub_sess_id) = 0;
				virtual ~client_role_source(){};
		};

		struct server_role_source {
				virtual ChannelRoler	get_user_role(uint32_t uid, uint32_t sub_sess_id)	= 0;
				virtual ~server_role_source(){};
		};


		template <typename T> 
		class auto_clone_ptr 
		{
		public:
			auto_clone_ptr()
			{	
				__ptr = NULL;
			}
			~auto_clone_ptr()
			{
				if(__ptr)
					__ptr->destroy();
			}
			T* operator->()
			{
				return __ptr;
			}
			auto_clone_ptr* operator=(T* ptr)
			{
				 if(__ptr) __ptr->destroy();
				 __ptr = (T*) ptr->clone();
				 return this;
			}
			operator T*()
			{
					return __ptr;
			}
		private:
			T* __ptr;
		};

	}//namespace session
}//namespace server
#endif //__INC_SESSION_ROLER_H
