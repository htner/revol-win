#ifndef nsox_connection_h
#define nsox_connection_h
#include "sockcomm.h"
namespace nsox{
struct Runnable
{
		virtual  ~Runnable(){};
		virtual void run()				= 0;
};
class Task : public Runnable
{
public:
	virtual void run()
	{}
};
struct Request{
		virtual	~Request(){}
		virtual Request*	clone()		= 0;
};
struct Response{
		virtual	~Response(){}
		virtual Response*	clone()		= 0;
};
struct Connection
{
		virtual ~Connection(){}
		virtual void		close()						= 0;
		virtual void		flush()						= 0;
		virtual Request*	read()						= 0;
		virtual void		write(Response* response)	= 0;
		virtual void		release()					= 0;
		virtual HSOCKET		getSocket()					= 0;
};

struct ConnectionHandler
{
		virtual void		onCreate(Connection* conn)			= 0;
		virtual void		onClose(Connection* conn)			= 0;

};

struct IProtocol
{
		virtual void		onCreate(Connection* conn)						 = 0;
		virtual void		onRequest(Request* request)						 = 0;
		virtual void		onDestroy(Connection* conn)					 = 0;
		virtual void		onException(Connection* conn)					 = 0;
};

}// end of namespace nsox
#endif //end nsox