#include "selector_sel.h"
#include "nuassert.h"
#include "nu_except.h"
#include "nulog.h"
#include <algorithm>
#include "nsoxenv.h"
nsox_namespace_begin

inline abstract_selector::abstract_selector(void):__quiting(false)
{
		__time_elapse = 0;
}

inline abstract_selector::~abstract_selector(void)
{
}
inline void abstract_selector::advise(event_handler* handler, int ev)
{
		if(!handler) return;
		__action_handlers.push_back(action_item(handler,sel_add,ev));		
}
inline void abstract_selector::unadvise(event_handler* handler, int ev)
{
		if(!handler) return;
		__action_handlers.push_back(action_item(handler,sel_del,ev));		
}
inline void	abstract_selector::clear()
{
		__handlers.clear();
		__action_handlers.clear();
}

inline void abstract_selector::stop()
{
		__quiting = true;
		nsox::xlog(NsoxInfo, "abstract_selector begin to stop");
}

inline void abstract_selector::begin_loop()
{
		while(__action_handlers.size())
		{
				action_item& item = __action_handlers.front();
				if(item.action == sel_add){
						std::pair<select_map::iterator, bool> p = __handlers.insert(select_map::value_type(item.hsocket, event_item()));
						p.first->second.ev		|=  item.event;
						p.first->second.handler = item.handler;
						p.first->second.hsocket = item.hsocket;

						if(p.second)	on_new_handler(p.first->second);// handler insert
						else			on_event_change(p.first->second);// event change
				}else{
						select_map::iterator itr = __handlers.find(item.hsocket);
						if(itr != __handlers.end())
						{
								itr->second.ev &= ~item.event;
								if(!itr->second.ev){
										on_del_handler(itr->second); // handler remove
										__handlers.erase(itr);								
								}else{
										on_event_change(itr->second);// event change
								}
						}
				}				
				__action_handlers.pop_front();
		}		

		if(__handlers.empty()) {
				nsox::xlog(NsoxInfo, "no handlers for selector");
				//stop();
		}
}

inline void abstract_selector::end_loop()
{
		nu_time_t now = SocketUtil::get_now_time();
		env::inst()->timer()->tick(now - __time_elapse);
		__time_elapse = now;
}


inline void abstract_selector::run(nu_time_t timeout)
{	
		__time_elapse	= SocketUtil::get_now_time();
		__quiting		= false;

		nsox::xlog(NsoxInfo, "abstract_selector begin to run");
}

nsox_namespace_end

