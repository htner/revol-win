#pragma once

class CalcRuntime
{
public:
	CalcRuntime(LPCTSTR name,int dwThreshold)
	{
		_dwTick = GetTickCount();
		_name = name;
		_dwThreshold = dwThreshold;
	}
	~CalcRuntime()
	{
		int offset = GetTickCount() - _dwTick;
		if( offset > _dwThreshold )
		{
			ATLTRACE(_T("runtime[%s]: %dms\n"),_name.c_str(),offset);
		}
	}
	DWORD			_dwTick;
	int				_dwThreshold;
#ifdef UNICODE
	std::wstring	_name;
#else
	std::string		_name;
#endif
};