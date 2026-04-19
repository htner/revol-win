#pragma once


class HttpCache
{
public:
	static void ClearUrlCache(LPCTSTR lpUrl)
	{
		if( !lpUrl )return;

		DWORD dwTotalSize = 1024 * 4;
		DWORD dwTrySize = dwTotalSize;
		LPINTERNET_CACHE_ENTRY_INFO lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwTotalSize];
		ZeroMemory(lpCacheEntry,dwTotalSize);
		lpCacheEntry->dwStructSize = dwTrySize;

		int nUrlLen = _tcslen(lpUrl);
		HANDLE hCacheDir = FindFirstUrlCacheEntry(NULL, lpCacheEntry,&dwTrySize);
		if( hCacheDir )
		{		
			dwTrySize = dwTotalSize;		
			lpCacheEntry->dwStructSize = dwTrySize;
			while(FindNextUrlCacheEntry(hCacheDir, lpCacheEntry, &dwTrySize))
			{
				if(lpCacheEntry->CacheEntryType & NORMAL_CACHE_ENTRY)
				{
					int nSrcLen = _tcslen(lpCacheEntry->lpszSourceUrlName);					
					if( nSrcLen >= nUrlLen && _tcsnicmp(lpUrl,lpCacheEntry->lpszSourceUrlName,nUrlLen) == 0)
					{
						DeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
						break;
					}
				}

				dwTrySize = dwTotalSize;		
				lpCacheEntry->dwStructSize = dwTrySize;

			}
			FindCloseUrlCache(hCacheDir); 
		}

		delete[] lpCacheEntry;
	}
};