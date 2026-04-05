#include "utf8to.h"
#include "tmpbuf.h"
//#include "/expp.hpp"

//#include "util/ppuid.hpp"
//#include "util/protocol/isrvuni.hpp"

#include <windows.h>

//////////////////////////////
// utf8 <-> utf16

void net2app(const std::string & net, wchar_t ** io, int cp)
{
	// utf8 -> utf16

	// 字符数目减少，多分配些，不计算实际需要的大小
	// 临时使用，如果需要存贮，改成先计算大小
	// 但是仍然保留 NULL terminate
	int chars = int(net.size());
	// int chars = WideCharToMultiByte(cp, 0, net.c_str(),
	//	-1, NULL, NULL, NULL, NULL);

	PPSTRB buf(chars + 2); // allocate
	int ret = MultiByteToWideChar(cp, 0, net.data(), chars, buf.m_str, chars);
	if (ret < 0) ret = 0;
	buf.m_str[ret] = 0; // null terminate
	buf.m_str[ret + 1] = 0;

	if (*io) // free old
		delete [] (*io);

	*io = buf.detach();
}

std::string app2net(const std::wstring &str, int cp){
	return app2net(str.data(), cp);
}

std::string app2net(const wchar_t * w, int cp)
{
	if (!w)
		return std::string();

	// utf16 -> utf8
	TempBuffer<char> str;

	// 计算大小
	int size = WideCharToMultiByte(cp, 0, w, -1, NULL, NULL, NULL, NULL);

	// 转换
	WideCharToMultiByte(cp, 0, w, -1, str.Allocate(size), size, NULL, NULL);

	return std::string(str.data());
}

//////////////////////////////////////////////////////////////
// helper
/*
std::string uid_app2net(const wchar_t * w)
{
	std::string uid = app2net(w);
	if (!util::TransformValidateUid(uid))
		THROW_EXPP(PP_EBADUID, uid);
	return uid;
}

void app2net(const _tuidlist_t_ & in, strlist_t & out)
{
	for (_tuidlist_t_::const_iterator i = in.begin(); i != in.end(); ++i)
	{
		out.push_back(uid_app2net(*i));
	}
}

void app2net(const _tproperties_type & in, properties_type & out)
{
	for (_tproperties_type::const_iterator i = in.begin(); i != in.end(); ++i)
	{
		out[i->first] = app2net(i->second);
	}
}
*/
/*
void net2app(const properties_type & in, _tproperties_type & out)
{
	for (properties_type::const_iterator i = in.begin(); i != in.end(); ++i)
	{
		out[i->first] = net2app(i->second);
	}
}
*/

