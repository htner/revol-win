#ifndef nsox_connection_factory_h
#define nsox_connection_factory_h

#include "selector.h"
#include "connection.h"
#include "connectionmanager.h"
namespace nsox{

template<typename TConn>
class ConnectionFactory
{
	class ConnectionAdapter : public TConn
	{
			typedef ConnectionFactory CallBack;
	public:
			ConnectionAdapter(){}
			ConnectionAdapter(HSOCKET hSocket) : TConn(hSocket)
			{
			}
			void setCallBack(CallBack* callback)
			{
					_callback = callback;
			}
	public:
			virtual void close()
			{
					_callback->onClose(this);
			}
	private:
			CallBack* _callback;
	};
public:
		void setHandler(ConnectionHandler* handler)
		{
			_connHandler = handler;
		}
		void create()
		{
				Connection*  conn = _listend.accept<ConnectionAdapter>();
				onCreate(conn);
		}

		void onCreate(Connection* conn)
		{
				
				_conns.addConnection(conn);
				_connHandler->onCreate(conn);
		}

		void onClose(Connection* conn)
		{
				_connHandler->onClose(conn);
				_conns.delConncetion(conn);
		}


		void listen(const Address& address)
		{
				_listend.create();
				_listend.bind(address);
		}

		void start()
		{
				_listend.listen();
		}
protected:
		ConnectionManager	_conns;
		ListenSocket		_listend;	
		ConnectionHandler*	_connHandler;
};


template<typename TConnection>
class AsynConnectionFactory : public ConnectionFactory<TConnection>, public IEventHandler
{
public:
	virtual void onEvent(int ev)
	{
			if(ev == EVENT_READ)
					create();
	}
	virtual HSOCKET getHandler()
	{
		return _listend.getHSocket();
	}
};

}//end of namespace
#endif //nsox_connection_factory_h