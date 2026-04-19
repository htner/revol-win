#ifndef	__asyn_socket_h
#define __asyn_socket_h
#include "selector.h"
#include "nsoxenv.h"

nsox_namespace_begin

template<typename TSocket>
class asyn_socket :  public TSocket
				  ,  public event_handler
				  ,  public timer_handler
{
public:
	asyn_socket(){
			
	}

	virtual void set_socket_handler(HSOCKET hSocket)
	{
			TSocket::set_socket_handler(hSocket);		
			TSocket::set_block_mode(false);
	}
	virtual ~asyn_socket(){}
	virtual void onevent(int ev)
	{
			if(ev & EVENT_READ){
					onread();
			}if(ev & EVENT_WRITE){
					onwrite();
			}if(ev & EVENT_TIMTOUT){
					ontimeout();
			}if(ev & EVENT_EXCPTION){
					onexception();
			}
	}
	virtual HSOCKET	getsocket()
	{
			return TSocket::get_socket_handler();
	}
public://timer_handler
	virtual void timeout()
	{
			nsox::xlog(NsoxDebug, "timeout did't handler");	
	}
public:
	void	select(int ev, bool badd)
	{	
			if(badd)
					env::inst()->selector()->advise(this, ev);
			else
					env::inst()->selector()->unadvise(this, ev); 
	}

	timer_id    select_timeout(int timeout)
	{
			return env::inst()->timer()->set_timer(this, timeout);
	}
	void		remove_timeout(timer_id id)
	{
			env::inst()->timer()->kill_timer(id);
	}
	virtual void onread(){
			nsox::xlog(NsoxDebug, "read event ignored");
	};
	virtual void onwrite(){
			nsox::xlog(NsoxDebug, "write event ignored");
	};
	virtual void onexception(){
			nsox::xlog(NsoxDebug, "exception event ignored");
	};
	virtual void ontimeout(){
			nsox::xlog(NsoxDebug, "timeout event ignored");
	};
};

nsox_namespace_end
#endif

