#pragma once

class CFreqUsage
{
public:
	CFreqUsage()
	{
		_dwTriger = 0;
		_dwTrigerTotal = 0;
		_dwTickLast = GetTickCount();
	}
	inline int Triger()
	{
		_dwTrigerTotal++;
		
		DWORD dwTick = GetTickCount();
		int slaps = (dwTick - _dwTickLast);
		if( slaps >= 1000){
			_dwTriger = _dwTrigerTotal * 1000 / slaps;

			_dwTrigerTotal = (_dwTrigerTotal >>1);
			DWORD dwTickLast = ((_dwTickLast >> 1) + (dwTick >>1));
			_dwTickLast = dwTickLast;
			
		}

		return _dwTriger;
	}
	inline void TrigerSkip(){
		_dwTrigerTotal--;		
	}	
public:
	DWORD _dwTickLast;
	DWORD _dwTrigerTotal;
	DWORD _dwTriger;
};

#define DEFINE_FREQ(name)	static CFreqUsage g_##name_freq;
#define GET_FREQ(name)		g_##name_freq._dwTriger
#define INC_FREQ(name)		g_##name_freq.Triger();
#define IF_TRIGER_FREQ_RETURN(name,c)	if(g_##name_freq.Triger() >= c) { g_##name_freq.TrigerSkip();ATLTRACE(_T("TRIGER_FREQ[%s,%d] return...\n"),_T(#name),c); return;}
#define IF_TRIGER_FREQ_RETURN_VALUE(name,c,v)	if(g_##name_freq.Triger() >= c) { g_##name_freq.TrigerSkip();ATLTRACE(_T("TRIGER_FREQ[%s,%d] return...\n"),_T(#name),c); return v;}

