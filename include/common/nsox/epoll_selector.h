#ifndef __epoll_selector_h
#define __epoll_selector_h
 
#include "selector.h"
#include <sys/epoll.h>

#include <map>
#include <list>
#include "selector.h"
#include "nuautoptr.h"
#include "abstract_selector.h"
#include "nulog.h"

#define EPOLL_QUEUE_SIZE 65535

nsox_namespace_begin


class epoll_selector : public abstract_selector
{
public:
	epoll_selector(int maxFdCount = EPOLL_QUEUE_SIZE);
	virtual ~epoll_selector();

	virtual void on_new_handler(event_item& item);
	virtual void on_event_change(event_item& item);
	virtual void on_del_handler(event_item& item);
	void notify_event(event_handler* handler, int ev);

	void setup_epoll(int efd, int op, int sfd, epoll_event &ev);

	virtual void run(nu_time_t timeout);

protected:
	int m_epoll_fd;
	int m_maxfds;
};
nsox_namespace_end

#include "epoll_selector.hpp"
#endif



