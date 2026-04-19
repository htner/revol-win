#pragma once

#ifndef __POPO3_UTF8_TOOLS_CODE_HPP_INCLUDE__
#define __POPO3_UTF8_TOOLS_CODE_HPP_INCLUDE__
#include <string>
#include <tchar.h>
#include <windows.h>
#include "tmpbuf.h"
// 内部分配内存. 需要在外面释放

namespace helper
{

inline std::wstring itow10(int i)
{
	wchar_t buf[20];
	return std::wstring(_itow(i, buf, 10));
}
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



inline void net2app(const std::string & net, wchar_t ** io, int cp = CP_UTF8)
{
	int chars = int(net.size());

	PPSTRB buf(chars + 2); // allocate
	int ret = MultiByteToWideChar(cp, 0, net.data(), chars, buf.m_str, chars);
	if (ret < 0) ret = 0;
	buf.m_str[ret] = 0; // null terminate
	buf.m_str[ret + 1] = 0;

	if (*io) // free old
		delete [] (*io);

	*io = buf.detach();
}

inline std::string app2net_s(const wchar_t * w, int cp = CP_UTF8)
{
	if (!w) return std::string();
	// utf16 -> utf8
	TempBuffer<char> str;
	// 计算大小
	int size = WideCharToMultiByte(cp, 0, w, -1, NULL, NULL, NULL, NULL);
	// 转换
	WideCharToMultiByte(cp, 0, w, -1, str.Allocate(size), size, NULL, NULL);
	return std::string(str.data());
}

inline std::string app2net(const std::wstring &str, int cp = CP_UTF8 ){
	return app2net_s(str.data(), cp);
}


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


inline std::wstring asci2app(const std::string & net){
	PPSTRB app;
	net2app(net, &app, CP_ACP);
	std::wstring io = app;
	return io;
}

inline std::string app2asci(const std::wstring& app)
{
	return app2net_s(app.data(), CP_ACP);
}



inline  std::string urlencode(const std::string& str)
{
	static const std::string allow_chars("aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ0123456789-_.");
	std::string allow = allow_chars;
	std::string res;
	std::string::size_type pos = 0, fpos;
	char buf[4];
	unsigned char ch;
	while (true) {
		fpos = str.find_first_not_of(allow, pos);
		if (fpos == std::string::npos)
			break;
		res.append(str, pos, fpos-pos);
		ch = str[fpos];
		sprintf(buf, "%%%02hhx", ch);
		res += buf;
		pos = fpos+1;
	}
	res.append(str, pos, str.length()-pos);
	return res;
}

inline std::wstring urlencode(const std::wstring& str)
{
	return net2app(urlencode(app2net(str)));
}

inline static int HexPairValue(const char * code) 
{
	int value = 0;
	const char * pch = code;
	for (;;) {
		int digit = *pch++;
		if (digit >= '0' && digit <= '9') {
			value += digit - '0';
		}
		else if (digit >= 'A' && digit <= 'F') {
			value += digit - 'A' + 10;
		}
		else if (digit >= 'a' && digit <= 'f') {
			value += digit - 'a' + 10;
		}
		else {
			return -1;
		}
		if (pch == code + 2)
			return value;
		value <<= 4;
	}
}


inline int urldecode(const char *source, char *dest)
{
	char * start = dest;

	while (*source) {
		switch (*source) {
	case '+':
		*(dest++) = ' ';
		break;
	case '%':
		if (source[1] && source[2]) {
			int value = HexPairValue(source + 1);
			if (value >= 0) {
				*(dest++) = value;
				source += 2;
			}
			else {
				*dest++ = '?';
			}
		}
		else {
			*dest++ = '?';
		}
		break;
	default:
		*dest++ = *source;
		}
		source++;
	}

	*dest = 0;
	return dest - start;
}  

inline std::string urldecode(const std::string & encoded) 
{
	const char * sz_encoded = encoded.c_str();
	size_t needed_length = encoded.length();
	for (const char * pch = sz_encoded; *pch; pch++) {
		if (*pch == '%')
			needed_length += 2;
	}
	needed_length += 10;
	char stackalloc[64];
	char * buf = needed_length > sizeof(stackalloc)/sizeof(*stackalloc) ?
		(char *)malloc(needed_length) : stackalloc;
	urldecode(encoded.c_str(), buf);
	std::string result(buf);
	if (buf != stackalloc) {
		free(buf);
	}
	return result;
}

inline std::wstring urldecode(const std::wstring & encoded) 
{
	std::string encodeA = helper::app2net(encoded);
	return helper::net2app( urldecode(encodeA) );
}

}

#endif // __POPO3_UTF8_TOOLS_CODE_HPP_INCLUDE__
