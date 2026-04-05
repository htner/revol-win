#include "SessRoleSource.h"
#include "assert.h"


/************************************************************************/
/*                                                                      */
/************************************************************************/

using namespace protocol::session;


#include "SessionImp.h"

client_role_source_imp::client_role_source_imp(SessionImp* sess)
{
		__sess = sess;
}

client_role_source_imp::~client_role_source_imp(void)
{
}

ChannelRoler client_role_source_imp::get_user_role(const UID_T& uid, const SID_T& sub_sess_id)
{
		if(uid == UID_NULL) return NUL_ROLE;

		return __sess->GetRoler(uid, sub_sess_id);	
}
/************************************************************************/
/*                                                                      */
/************************************************************************/

bool sess_auth_checker_imp::check(ChannelRoler from, ChannelRoler to, session_auth::auth_code ac)
{
		from_auth_index::iterator fitr = __auths.find(from);
		if(fitr != __auths.end())
		{
				to_auth_index::iterator titr = fitr->second.find(to);
				if(titr != fitr->second.end())
				{
						auth_code_index::iterator aitr = titr->second.find(ac);
						bool r = aitr != titr->second.end();
						return (r) ;
				}
		}
		return false;
}
void sess_auth_checker_imp::push(ChannelRoler from, ChannelRoler to, session_auth::auth_code ac)
{
		from_auth_index::iterator fitr = __auths.find(from);
		if(fitr != __auths.end())
		{
				to_auth_index::iterator titr = fitr->second.find(to);
				if(titr != fitr->second.end())
				{
						auth_code_index::iterator aitr = titr->second.find(ac);
						if(aitr == titr->second.end())
						{
								titr->second[ac] = TRUE;
						}
				}else{
						auth_code_index aci; 
						aci[ac] = TRUE;

						fitr->second[to] = aci;
				}
		}else{
				auth_code_index aci; 
				aci[ac] = TRUE;

				to_auth_index   tai;
				tai[to] = aci;

				__auths[from] = tai;
		}
}