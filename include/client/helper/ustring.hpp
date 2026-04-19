

#include <sstream>
#include <algorithm>
#include <assert.h>
namespace 
{
	inline bool IsWhiteSpace(wchar_t ch)
	{
		return ch <= 0x0020 || ch == 0x3000 || ch == 0xE7FE || ch == 0xE812 || ch == 0xF8F8 || ch == 0xEE7C || ch == 0xE5D1 || ch == 0xEEAE || ch == 0xEEAF || ch == 0xE2C1 || ch == 0xE009;

	}

	inline bool IsUniCodeControlChar(wchar_t& ch)
	{
		if (ch == 0x200E || ch == 0x200F ||
			ch == 0x200D ||	ch == 0x200C ||
			ch == 0x202A ||	ch == 0x202D ||
			ch == 0x202E ||	ch == 0x202C ||
			ch == 0x206E ||	ch == 0x206F ||
			ch == 0x206B ||	ch == 0x206A ||
			ch == 0x206D ||	ch == 0x206C ||
			ch == 0x1E || ch == 0x1F )
		{
			return true;
		}
		return false;
	}
}

namespace helper
{
	inline RECT ustring::ToRect(const wchar_t* str)
	{		
		RECT rcEmpty = {0};
		std::vector<std::wstring> array;
		if( Analyze(str, ',', array) && array.size() >= 4 )
		{
			RECT rcResult = 
			{
				_wtoi(array[0].c_str()), 
				_wtoi(array[1].c_str()),
				_wtoi(array[2].c_str()), 
				_wtoi(array[3].c_str())
			};
			return rcResult;
		}
		return rcEmpty;
	}

	inline POINT ustring::ToPoint(const wchar_t* str)
	{
		POINT ptZero = {0};
		std::vector<std::wstring> array;
		if( Analyze(str, ',', array) && array.size() >= 2)
		{
			POINT pt = {_wtoi(array[0].c_str()), _wtoi(array[1].c_str())};
			return pt;
		}

		return ptZero;
	}

	inline SIZE ustring::ToSize(const char* str)
	{
		std::wstring wstr = toWideString(str, strlen(str));
		return ToSize(wstr.c_str());
	}

	inline SIZE ustring::ToSize(const wchar_t* str)
	{
		SIZE sizeEmpty = {0};
		std::vector<std::wstring> array;
		if( Analyze(str, ',', array) && array.size() >= 2)
		{
			SIZE size = { _wtoi(array[0].c_str()), _wtoi(array[1].c_str()) };
			return size;
		}
		if( Analyze(str, 'x', array) && array.size() >= 2)
		{
			SIZE size = { _wtoi(array[0].c_str()), _wtoi(array[1].c_str()) };
			return size;
		}
		return sizeEmpty;
	}

	inline bool ustring::ToBool(const wchar_t* str)
	{
		std::wstring src(str);
		return (src == std::wstring(_T("T")) ||
			src == std::wstring(_T("t")) ||
			src == std::wstring(_T("1"))
			);
	}
	inline bool ustring::ToBOOL(const wchar_t* str)
	{
		return (ToBool(str)==true)?TRUE:FALSE;
	}

	inline std::wstring ustring::ToString(long val)
	{
		TCHAR buffer[1024];
		ZeroMemory(buffer, sizeof(TCHAR) * 1024);
		wsprintf(buffer, _T("%ld"), val);

		return std::wstring(buffer);
	}
	inline int ustring::ToInt(const wchar_t* p)
	{
		if(p == NULL) return 0;
		int i = 0;
		try	{i = _wtoi(p);}catch(...){}
		return i;
	}
	inline int ustring::ToLong(const wchar_t* p)
	{
		if(p == NULL) return 0;
		int i = 0;
		try	{i = _wtol(p);}catch(...){}
		return i;
	}

