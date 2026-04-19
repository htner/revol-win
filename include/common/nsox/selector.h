#ifndef nsox_select_h
#define nsox_select_h
#include "sockcomm.h"
namespace nsox
{



struct IAutoPtr {
	virtual ~IAutoPtr(){};
	virtual void addref() = 0;
	virtual void release() = 0;
};
    
struct event_handler : virtual public IAutoPtr
{
		 virtual ~event_handler(){};
        virtual void		onevent(int ev)				= 0;
        virtual HSOCKET	getsocket()					= 0; 
};   

struct timer_handler : virtual public IAutoPtr
{
		virtual ~timer_handler(){};
		virtual void timeout() = 0;
};

struct timer_selector {
		virtual ~timer_selector(){};
		virtual timer_id set_timer(timer_handler*, timer_span) = 0;
		virtual void	 kill_timer(timer_id tid) = 0;
		virtual void	 tick(uint32_t elapse)		  = 0;	
};



struct event_selector
{
		virtual ~event_selector(){};
		virtual void advise(event_handler* handler, int ev)		= 0;
		virtual void unadvise(event_handler* handler, int ev)	= 0;
		virtual void run(nu_time_t timeout)										= 0;
		virtual void stop()										= 0;
};

}    
#endif


