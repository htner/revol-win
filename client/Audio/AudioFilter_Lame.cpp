#include "AudioFilter_Lame.h"
#include <assert.h>

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

CAudioDecoder_Lame::CAudioDecoder_Lame()
{
// 	m_hLameDecoder	= NULL;
// 	m_nFrameSize	= 0;
// 	m_nSampleRate	= 0;
// 	m_nChannle		= 2;
// 	m_decbuf.resize(LAME_MAXMP3BUFFER/2);
}
CAudioDecoder_Lame::~CAudioDecoder_Lame()
{
//	hip_decode_exit(m_hLameDecoder);
}

BOOL CAudioDecoder_Lame::Decode(const std::string& indata, std::string& outdata)
{
	return false;
// 	if(!m_hLameDecoder || m_nFrameSize == 0 || m_nSampleRate == 0)
// 		return false;
// 	if(m_nChannle != 1 && m_nChannle != 2)
// 		return false;
// 
// 	int nDecLen = hip_decode(m_hLameDecoder,(unsigned char*)indata.data(),indata.size(),
// 			(short*)m_decbuf.data(),(short*)(m_decbuf.data()+m_decbuf.size()/2));
// 	
// 	if(nDecLen > 0)
// 	{
// 		if(m_nChannle == 1)
// 		{
// 			m_data.append(m_decbuf.data(), nDecLen * 2);
// 		}
// 		else
// 		{
// 			std::string data;
// 			MergeChannle((short*)m_decbuf.data(), (short*)(m_decbuf.data()+m_decbuf.size()/2), nDecLen, data);
// 			m_data.append(data);
// 		}
// 	}
// 
// 	if(m_data.size() < m_nFrameSize * 2 * m_nChannle)
// 		return false;
// 
// 	outdata.assign(m_data.data(), m_nFrameSize * 2 * m_nChannle);
// 	m_data.erase(0, m_nFrameSize * 2 * m_nChannle);
	return true;
}

BOOL CAudioDecoder_Lame::DecodeLoss(std::string& out)
{
// 	if(m_data.size() >= m_nFrameSize * 2 * m_nChannle)
// 	{
// 		out.assign(m_data.data(), m_nFrameSize * 2 * m_nChannle);
// 		m_data.erase(0, m_nFrameSize * 2 * m_nChannle);
// 		
// 		return true;
// 	}else{
// 		out.resize(m_nFrameSize * 2 * m_nChannle);
// 		memset((void*)out.data(),0, m_nFrameSize * 2 * m_nChannle);
// 	}
	
	return false;
}

void CAudioDecoder_Lame::SetRate(int nFrameSize, int nSampleRate, int nChannle)
{
// 	if(m_nSampleRate != nSampleRate || m_nFrameSize != nFrameSize || m_nChannle != nChannle)
// 	{
// 		m_nSampleRate = nSampleRate;
// 		m_nFrameSize  = nFrameSize;
// 		m_nChannle	  = nChannle;
// 		hip_decode_exit(m_hLameDecoder);
// 		m_hLameDecoder = hip_decode_init();
// 	}
}