	//inline std::wstring ustring::TimeToString(const time_t& timestamp)
	//{
	//	tm* ts = ::localtime(&timestamp);
	//	if(NULL == ts)
	//		return std::wstring(_T(""));
	//	TCHAR buffer[1024];
	//	ZeroMemory(buffer, sizeof(TCHAR) * 1024);
	//	wsprintf(buffer, _T("%04d-%02d-%02d %02d:%02d:%02d"), ts->tm_year + 1900,ts->tm_mon + 1, ts->tm_mday, ts->tm_hour, ts->tm_min, ts->tm_sec);
	//	return std::wstring(buffer);
	//}

	inline bool ustring::Analyze(const wchar_t* str, TCHAR c, std::vector<std::wstring>& array)
	{
		array.clear();

		std::wstring src(str);
		if(src.length() == 0)
			return false;

		int off		= 0;
		int oldoff	= 0; 
		while(1)
		{
			oldoff = off;

			off = (int)src.find(c, off);

			if(off < 0)
			{
				break;
			}

			std::wstring temp;
			for(int i=0; i<(off-oldoff); ++i)
			{
				temp += src[oldoff + i];
			}
			array.push_back(temp);

			++off;
		}

		std::wstring temp;
		for(int i=oldoff; i<((int)src.length()); ++i)
		{
			temp += src[i];
		}

		array.push_back(temp);

		return true;
	}
	inline bool ustring::Analyze(const wchar_t* str, TCHAR c, TCHAR c2, std::map<std::wstring,std::wstring>& out_map)
	{
		out_map.clear();

		std::wstring src(str);
		if(src.length() == 0)
			return false;

		src += _T(" ");
		src[src.length()-1] = c;

		int off		= 0;
		int oldoff	= 0; 
		while(1)
		{
			oldoff = off;

			off = (int)src.find(c, off);

			if(off < 0)
			{
				break;
			}

			std::wstring temp;
			for(int i=0; i<(off-oldoff); ++i)
			{
				temp += src[oldoff + i];
			}
			
			if(temp.find(c2) != std::wstring::npos)
			{
				size_t ind = temp.find(c2);
				out_map[temp.substr(0,ind)] = (ind<(temp.length()-1))?temp.substr(ind+1):_T("");
			}
			else out_map[temp] = _T("");

			++off;
		}

		return true;
	}

	inline std::wstring ustring::toWideString( const char* pStr , int len )
	{
		std::wstring buf ;

		if(pStr == NULL)
		{
			assert(NULL);
			return buf;
		}

		if (len < 0 && len != -1)
		{
			assert(NULL);
			//OutputDebugStringW( _T("Invalid string length: ") + len);
			return buf;
		}

		// figure out how many wide characters we are going to get 
		int nChars = MultiByteToWideChar( CP_ACP , 0 , pStr , len , NULL , 0 ) ; 
		if ( len == -1 )
			-- nChars ; 
		if ( nChars == 0 )
			return L"" ;

		// convert the narrow string to a wide string 
		// nb: slightly naughty to write directly into the string like this
		buf.resize( nChars ) ; 
		MultiByteToWideChar( CP_ACP , 0 , pStr , len , 
			const_cast<wchar_t*>(buf.c_str()) , nChars ) ; 

		return buf ;
	}

	inline std::string ustring::toNarrowString( const wchar_t* pStr , int len )
	{
		std::string buf ;

		if(pStr == NULL)
		{
			assert(NULL);
			return buf;
		}

		if (len < 0 && len != -1)
		{
			assert(NULL);
			//OutputDebugStringW( _T("Invalid string length: ") + len);
			return buf;
		}

		// figure out how many narrow characters we are going to get 
		int nChars = WideCharToMultiByte( CP_ACP , 0 , 
			pStr , len , NULL , 0 , NULL , NULL ) ; 
		if ( len == -1 )
			-- nChars ; 
		if ( nChars == 0 )
			return "" ;

		// convert the wide string to a narrow string
		// nb: slightly naughty to write directly into the string like this
		buf.resize( nChars ) ;
		WideCharToMultiByte( CP_ACP , 0 , pStr , len , 
			const_cast<char*>(buf.c_str()) , nChars , NULL , NULL ) ; 

		return buf ; 
	}

