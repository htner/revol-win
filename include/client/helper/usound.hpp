#include "stdafx.h"

#include "pubfunc/usound.h"

#include <cmath>

namespace helper
{
	int usound::ToVisibleVolume(int vol)
	{
		double tmp	= pow((float)vol,(float)0.333333) * 21;
		return (int)tmp;
	}
	int usound::CalcSoundVolume(const char* p, UINT len)
	{
		if(len == 0)
			return 0;
		int framemax	= 0;
		int total		= 0;
		for(int i=0; i<(int)(len/sizeof(short)); ++i)
		{
			short* pcm	= (short*)p;
			pcm			+= i;
			if (*pcm < 0)
				total -= *pcm;
			else
				total += *pcm;
			if(abs(*pcm) > framemax)
				framemax = *pcm;
		}
		return total / ((int)len/sizeof(short));
	}
}