void CAudioDecoder_Lame::Destory()
{
// 	hip_decode_exit(m_hLameDecoder);
// 	m_hLameDecoder = NULL;
// 
// 	m_nFrameSize	= 0;
// 	m_nSampleRate	= 0;
// 	m_nChannle		= 2;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CAudioEncoder_Lame::CAudioEncoder_Lame()
{
	m_pLameEnc		= NULL;
	m_nFrameSize	= 0;
	m_nSampleRate	= 0;
	m_nChannle		= 0;
	m_encbuf.resize(LAME_MAXMP3BUFFER/2);
}

CAudioEncoder_Lame::~CAudioEncoder_Lame()
{
	lame_close(m_pLameEnc);
	m_pLameEnc = NULL;
}

BOOL CAudioEncoder_Lame::Encode(const std::string& indata, std::string& outdata)
{
	if(!m_pLameEnc || m_nFrameSize == 0 || m_nSampleRate == 0)
		return false;
	if(m_nChannle != 1 && m_nChannle != 2)
		return false;

	int nEncLen = 0; 
	if(m_nChannle == 1){
		nEncLen = lame_encode_buffer(m_pLameEnc, (short*)indata.data(),NULL, indata.size()/2,
										(unsigned char*)m_encbuf.data(), m_encbuf.size());
	}
	else{
		nEncLen = lame_encode_buffer_interleaved(m_pLameEnc,(short*)indata.data(),indata.size()/4, 
										(unsigned char*)m_encbuf.data(), m_encbuf.size());
	}

	if(nEncLen <= 0)
		return false;

	outdata.assign(m_encbuf.data(), nEncLen);

	return true;
}

void CAudioEncoder_Lame::SetRate(int nFrameSize, int nSampleRate, int nChannle)
{
	if(m_nSampleRate != nSampleRate || m_nFrameSize != nFrameSize || m_nChannle != nChannle)
	{
		m_nSampleRate = nSampleRate;
		m_nFrameSize  = nFrameSize;
		m_nChannle	  = nChannle;

		lame_close(m_pLameEnc);
		m_pLameEnc = lame_init();
		SetConfig(m_pLameEnc);
	}
}

void CAudioEncoder_Lame::Destory()
{
	lame_close(m_pLameEnc);
	m_pLameEnc = NULL;
}

void CAudioEncoder_Lame::SetConfig(lame_global_flags* pLameEnc)
{
	lame_set_in_samplerate(pLameEnc, m_nSampleRate);
	lame_set_out_samplerate(pLameEnc,44100);
	lame_set_num_channels(pLameEnc, m_nChannle);
	lame_set_quality(pLameEnc, 0);

	lame_set_VBR(pLameEnc, vbr_default);
	lame_set_VBR_quality(pLameEnc, 0.f);
	vbr_mode xx = lame_get_VBR(pLameEnc);
	
	if(m_nChannle == 1)
	{
		lame_set_VBR_max_bitrate_kbps(pLameEnc,78);
		lame_set_VBR_mean_bitrate_kbps(pLameEnc,64);
	}else{
		lame_set_VBR_max_bitrate_kbps(pLameEnc,400);
		lame_set_VBR_mean_bitrate_kbps(pLameEnc,320);
	}
	
	int nRes = lame_init_params(pLameEnc);

	
	int nBitRate = lame_get_VBR_mean_bitrate_kbps(pLameEnc);
	nBitRate = lame_get_VBR_max_bitrate_kbps(pLameEnc); 
}


LameMonoFactory::~LameMonoFactory()
{
	std::map<unsigned int ,IAudioEncoder*>::iterator eitr = __encoders.begin();
	for(;eitr != __encoders.end(); ++ eitr){
		eitr->second->Destory();
	}

	std::map<unsigned int ,IAudioDecoder*>::iterator ditr = __decoders.begin();
	for(;ditr != __decoders.end(); ++ ditr){
		ditr->second->Destory();
	}
}

void LameMonoFactory::ReleaseDecoder(bool isAll, unsigned int uid)
{
	if(isAll)
	{
		std::map<unsigned int ,IAudioDecoder*>::iterator ditr = __decoders.begin();
		for(;ditr != __decoders.end(); ++ ditr){
			ditr->second->Destory();
		}
		__decoders.clear();
	}else{
		std::map<unsigned int ,IAudioDecoder*>::iterator it = __decoders.find(uid);

		if(it != __decoders.end())
		{
			it->second->Destory();
			__decoders.erase(it);
		}
	}
}

IAudioEncoder* LameMonoFactory::GetEncoder(unsigned int uid)
{
	std::map<unsigned int ,IAudioEncoder*>::iterator itr = __encoders.find(uid);
	if(itr != __encoders.end()){
		return itr->second;
	}
	IAudioEncoder* encoder = new CAudioEncoder_LameMono();
	__encoders[uid] = encoder;
	return encoder;
}

IAudioDecoder* LameMonoFactory::GetDecoder(unsigned int uid)
{
	std::map<unsigned int ,IAudioDecoder*>::iterator itr = __decoders.find(uid);
	if(itr != __decoders.end()){
		return itr->second;
	}
	IAudioDecoder* decoder = new CAudioDecoder_LameMono();
	__decoders[uid] = decoder;
	return decoder;
}


//////////////////////////////////////////////////////////////////
LameStereoFactory::~LameStereoFactory()
{
	std::map<unsigned int ,IAudioEncoder*>::iterator eitr = __encoders.begin();
	for(;eitr != __encoders.end(); ++ eitr){
		eitr->second->Destory();
	}

	std::map<unsigned int ,IAudioDecoder*>::iterator ditr = __decoders.begin();
	for(;ditr != __decoders.end(); ++ ditr){
		ditr->second->Destory();
	}
}

void LameStereoFactory::ReleaseDecoder(bool isAll, unsigned int uid)
{
	if(isAll)
	{
		std::map<unsigned int ,IAudioDecoder*>::iterator ditr = __decoders.begin();
		for(;ditr != __decoders.end(); ++ ditr){
			ditr->second->Destory();
		}
		__decoders.clear();
	}else{
		std::map<unsigned int ,IAudioDecoder*>::iterator it = __decoders.find(uid);

		if(it != __decoders.end())
		{
			it->second->Destory();
			__decoders.erase(it);
		}
	}
}

IAudioEncoder* LameStereoFactory::GetEncoder(unsigned int uid)
{
	std::map<unsigned int ,IAudioEncoder*>::iterator itr = __encoders.find(uid);
	if(itr != __encoders.end()){
		return itr->second;
	}
	IAudioEncoder* encoder = new CAudioEncoder_LameStereo();
	__encoders[uid] = encoder;
	return encoder;
}

IAudioDecoder* LameStereoFactory::GetDecoder(unsigned int uid)
{
	std::map<unsigned int ,IAudioDecoder*>::iterator itr = __decoders.find(uid);
	if(itr != __decoders.end()){
		return itr->second;
	}
	IAudioDecoder* decoder = new CAudioDecoder_LameStereo();
	__decoders[uid] = decoder;
	return decoder;
}

};