	inline std::string ustring::ToNarrowString(const std::wstring& wString)
	{
		int len = wString.length()*2;
		char* buffer = new char[len];
		memcpy(buffer,wString.c_str(),len);
		std::string result(buffer,len);
		delete[] buffer;
		return result;
	}

	inline std::wstring ustring::ToWideString(const std::string& nString)
	{
		UINT len = nString.size();

		char* buffer = 0;
		if(len % 2 == 0)//判断长度是否是偶数,如果是，直接补2个0
		{
			buffer = new char[len + 2];
			buffer[len+1] = buffer[len] = 0;
		}
		else//如果为基数，直接补3个0
		{
			buffer = new char[len + 3]; buffer[len+2] = buffer[len + 1] = buffer[len] = 0;
		}
		memcpy(buffer,nString.data(),len);

		std::wstring wStr((wchar_t*)buffer);

		delete[] buffer;

		return wStr;
	}

	inline size_t ustring::find(const char* pattern, size_t pLen, const char* des, size_t sLen)
	{
		if(pLen == 0 || pLen > sLen) return -1;
		size_t off = 0;
		const char* _Uptr, *_Vptr;
		size_t _Nm = sLen;
		for (_Nm -= pLen - 1, _Vptr = des + off;
			(_Uptr = (const char*)memchr(_Vptr, *pattern, _Nm)) != 0;
			_Nm -= _Uptr - _Vptr + 1, _Vptr = _Uptr + 1)
			if (memcmp(_Uptr, pattern, pLen) == 0)
				return (_Uptr - des);	// found a match
		return -1;
	}

	inline std::wstring ustring::EscapeHtmlEntity(const std::wstring& text)
	{
		std::wostringstream out;
		for(std::wstring::const_iterator itr = text.begin(); itr != text.end(); ++itr)
		{
			if( (*itr) == _T('<') )
			{
				out << _T("&lt;");
			}
			else if( (*itr) == _T('>') )
			{
				out << _T("&gt;");
			}
			else
			{
				out << (*itr);
			}
		}

		return out.str();
	}

