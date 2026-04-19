#ifndef __nsox_env_h
#define __nsox_env_h

#include "nsox.h"
#include "nutimer.h"

nsox_namespace_begin

struct env
{
private:
		env()
		{
				__timer			= NULL;
				__selector		= NULL;
		}
public:
		static env* inst()
		{
				static env instance;
				return &instance;
		}
		timer_selector* timer()
		{
				return __timer;
		}
		void set_timer_inst(timer_selector * timer)
		{
				__timer = timer;
		}
		event_selector* selector()
		{
				return __selector;
		}
		void set_selector_inst(event_selector* selector)
		{
				__selector = selector;
		}

private:
		timer_selector* __timer;
		event_selector* __selector;
};


#define env_timer_inst env::inst()->timer()

#define env_selector_inst env::inst()->selector()




nsox_namespace_end

#endif

