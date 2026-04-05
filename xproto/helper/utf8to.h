
#ifndef __POPO3_UTF8_TOOLS_CODE_HPP_INCLUDE__
#define __POPO3_UTF8_TOOLS_CODE_HPP_INCLUDE__
#include <string>
#include <tchar.h>
#include <windows.h>
// 内部分配内存. 需要在外面释放
void net2app(const std::string & net, wchar_t ** io, int cp = CP_UTF8);
// 
std::string app2net(const wchar_t * w, int cp = CP_UTF8);

std::string app2net(const std::wstring &w, int cp = CP_UTF8);

inline std::wstring itow10(int i){
	wchar_t buf[20];
	return std::wstring(_itow(i, buf, 10));
}
/*
void net2app(const properties_type & in, _tproperties_type & out);
void app2net(const _tproperties_type & in, properties_type & out);

std::string uid_app2net(const wchar_t * w);

inline void app2net(const _TProperties & in, Properties & out)
{
	app2net(in.props, out.props);
}

void app2net(const _tuidlist_t_ & in, strlist_t & out);

inline void app2net(const _TEditProperties & in, EditProperties & out)
{
	out.op = in.op;
	out.tag = in.tag;
	app2net(in.props, out.props);
}

inline void net2app(const Properties & in, _TProperties & out)
{
	net2app(in.props, out.props);
}

struct PPSTRBS
{
	std::list<wchar_t *> gs;

	PPSTRBS() { }
	void guard(wchar_t * str) { gs.push_back(str); }

	~PPSTRBS()
	{
		for (std::list<wchar_t *>::iterator i = gs.begin(); i != gs.end(); ++i)
		{
			delete [] (*i);
		}
	}

private:
	PPSTRBS(const PPSTRBS&);
	void operator=(const PPSTRBS&);
};
*/
struct PPSTRB
{
	explicit PPSTRB(wchar_t * str = NULL) : m_str(str) { }
	explicit PPSTRB(int size) { m_str = new wchar_t[size]; }

	wchar_t * detach()     { wchar_t * tmp = m_str; m_str = NULL; return tmp; }
	operator wchar_t *()   { return m_str; }
	wchar_t ** operator&() { return &m_str; }
	~PPSTRB()              { delete [] m_str; }

	wchar_t * m_str;

private:
	PPSTRB(const PPSTRB&);
	void operator=(const PPSTRB&);
};
/*
// 回调处理转换辅助，内部构造 XSTRING，
inline wchar_t * net2app(const std::string & net, PPSTRBS & c)
{
	PPSTRB app;
	net2app(net, &app);
	c.guard(app);
	return app.detach();
}
*/

// get_ 方法中处理辅助，不能把内部的 XSTRING 给出去。
inline std::wstring& net2app(const std::string & net, std::wstring& io)
{
	PPSTRB app;
	net2app(net, &app);
	io = app;
	return io;
}

inline std::wstring net2app(const std::string & net){
	PPSTRB app;
	net2app(net, &app);
	std::wstring io = app;
	return io;
}

#endif // __POPO3_UTF8_TOOLS_CODE_HPP_INCLUDE__
