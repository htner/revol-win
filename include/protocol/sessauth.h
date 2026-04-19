#pragma once
#ifndef __INC_SESSION_ROLER_H
#define __INC_SESSION_ROLER_H

#include "common/core/request.h"
#include "protocol/psessionbase.h"
#include "protocol/authcode.h"
#include "common/xstring.h"

namespace protocol {
	namespace session {
		struct clonable
		{
				virtual clonable*   clone() = 0;
				virtual void		 destroy() = 0;
				virtual ~clonable(){};
		};

	/*	struct user_sess_auth : public protocol::session::session_auth
							  , public clonable
		{			
			// 检查权限是否可用
				virtual bool check(session_auth::auth_code auth) = 0;
				virtual ~user_sess_auth(){};
		};
		struct session_role : public clonable
		{
				virtual user_sess_auth* get_authority()  = 0;
				virtual	int				get_priority()	 = 0;
				virtual const char*			get_name()		 = 0;
				virtual ~session_role(){};
		};*/

		struct sess_auth_checker{
				virtual ~sess_auth_checker(){}
				virtual bool check(ChannelRoler from, ChannelRoler to, session_auth::auth_code ac) = 0;
		};

		struct client_role_source {
				virtual ChannelRoler	get_user_role(uint32_t, const xstring& sub_sess_id)	= 0;
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