	inline std::wstring& ustring::ToLower(std::wstring& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), tolower);
		return str;
	}

	inline void ustring::ToUpper(std::wstring& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), toupper);
	}

	inline std::wstring ustring::Trim(const std::wstring& str)
	{
		size_t begin = std::wstring::npos;
		for(size_t i = 0; i < str.size(); ++i)
		{
			if(!IsWhiteSpace(str[i]))
			{
				begin = i;
				break;
			}
		}

		if(begin == std::wstring::npos)
		{
			return std::wstring();
		}

		size_t end = str.size();
		for(size_t i = str.size() - 1; i >= begin; --i)
		{
			if(!IsWhiteSpace(str[i]))
			{
				end = i + 1;
				break;
			}
		}

		return str.substr(begin, end - begin);
	}

	inline wchar_t* ustring::TrimChar(wchar_t* pString,wchar_t* pattern)
	{
		if( !pString || !pattern || !pattern[0] )
			return pString;

		wchar_t* ptr = pString;

		for (wchar_t* p = pString; *p; p++)
		{
			bool bSkip = false;
			for (wchar_t* c = pattern; *c; c++)
			{
				if( *p == *c )
				{
					bSkip = true;
					break;
				}
			}
			if( !bSkip )			
			{
				*ptr ++ = *p;
			}
		}
		*ptr = NULL;
		return pString;
	}

	inline std::wstring ustring::TrimChar(const std::wstring& txt,wchar_t* pattern)
	{
		if( txt.empty() || !pattern || !pattern[0] )
			return txt;

		std::wstring res;

		res.reserve(txt.size() + 1);
		for (int j = 0; j < txt.size(); j++)
		{
			bool bSkip = false;
			for (wchar_t* c = pattern; *c; c++)
			{
				if( txt[j] == *c )
				{
					bSkip = true;
					break;
				}
			}
			if( !bSkip )
				res.append(1,txt[j]);
		}
		return res;
	}

	inline void ustring::TrimUnicodeCtrlChar(std::wstring& txt)
	{
		std::wstring::iterator it = txt.begin();
		for (; it != txt.end(); )
		{
			wchar_t ch = *it;
			if (IsUniCodeControlChar(ch))
			{
				it = txt.erase(it);				
			}else
				it++;
		}
	}

	inline std::wstring ustring::SecondToSimpleDisplayTime(DWORD dwSecond)
	{
		wchar_t szBuffer[256] = {0};
		int nHour		= dwSecond / 3600;
		int nLeftTime	= dwSecond - nHour * 3600;
		int nMinute		= nLeftTime / 60;
		int nSecond		= nLeftTime % 60;
		if(nHour > 0)
		{
			wsprintf(szBuffer,L"%d:%02d:%02d",nHour, nMinute, nSecond);
		}
		else if(nMinute > 0)
		{
			wsprintf(szBuffer,L"%d:%02d",nMinute, nSecond);
		}
		else
		{
			wsprintf(szBuffer,L"%d",nSecond);
		}
		return std::wstring(szBuffer);
	}

	inline std::wstring ustring::SecondToFullDisplayTime(DWORD dwSecond)
	{
		wchar_t szBuffer[256] = {0};
		int nHour		= dwSecond / 3600;
		int nLeftTime	= dwSecond - nHour * 3600;
		int nMinute		= nLeftTime / 60;
		int nSecond		= nLeftTime % 60;
		wsprintf(szBuffer,L"%02d:%02d:%02d",nHour, nMinute, nSecond);
		return std::wstring(szBuffer);
	}


	inline bool ustring::isBeginWith(LPCWSTR lpString,LPCWSTR lphead)
	{
		if( lpString == NULL || lphead == NULL )
			return false;

		int i=0;
		for (; lpString[i]; i++)
		{
			if( lphead[i] == NULL )
				return true;
			if( lphead[i] != lpString[i] && lphead[i] != ::tolower(lpString[i]) )
				return false;
		}
		return (lphead[i] == NULL);
	}

	inline bool ustring::isNumber(const std::wstring& str)
	{
		if (str.length() <= 0)
			return false;

		for(int i = 0; i != str.length(); i++)
		{
			if(str[i] < '0' || str[i] > '9')// (!isdigit(str[i]))
				return false;
		}
		return true;
	}

	inline void ustring::StringReplace(std::wstring& strsrc, 
										const std::wstring& strsub,const std::wstring& strdst)
	{
		std::wstring::size_type pos = strsrc.find(strsub,0);
		if( pos == std::wstring::npos )
			return ;

		int len = strsub.size();
		std::wstring strtemp;
		strtemp.reserve(strsrc.size());
		strtemp.append(strsrc.c_str(),pos);
		strtemp.append(strdst);
		pos += len;
		int p = 0;
		while( (p = strsrc.find(strsub, pos)) != std::wstring::npos )
		{
			strtemp.append(strsrc.c_str() + pos, p - pos);
			strtemp.append(strdst);
			pos = p + len;
		}
		strtemp.append(strsrc.c_str()+pos);		
		strsrc = strtemp;
	}

	inline int ustring::GetStringBytes(const xstring &strText)
	{
		int nCount = 0;
		for(int i=0;i<strText.length();i++)
		{
			if(strText[i] < 0xFF && strText[i] >= 0)
				nCount += 1;
			else
				nCount += 2;
		}

		return nCount;
	}

	inline xstring ustring::GetSubStringByBytes(const xstring &strText,int nBytes)
	{
		if(strText.length() * 2 <= nBytes)
			return strText;

		xstring str;

		int nCount = 0;
		for(int i=0;i<strText.length();i++)
		{
			if(strText[i] < 0xFF && strText[i] >= 0)
				nCount += 1;
			else
				nCount += 2;

			if(nCount <= nBytes)
				str += strText[i];
		}

		return str;
	}
}
