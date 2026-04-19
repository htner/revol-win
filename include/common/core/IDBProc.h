#ifndef IDBPROC_H_
#define IDBPROC_H_
#include "ibase.h"

namespace mysqlpp{
	class Connection;
}

namespace core{
struct IDBProc{
	virtual ~IDBProc(){}
	virtual Request::ResponseType procDB(Request &request, mysqlpp::Connection *c) = 0;
};
}
#endif /*IDBPROC_H_*/
