#pragma once

namespace nsox{

class CBaseProtocol :
	public IProtocol
{
public:
		template<typename T> 
		class task_func
		{
		public:
			task_func(){
			}
			task_func(T* value){	
					std::auto_ptr<T> temp(value);
					task = temp;
			}		
			task_func(const task_func& obj){
					task_func* temp = const_cast<task_func*>(&obj);
					this->task = temp->task;
			}			
			void operator()(){
					task->run();
			}
		protected:
			std::auto_ptr<T> task;
		};

		class AsynTcpTask : public Task
		{
		public:
			AsynTcpTask(Request* request){
					_request = request;
			}
			virtual ~AsynTcpTask(){
					delete _request;
			}
			virtual void run(){						
			}
		protected:
			Request* _request;
		};

public:
	CBaseProtocol(void);
	~CBaseProtocol(void);
public:
	virtual void onCreate(Connection* conn)
	{

	}
	virtual void onRequest(Request* request)
	{
			_threads += task_func<Task>(parse(request));
	}
	virtual void onCreate(Connection* conn)
	{
	}
	virtual void onException(Connection* conn)
	{
	}
	virtual Task* parse(Request* request)
	{
		return new AsynTcpTask(request);
	}		
protected:
	threadpool::scoped_pool<threadpool::fifo_pool>		_threads;
};

}
