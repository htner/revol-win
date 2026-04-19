#pragma once

#ifndef __UNIX_SIGNAL_H__
#define __UNIX_SIGNAL_H__

#define  SIGNAL_MAP_BEGIN(theClass)  \
		static void onsignal(int sigx){ \
				printf("signal happened, %d", sigx);
#define  SIGNAL_HANDLER(sig, func) if(sig == sigx) return func(sigx);
		 
#define  SIGNAL_MAP_END	\
		 printf("signal ignored, %d", sigx);}

#ifdef NSOX_WIN32
		#define  SIGUSR1 10
#endif

namespace nsox{

class unix_signal
{
public:
		SIGNAL_MAP_BEGIN(unix_signal)
		SIGNAL_MAP_END
		static void add(int sig){
#ifndef NSOX_WIN32
			signal(sig, onsignal);
#endif				
		}
		static void remove(int sig){
				assert(false);
		}
};

class nsox_signal : public unix_signal
{
public:
		SIGNAL_MAP_BEGIN(nsox_signal)
				SIGNAL_HANDLER(SIGUSR1, stop_process)
		SIGNAL_MAP_END
		static void stop_process(int sig){
				nsox::env_selector_inst->stop();
		}

		static void add_stop_sig()
		{
				add(SIGUSR1);
		}
};
}

#endif