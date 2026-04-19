#ifndef __abstract_selector_h
#define __abstract_selector_h

#include "sockcomm.h"
#include "nuautoptr.h"
#include <map>

nsox_namespace_begin

/*template<typename T>
class nsox_vector_map
{
public:
		struct xx_item {
				bool __in_use;
				T    __value;
		};

		typedef std::map<int, T> item_map;
		typedef std::list<int>	 item_list;

		nsox_vector_map(int vectorsize)
		{
				__array  = new xx_item[vectorsize];
				__max_fd = -1;
		}
		void set(int fd, T& value){
				if(fd < __vector_size){
						__array[fd] = value;
				}else{
						__map[fd] = value;
				}
		}
		T&  get(int fd){
				if(fd < __vector_size){
						return __array[fd];
				}else{
						return __map[fd];
				}
		}
		void remove(int fd){
				if(fd > __max_fd) return;

				if(fd < __vector_size){
						__array[fd].__in_use = false;
				}else{
						__map.erase(fd);
				}
				if(fd == __max_fd){

				}
		}

		bool exist(int fd){
				if(fd < __vector_size){
						return __array[fd].__in_use;
				}else{
						return __map.find(fd) != __map.end();
				}
		}
		bool empty(){
				return __max_fd == -1;
		}

		T& first()
		{
				__array_cur		= -1;
				__map_cur		= __map.begin();

				return next();
		}
		T& next()
		{
				if(__array_cur < __vector_size){
						do{
								__array_cur ++ ;
								if(__array[__cursor].__in_use){
										return __array[__cursor].__value;
								}						
						}while(__array_cur<__vector_size && __array_cur<__max_fd)
				}
				if(__array_cur= __max_fd || __map_cur==__map.end()) {
						throw std::runtime_error("next error , no fd avalible");
				}
				T& ret = __map_cur->second;
		}
		bool end()
		{
				return ((__cursor>=__max_fd) || (__map_cur==__map.end()));
		}


protected:
		nu_size_t				__vector_size;
		xx_item*				__array;
		item_map				__map;		
		int					   __max_fd;

		item_map::iterator	   __map_cur;
		int					   __array_cur;
};*/

class abstract_selector : public event_selector
{
public:
	abstract_selector(void);
	virtual ~abstract_selector(void);

	typedef nu_auto_ptr<event_handler>  event_auto_ptr;

	enum sel_action{
		sel_add = 0,
		sel_del
	};
	struct action_item{
			action_item(event_handler* p, sel_action act, int ev){
					handler = p; action =act; event = ev;
					hsocket = handler->getsocket();
			}
			event_auto_ptr		handler;
			sel_action			action;
			int					event;
			HSOCKET				hsocket; 
	};

	struct event_item 
	{
				event_item():ev(EVENT_NONE){}
				int						ev;
				event_auto_ptr			handler;
				HSOCKET					hsocket; 
	};

 
	typedef std::map<HSOCKET, event_item>		select_map;
	typedef	std::list<action_item>				select_list;// should be lock free

public:
	virtual void advise(event_handler* handler, int ev);
	virtual void unadvise(event_handler* handler, int ev);
	virtual void run(nu_time_t timeout);
	virtual	void stop();

	void	clear();

	boolean is_running(){return !__quiting;};
public:
	virtual void on_new_handler(event_item& item){};
	virtual void on_event_change(event_item& item){};
	virtual void on_del_handler(event_item& item){};

protected:
	void begin_loop();
	void end_loop();
protected:
	//two contaniers for multi-thread applications
	select_map		__handlers;		
	select_list		__action_handlers;
	bool			__quiting;
	int				__time_elapse;
};
nsox_namespace_end

#include "abstract_selector.hpp"
#endif
