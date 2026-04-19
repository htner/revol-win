#ifndef __TIMER_AGENT_H__
#define __TIMER_AGENT_H__

#include "nsoxenv.h"

namespace nsox
{

template <class T>
class timer_handler_agent : public nsox::timer_handler
{
public:
		timer_handler_agent()
		{
				__to	= 0;
				__tid	= -1;
				callback = NULL;
		}
		void start(int timeout)
		{
				if(callback == NULL){
						nsox::xlog(NsoxInfo, "timer callback not set");
				}
				if(__tid != -1){
						stop();
				}
				__to = timeout;
				__tid = nsox::env_timer_inst->set_timer(this, __to);
		}
		void stop()
		{
				if(__tid != -1)
				{
						nsox::env_timer_inst->kill_timer(__tid);
						__tid = -1;
				}			
				__to  = 0;
		}

		virtual void timeout()
		{
				if(callback->timeout() && __to)
				{
						__tid = nsox::env_timer_inst->set_timer(this, __to);
				}else{
						__tid = -1;
				}
		}
		T* callback;
		uint32_t __to;
		nsox::timer_id __tid;
};


template <typename T>
struct bind_func
{
		typedef bool (T::*callback)();
};


template<typename T, typename bind_func<T>::callback ptr>
class timer_bind_handler : public nsox::timer_handler
{
public:
		timer_bind_handler()
		{
				__to		= 0;
				__tid		= -1;
				__cb		= NULL;
		}
		void init(T* cb){
				__cb = cb;
		}
		void start(int timeout)
		{
				if(__tid != -1){
						stop();
				}
				__to = timeout;
				__tid = nsox::env_timer_inst->set_timer(this, __to);
		}
		void stop()
		{
				if(__tid != -1)
				{
						nsox::env_timer_inst->kill_timer(__tid);
						__tid = -1;
				}			
				__to  = 0;
		}

		virtual void timeout()
		{
				if((__cb->*ptr)() && __to)
				{
						__tid = nsox::env_timer_inst->set_timer(this, __to);
				}else{
						__tid = -1;
				}
		}
protected:
		T*				__cb;
		uint32_t		__to;
		nsox::timer_id  __tid;
};
}
#endif