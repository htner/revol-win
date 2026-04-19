#pragma once

#include <algorithm>
#include <vector>
#include <map>


namespace nsox{


struct xx_timer_handler
{
		virtual void timeout() = 0;
};

#define XX_TVN_BITS (8)
#define XX_TVN_SIZE (1 << XX_TVN_BITS)
#define XX_TVN_MASK (XX_TVN_SIZE - 1)

#define XX_MAKE_TIMER_ID(slot, id)  ((int)(((BYTE)((DWORD_PTR)(id) & 0xffffff)) | ((int)((BYTE)((DWORD_PTR)(slot) & 0xff))) << 24))

#define XX_TIMER_FD(id)	((int)((DWORD_PTR)(id) & 0xffffff))
#define XX_TIMER_SLOT(id)    ((WORD)((DWORD_PTR)(id) >> 24))


typedef int extra_timer_span;
typedef int extra_timer_id;



class xxtimer
{
protected:
	struct timer_item
	{
			timer_item(){};
			timer_item(xx_timer_handler* h):handler(h){}

			xx_timer_handler* handler;
			extra_timer_id	   fd;
	};


	typedef std::vector<timer_item>		timer_container;
	
	struct xcontainer : public timer_container
	{
		xcontainer(){
			_bRunning = false;
		}
		bool _bRunning;
	};

	//typedef std::vector<timer_item>		timer_list;
	typedef xcontainer					timer_list;
	typedef timer_list::iterator		timer_list_itr;				
	typedef timer_list::iterator		timer_pos;
	typedef std::map<int, timer_pos>	timer_pos_map;

	static bool timer_compare(const timer_item& a, const timer_item& b)
	{
		return a.fd < b.fd;
	}

	
public:
	xxtimer(int tick_span = 50)
	{
			__XX_TIMER_FD		= 1;
			__tick_span		= tick_span;
			__elapse_tick	= 0;
			__ticking		= false;// 正在ticking
	}
	~xxtimer(void){
			
	}

public:

	extra_timer_span add_timeout(int timespan, xx_timer_handler* handler)
	{
			return __add_timeout(timespan, handler);
	}
	void	del_timeout(extra_timer_span id)
	{
			return __del_timeout(id);
	}
	int		tickaway(int elapse)
	{						
			__tickaway(elapse/__tick_span);
			return elapse % __tick_span;
	}

	void	notify_all()
	{
				__tickaway(XX_TVN_SIZE);
	}

	
protected:
	timer_list_itr locate(timer_list& tlist, int fd)
	{
			__compItem.fd = fd;
			return std::lower_bound(tlist.begin(), tlist.end(), __compItem, timer_compare);
	}


	extra_timer_span __add_timeout(int timespan, xx_timer_handler* handler) // 添加定时器
	{			
			int slots = timespan/__tick_span ;
			if(!slots)  {
					slots ++;
			}
			timer_item item(handler);
			if(slots < XX_TVN_SIZE )
			{
					int slot = (slots + __elapse_tick) & XX_TVN_MASK;
					item.fd  = __XX_TIMER_FD ++;

					timer_list& tlist = __timers[slot];
					ATLASSERT( tlist._bRunning == false );
					if( !tlist._bRunning )
					{
						timer_list_itr itr = locate(tlist, item.fd);
						tlist.insert(itr, item);

						return XX_MAKE_TIMER_ID(slot, item.fd);
					}
			}
			return 0;
	}
	void __del_timeout(extra_timer_span id)
	{
			int slot  = XX_TIMER_SLOT(id);
			int fd	  = XX_TIMER_FD(id);

			timer_list& tlist = __timers[slot];
			timer_list_itr itr = locate(tlist, fd);
			if(itr != tlist.end() && itr->fd == fd )
			{
					itr->handler = NULL;
			}
	}

	void	__tickaway(int ticks)
	{
			ATLASSERT(__ticking == false);
			__ticking = true;

			for(int i=0; i<ticks; ++ i)
			{
				__elapse_tick = (++__elapse_tick) & XX_TVN_MASK;
				
				timer_list& list = __timers[__elapse_tick];
#if 0				
				for(timer_list_itr itr = list.begin(); itr != list.end(); ++ itr)
				{
					if(itr->handler)itr->handler->timeout();								
				}
				list.clear(); 
#else
				ATLASSERT( list._bRunning == false );
				if( !list._bRunning )
				{
					list._bRunning = true;
					for(timer_list_itr itr = list.begin(); itr != list.end(); ++ itr)
					{
						if(itr->handler)itr->handler->timeout();								
					}
					list._bRunning = false;
					list.clear();
				}
#endif
			}
			__ticking = false;
	}
protected:
	timer_list				__timers[XX_TVN_SIZE];

	int						__tick_span;
	int						__elapse_tick;
	int						__XX_TIMER_FD;		// maintain ids, BUGBUG, how to recycle id
	boolean					__ticking;
	timer_item				__compItem;
};

}
