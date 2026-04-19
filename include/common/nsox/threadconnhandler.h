#ifndef nsox_thread_conn_handler_h
#define nsox_thread_conn_handler_h
#include "threadpool/threadpool.hpp"
#include "Connection.h"

using namespace  boost::threadpool;
namespace nsox{

class ThreadConnHandler : public ConnectionHandler
{
	class ThreadWorker
	{
	public:
			ThreadWorker(Connection* conn, Protocol* Protocol)
			{
					_conn = conn;
					_protocol = Protocol;
			}
			virtual ~ThreadWorker(){}
			void operator() ();
	private:
			Connection*		_conn;
			Protocol*		_protocol;
	};

public:
	ThreadConnHandler(void);
	virtual ~ThreadConnHandler(void);
public:
	virtual void onCreate(Connection* conn)
	{			
			ThreadWorker client(conn,_protocol);
			tp +=  client;
	}
	virtual void onClose(Connection* conn){
	}

	virtual void setProtocol(Protocol* protocol)
	{
			_protocol = protocol;
	}
private:
	scoped_pool<fifo_pool, 5> tp;
	Protocol*				_protocol;
};

}// end namespace nsox
#endif //nsox_thread_conn_handler_h 

