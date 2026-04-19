#ifndef __nuinit_h__
#define __nuinit_h__
nsox_namespace_begin


template<typename TTimer>
class init_timer
{
public:
		init_timer(int span = 100) : __timer(span)
	  {
			  env::inst()->set_timer_inst(&__timer);
	  }
protected:
      TTimer __timer;
};


template<typename TSelector>
class init_selector
{
public:
		init_selector()
		{
				 env::inst()->set_selector_inst(&__selector);
		}
protected:
		TSelector __selector;
};

nsox_namespace_end
#endif //__nuinit_h__

