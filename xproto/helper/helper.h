#ifndef HELPER_H_SESSION_
#define HELPER_H_SESSION_

#include "third/openssl/md5.h"
#include "third/openssl/sha.h"
#include "common/soxhelper.h"

class CAutoLock
{
public:
	CAutoLock( LPCRITICAL_SECTION cs )
	{
		_cs = cs;
		EnterCriticalSection(cs);
	}
	~CAutoLock()
	{
		LeaveCriticalSection(_cs);
	}
	LPCRITICAL_SECTION _cs;
};

static inline int GetStampMS()
{
	SYSTEMTIME st;   
	unsigned int stamp;

	GetLocalTime(&st);
	stamp = st.wHour*60*60 + st.wMinute*60 + st.wSecond;
	stamp = stamp*1000 + st.wMilliseconds;
	return stamp;
}


static inline std::string GetShowTime()
{
	static char s[255];
	SYSTEMTIME st;
	GetSystemTime(&st);

	sprintf( s, "<--> %d %02d:%02d:%02d  %04d <-->",
		st.wDay,
		st.wHour + 8,
		st.wMinute,
		st.wSecond,
		st.wMilliseconds  );
	return s;

}

inline std::string sha1sum(void* pszBuffer, int len)
{
    unsigned char digest[SHA_DIGEST_LENGTH + 1] = {0};
    SHA1((unsigned char *)pszBuffer, len, digest);
	return sox::bin2hex(digest, SHA_DIGEST_LENGTH);
}

inline std::string sha1sum(const std::string& str)
{
    return sha1sum((unsigned char *)str.c_str(), str.size());
}

inline std::string sha1sum(const std::wstring& str)
{
    return sha1sum((unsigned char *)str.c_str(), str.size() * sizeof(wchar_t));
}


#endif