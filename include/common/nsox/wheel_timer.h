#pragma once
#ifndef __nsox_wheel_timer_h
#define __nsox_wheel_timer_h

#include "nutimer.h"

nsox_namespace_begin

// don't support time_span more than 255 * tick_span

class wheel_timer : public timer_selector
{
public:
	wheel_timer(void){};
	~wheel_timer(void){};
	virtual timer_id set_timer(timer_handler* handler, timer_span timeout)
	{
			return __nutimer.add_timeout(timeout, handler);
	}
	virtual void	 kill_timer(timer_id tid)
	{
			__nutimer.del_timeout(tid);
	}

protected:
	nutimer __nutimer;
};

nsox_namespace_end

#endif

