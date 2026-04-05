#pragma once
#include <map>
#include "proto/sessauth.h"
#include "proto/isession.h"

#define NUL_UID  _T("")

namespace protocol{
		namespace session{		
				
				class sess_auth_checker_imp : public sess_auth_checker
				{
				public:
						virtual bool check(ChannelRoler from, ChannelRoler to, session_auth::auth_code ac);

						void push(ChannelRoler from, ChannelRoler to, session_auth::auth_code ac);
				protected:
						typedef std::map<session_auth::auth_code, bool>			auth_code_index;
						typedef std::map<ChannelRoler, auth_code_index>			to_auth_index;
						typedef std::map<ChannelRoler, to_auth_index>			from_auth_index;

						from_auth_index __auths;
				};
				class SessionImp;
				class client_role_source_imp  : public client_role_source
				{
				public:
						client_role_source_imp(SessionImp* sess);
						~client_role_source_imp();

						virtual ChannelRoler	get_user_role(const SID_T&, const UID_T& sub_sess_id)	;
				protected:
						SessionImp* __sess;
 			
				};
		}
}
