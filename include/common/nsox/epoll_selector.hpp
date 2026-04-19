
#include <time.h>
#include <signal.h>
#include <unistd.h>


nsox_namespace_begin

inline epoll_selector::epoll_selector(int maxFdCount)
{
		m_maxfds = maxFdCount;
		m_epoll_fd = -1;
		if(-1 == (m_epoll_fd = epoll_create(m_maxfds)))
		{
				nsox::xlog_err(NsoxFatal, nsox_last_error, "epoll_create failed");
				throw socket_error("epoll_create failed", nsox_last_error);
		}
}
inline epoll_selector::~epoll_selector(void)
{
		close(m_epoll_fd);

}

inline void epoll_selector::setup_epoll(int efd, int op, int sfd, epoll_event &ev) {
	int ret = epoll_ctl(efd, op, sfd, &ev);
	if (ret != 0) {
		switch (ret) {
		case EBADF:
			nsox::xlog(NsoxError, "epoll_selector::setup_epoll: epfd or fd is not a valid file descriptor.");
			return;
		case EEXIST:
			nsox::xlog(NsoxError, "epoll_selector::setup_epoll: was EPOLL_CTL_ADD, and the supplied file descriptor fd is already in epfd.");
			return;
		case EINVAL:
			nsox::xlog(NsoxError,"epoll_selector::setup_epoll: epfd is not an epoll file descriptor, or fd is the same as epfd, or the requested operation op is not supported by this interface.");
			return;
		case ENOENT:
			nsox::xlog(NsoxError,"epoll_selector::setup_epoll:  op was EPOLL_CTL_MOD or EPOLL_CTL_DEL, and fd is not in epfd. ");
			return;
		case ENOMEM:
			nsox::xlog(NsoxError,"epoll_selector::setup_epoll: There was insufficient memory to handle the requested op control operation.");
			return;
		case EPERM:
			nsox::xlog(NsoxError, "epoll_selector::setup_epoll: The target file fd does not support epoll.");
			return;
		}
	}
}
inline void epoll_selector::on_new_handler(event_item& item)
{
		struct epoll_event ep;
		memset(&ep, 0, sizeof(ep));

		if (item.ev & EVENT_READ)  ep.events |= EPOLLIN;
		if (item.ev & EVENT_WRITE) ep.events |= EPOLLOUT;
		ep.data.ptr = item.handler;
		setup_epoll(m_epoll_fd, EPOLL_CTL_ADD, item.hsocket, ep);
}
inline void epoll_selector::on_event_change(event_item& item)
{
		struct epoll_event ep;
		memset(&ep, 0, sizeof(ep));

		if (item.ev & EVENT_READ)  ep.events |= EPOLLIN;
		if (item.ev & EVENT_WRITE) ep.events |= EPOLLOUT;
		ep.data.ptr = item.handler;
		setup_epoll(m_epoll_fd, EPOLL_CTL_MOD, item.hsocket, ep);
}
inline void epoll_selector::on_del_handler(event_item& item)
{
		epoll_event ep = { 0, { 0 } };
		HSOCKET fd = item.handler->getsocket();

		setup_epoll(m_epoll_fd, EPOLL_CTL_DEL, item.hsocket, ep);
}
inline void epoll_selector::run(nu_time_t timeout)
{
		abstract_selector::run(timeout);

		nsox::xlog(NsoxInfo, "epoll begin to run + + + + + + + + + + + + ");
		epoll_event events[128];
		while(is_running()) {
				begin_loop();

				int nfds = epoll_wait(m_epoll_fd, events, 128, timeout);

				logger::inst()->set_logger_time();
				if (nfds < 0) {
						if (EINTR == nsox_last_error){
								continue;
						}else{
								nsox::xlog(NsoxFatal,"epoll_wait error");
								throw socket_error("epoll_wait error");
						}
				}
				for(int n = 0; n < nfds; ++n) {
						struct epoll_event*  ev = &events[n];
						notify_event((event_handler*)ev->data.ptr, ev->events); // please ensure the socket pointer is valid now?
				}
				end_loop();
		}
		clear();
		nsox::xlog(NsoxInfo, "epoll begin to stop + + + + + + + + + + + + ");
}

inline void epoll_selector::notify_event(event_handler* handler, int ev)
{

	    int events = 0;
		if(ev & EPOLLIN) events |= EVENT_READ;
		if(ev & EPOLLOUT) events |= EVENT_WRITE;

		handler->onevent(events);
}


nsox_namespace_end


