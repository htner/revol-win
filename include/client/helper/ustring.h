#pragma once

#include <vector>
#include <string>
#include <map>

#include "ctype.h"

namespace helper
{
	class  ustring
	{
	public:
		static bool Analyze(const wchar_t* str, TCHAR c, std::vector<std::wstring>& array);
		static bool Analyze(const wchar_t* str, TCHAR c, TCHAR c2, std::map<std::wstring,std::wstring>& out_map);

	public:	//字符串到基本结构的转换
		static RECT ToRect(const wchar_t* str);

		static POINT ToPoint(const wchar_t* str);

		static SIZE ToSize(const wchar_t* str);

		static SIZE ToSize(const char* str);

		static bool ToBool(const wchar_t* str);	
		static bool ToBOOL(const wchar_t* str);	

		static std::wstring ToString(long val);
		static int ToInt(const wchar_t* p);
		static int ToLong(const wchar_t* p);

		//static std::wstring TimeToString(const time_t& timestamp);

		static std::wstring SecondToSimpleDisplayTime(DWORD dwSecond);

		static std::wstring SecondToFullDisplayTime(DWORD dwSecond);
	public:
		/**
		 * 包含编码转换
		 */
		static std::wstring toWideString(const char* pStr, int len);

		/**
		 * 包含编码转换
		 */
		static std::string toNarrowString(const wchar_t* pStr, int len);

		/**
		 * 纯字节流变化
		 */
		static std::string ToNarrowString(const std::wstring& wString);

		/**
		 * 纯字节流变化
		 */
		static std::wstring	ToWideString(const std::string& nString);

	public:
		/**
		 * 不做参数检查，自己在外面做，code by neou
		 */
		static size_t find(const char* pattern, size_t pLen, const char* des, size_t sLen);

	public:
		static std::wstring EscapeHtmlEntity(const std::wstring& text);

		/**
		 * 将str的内容全部转换为小写
		 * @param
		 */
		static std::wstring& ToLower(std::wstring& str);

		/**
		 * 将str的内容全部转换为大写
		 * @param
		 */
		static void ToUpper(std::wstring& str);

		static std::wstring Trim(const std::wstring& str);

		/**
		 * 剔除txt中的unicode控制符
		 * @param
		 */
		static void TrimUnicodeCtrlChar(std::wstring& txt);

		static wchar_t* TrimChar(wchar_t* pString,wchar_t* pattern);
		static std::wstring TrimChar(const std::wstring& txt,wchar_t* pattern);

		//lpString與lphead是不是頭匹配
		static bool isBeginWith(LPCWSTR lpString,LPCWSTR lphead);

		//判断字符串是否全为数字
		static bool isNumber(const std::wstring& str);

		static void StringReplace(std::wstring& strsrc, 
											const std::wstring& strsub,const std::wstring& strdst);

		//获取字符串的字节数
		static int GetStringBytes(const xstring &strText);
		//獲取指定長度的字串
		static xstring GetSubStringByBytes(const xstring &strText,int nBytes);
	};
};

#include "ustring.hpp"