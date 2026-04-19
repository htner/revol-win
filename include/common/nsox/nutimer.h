
#ifndef __nsox_nutimer_h
#define __nsox_nutimer_h

#include <algorithm>
#include <vector>
#include "sockcomm.h"
#include "selector.h"
#include "nuautoptr.h"
#include "nulog.h"


nsox_namespace_begin



#define TVN_BITS (8)
#define TVN_SIZE (1 << TVN_BITS)
#define TVN_MASK (TVN_SIZE - 1)

#define MAKE_TIMER_ID(slot, id)  ((uint32_t)(((BYTE)((DWORD_PTR)(id) & 0xffffff)) | ((uint32_t)((BYTE)((DWORD_PTR)(slot) & 0xff))) << 24))

#define TIMER_FD(id)	((uint32_t)((DWORD_PTR)(id) & 0xffffff))
#define TIMER_SLOT(id)    ((uint32_t)((DWORD_PTR)(id) >> 24))

class nutimer : public timer_selector
{
protected:

	typedef nu_auto_ptr<timer_handler>  timer_auto_ptr;
	struct timer_item
	{
			timer_item();
			timer_item(timer_handler* h):handler(h){}

			timer_auto_ptr handler;
			timer_id	   fd;
	};



	typedef std::vector<timer_item>		timer_list;
	typedef timer_list::iterator		timer_list_itr;

	static bool timer_compare(const timer_item& a, const timer_item& b)
	{
		return a.fd < b.fd;
	}


public:
	nutimer(uint32_t tick_span = 50)
	{
			__timer_fd		= 1;
			__tick_span		= tick_span;
			__elapse_tick	= 0;
			__leavings_span = 0;
	}
	~nutimer(void){

	}

public:

	timer_id add_timeout(uint32_t timespan, timer_handler* handler)
	{
			return __add_timeout(timespan, handler);
	}
	void	del_timeout(timer_id id)
	{
			return __del_timeout(id);
	}
	uint32_t		tickaway(uint32_t elapse)
	{
			elapse += __leavings_span;
			__leavings_span = elapse % __tick_span;
			__tickaway(elapse/__tick_span);

			return 0;
	}

public: // override timer_selector
		virtual timer_id set_timer(timer_handler* handler, timer_span tspan){
					return add_timeout(tspan,handler);
		}
		virtual void	 kill_timer(timer_id tid)
		{
					del_timeout(tid);
		}
		virtual void	 tick(uint32_t elapse)
		{
				tickaway(elapse);
		}


protected:
	timer_list_itr locate(timer_list& tlist, uint32_t fd)
	{
			static timer_item compare_item;
			compare_item.fd = fd;
			return std::lower_bound(tlist.begin(), tlist.end(), compare_item, timer_compare);
	}


	timer_id __add_timeout(uint32_t timespan, timer_handler* handler) // Ìí¼Ó¶¨Ê±Æ÷
	{
			uint32_t slots = timespan/__tick_span ;
			if(!slots)  {
					slots ++;
			}
			timer_item item(handler);
			if(slots < TVN_SIZE )
			{
					uint32_t slot = (slots + __elapse_tick) & TVN_MASK;

					timer_list& tlist = __timers[slot];
					tlist.push_back(item);

					return MAKE_TIMER_ID(slot, tlist.size()-1);
			}else{
					nsox::xlog(NsoxDebug, "timespan too large %u, MAX:%u", timespan, __tick_span*TVN_SIZE -1);
					return 0;
			}
	}
	void __del_timeout(timer_id id)
	{
			uint32_t slot  = TIMER_SLOT(id);
			uint32_t fd	  = TIMER_FD(id);
			if(slot<TVN_SIZE){
					timer_list& tlist = __timers[slot];
					if(fd < tlist.size()){
							tlist[fd] = NULL;
					}else{
							nsox::xlog(NsoxDebug, "error timer id , id:%u", id);
					}
			}
	}

	void	__tickaway(uint32_t ticks)
	{

			for(uint32_t i=0; i<ticks; ++ i)
			{
					__elapse_tick = (++__elapse_tick) & TVN_MASK;

					timer_list& list = __timers[__elapse_tick];
					for(timer_list_itr itr = list.begin(); itr != list.end(); ++ itr)
					{
							if(itr->handler) itr->handler->timeout();
					}
					list.clear();
			}

	}
protected:
	timer_list				__timers[TVN_SIZE];

	uint32_t						__tick_span;
	uint32_t						__elapse_tick;
	uint32_t						__timer_fd;		// maintain ids, BUGBUG, how to recycle id?
	uint32_t						__leavings_span; // last tick leavings time
};


nsox_namespace_end
#endif

