#include "selector_sel.h"
#include "nuassert.h"
#include "nu_except.h"
#include "nulog.h"
#include <algorithm>
nsox_namespace_begin

inline void selector_select::run(nu_time_t timeout)
{
		abstract_selector::run(timeout);
		nsox::xlog(NsoxInfo, "selector_select begin to run");


		while(is_running()){
				HSOCKET max = 0;
				fd_set rfds, wfds, efds;
				FD_ZERO(&rfds);FD_ZERO(&wfds);FD_ZERO(&efds);

				struct timeval tvo;
				if (timeout >= 0){
						tvo.tv_sec = timeout / 1000;
						tvo.tv_usec = (timeout % 1000) * 1000;
				}
				begin_loop(); // add handles to container

				select_map::iterator itr;
				for (itr = __handlers.begin(); itr != __handlers.end(); ++itr)
				{
						HSOCKET s = itr->first;	if(!VALID_SOCKET(s)) continue;

						if(itr->second.ev & EVENT_READ)		FD_SET(s, &rfds);
						if(itr->second.ev & EVENT_WRITE)		FD_SET(s, &wfds);
						if(itr->second.ev & EVENT_EXCPTION)	FD_SET(s, &efds);
						if (max < s)	max = s;
				}
				if(!max){
						nsox::xlog(NsoxError, "there is no handles feed to selector");
						break;
				}
				int nready = ::select(int(max) + 1, &rfds, &wfds, &efds, &tvo);

				logger::inst()->set_logger_time(); // set logger time

				if(nready  == -1){
						nsox::xlog_err(NsoxError, nsox_last_error, "select error");
				}
				if(nready == 0){
						//nsox::xlog(NsoxDebug, "select timeout");
				}
				if (nready > 0){
					for (itr = __handlers.begin(); itr != __handlers.end(); ++itr){
							HSOCKET s = itr->first;
							int event = 0;
							if (FD_ISSET(s, &rfds)) event |= EVENT_READ;
							if (FD_ISSET(s, &wfds)) event |= EVENT_WRITE;
							if (FD_ISSET(s, &efds)) event |= EVENT_EXCPTION;
							if(event){
									itr->second.handler->onevent(event);
									if(!-- nready) break;
							}
					}
				}
				end_loop();
		}

		clear();

		nsox::xlog(NsoxInfo, "selector_select begin to end");
}

nsox_namespace_end


