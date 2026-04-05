#include <assert.h>
#include <string>

namespace audio
{
void MergeChannle(short* lCn,short* rCn,int nCount,std::string& stereo)
{
	stereo.resize(nCount * 4);
	short* pData = (short*)stereo.data();
	for(int i = 0; i < nCount; i++)
	{
		*pData++ = *lCn++;
		*pData++ = *rCn++;
	}
}

void SplitChannle(const std::string& stereo, std::string &lCn, std::string& rCn)
{
	int nLen = stereo.size()/4;
	lCn.resize(nLen * 2);
	rCn.resize(nLen * 2);
	short* pwStereo = (short*)stereo.data();
	short* pwL = (short*)lCn.data();
	short* pwR = (short*)rCn.data();

	for(int i = 0; i < nLen; i++)
	{
		*pwL++ = *pwStereo++;
		*pwR++ = *pwStereo++;
	}
}

};

