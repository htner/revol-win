#ifndef nsox_exception_h
#define nsox_exception_h


#include "sockcomm.h"
#include <exception>
#include "socketutil.h"

nsox_namespace_begin

class socket_exception : public std::exception
{
public:
	socket_exception(const std::string& str)        { init(str); }
	socket_exception(int e, const std::string& str) { init(e, str); }

	const std::string & what_str() const throw();
	int what_errno() const		   throw() { return m_errno; }
	virtual const char * what () const throw() { return what_str().c_str (); }

	~socket_exception() throw() { }

	static int getLastError();
private:
	socket_exception() : exception () { }
	void init(int e, const std::string & str)
	{
			m_errno = e;
			m_what = str + " : ";
			m_bstrerror = false;
	}
	void init(const std::string & str) { init(nsox_last_error, str); }
	int m_errno;
	mutable std::string m_what;
	mutable bool m_bstrerror;
};


class socket_error : public socket_exception
{
public:
	socket_error(const std::string& what_arg, int err = getLastError())
		: socket_exception(err, what_arg)
	{}

};

class connect_timeout: public socket_error
{
public:
	connect_timeout(const std::string& what_arg = "connect time out")
		: socket_error( what_arg, getLastError())
	{}
	connect_timeout(int e, const std::string& what_arg = "connect time out")
		: socket_error( what_arg,e)
	{}
};

class connect_error : public socket_error
{
public:
	connect_error(const std::string& what_arg = "connect  error")
		: socket_error( what_arg, getLastError())
	{}
	connect_error(int e, const std::string& what_arg = "connect  error")
		: socket_error( what_arg,e)
	{}
};

inline const std::string & socket_exception::what_str() const throw()
{
	if (m_bstrerror)
		return m_what;

	m_bstrerror = true;

	if (0 == what_errno())
		return m_what;

	//m_what += " - ";
	//m_what += ultoa(what_nsox_last_error()) + ": ";

	std::string errormsg;
	SocketUtil::format_error(what_errno(), errormsg);
	m_what += errormsg;

	return m_what;
}
#ifdef NSOX_WIN32
		inline int socket_exception::getLastError()
		{
				return WSAGetLastError();
		}
#else
		inline int socket_exception::getLastError()
		{
				return nsox_last_error;
		}
#endif

nsox_namespace_end
#endif

