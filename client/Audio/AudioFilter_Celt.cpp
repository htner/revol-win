#include "AudioFilter_Celt.h"
#include <atlsync.h>
using namespace audio;

#define MAX_FRAME_SIZE 512
CCriticalSection s_celtLock;

CeltDecoder::CeltDecoder()
{
		__mode		= NULL;
		__decoder	= NULL;
}
CeltDecoder::~CeltDecoder()
{
	destroy();
}
void CeltDecoder::SetRate(int nFrameSize, int nSampleRate,int nChannels)
{
		if(m_nSampleRate != nSampleRate || m_nFrameSize != nFrameSize || m_nChannels != nChannels)
		{
				m_nSampleRate = nSampleRate;
				m_nFrameSize  = nFrameSize;
				m_nChannels   = nChannels;
				destroy();
				init();
		}
}

void	CeltDecoder::init()
{
		__mode	  = celt_mode_create(m_nSampleRate,  m_nFrameSize/m_nChannels, NULL);
		__decoder = celt_decoder_create_custom(__mode, m_nChannels, NULL);
}
void	CeltDecoder::destroy()
{
		if(__decoder) celt_decoder_destroy(__decoder);
		if(__mode)	 celt_mode_destroy(__mode);
}
BOOL	CeltDecoder::Decode(const binString& in, binString& out)
{		
		if(!__decoder)
			return FALSE;
		short* buffer = new short[m_nFrameSize];

		s_celtLock.Enter();
		int nLen = celt_decode(__decoder, (const unsigned char *)in.data(), in.length(), (celt_int16 *)buffer,m_nFrameSize/m_nChannels);
		s_celtLock.Leave();

		out.append((const char*)buffer ,m_nFrameSize*2);
		delete[] buffer;
		return TRUE;
}
BOOL	CeltDecoder::DecodeLoss(binString& out)
{
		short* buffer = new short [m_nFrameSize];
		ZeroMemory(buffer, m_nFrameSize);	
		s_celtLock.Enter();
		celt_decode(__decoder, NULL, MAX_FRAME_SIZE, buffer, m_nFrameSize/m_nChannels);
		s_celtLock.Leave();
		out.assign((char*)buffer, m_nFrameSize*2);
		delete[] buffer;
		return TRUE;
}

/*********************************************************************************************************/

CeltEncoder::CeltEncoder()
{
		__mode		= NULL;
		__encoder	= NULL;
}
CeltEncoder::~CeltEncoder()
{
	destroy();
}
void CeltEncoder::SetRate(int nFrameSize, int nSampleRate,int nChannels)
{
	if(m_nSampleRate != nSampleRate || m_nFrameSize != nFrameSize || m_nChannels != nChannels)
	{
			m_nSampleRate = nSampleRate;
			m_nFrameSize  = nFrameSize;
			m_nChannels   = nChannels;
			destroy();
			init();
	}
}
void CeltEncoder::destroy()
{
	if(__encoder) celt_encoder_destroy(__encoder);
	if(__mode)	 celt_mode_destroy(__mode);
}

void CeltEncoder::init()
{
	__mode	  = celt_mode_create(m_nSampleRate,  m_nFrameSize/m_nChannels, NULL);
	__encoder = celt_encoder_create_custom(__mode, m_nChannels, NULL);

	celt_encoder_ctl(__encoder, CELT_SET_BITRATE(1024*3.5 * m_nChannels * 8));
	celt_encoder_ctl(__encoder, CELT_SET_VBR(1));
}


BOOL CeltEncoder::Encode(const binString& in, binString& out)
{
	unsigned char buffer[MAX_FRAME_SIZE];

	s_celtLock.Enter();
	int len = celt_encode(__encoder, (const celt_int16 *)in.data(), m_nFrameSize/m_nChannels, buffer, MAX_FRAME_SIZE);
	s_celtLock.Leave();

	if(len > 0){
		out.append((const char*)buffer, len);
	}
	return true;
}

void CeltEncoder::SetQuality(int quality){}

/**************************************************************************************************/


Celt11StereoFactory::~Celt11StereoFactory()
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

void Celt11StereoFactory::ReleaseDecoder(bool isAll, unsigned int uid)
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

IAudioEncoder*	Celt11StereoFactory::GetEncoder(unsigned int uid)
{
	std::map<unsigned int ,IAudioEncoder*>::iterator itr = __encoders.find(uid);
	if(itr != __encoders.end()){
		return itr->second;
	}
	IAudioEncoder* encoder = new Celt11EncoderStereo();
	__encoders[uid] = encoder;
	return encoder;
}
IAudioDecoder*	Celt11StereoFactory::GetDecoder(unsigned int uid)
{
	std::map<unsigned int ,IAudioDecoder*>::iterator itr = __decoders.find(uid);
	if(itr != __decoders.end()){
		return itr->second;
	}
	IAudioDecoder* decoder = new Celt11DecoderStereo();
	__decoders[uid] = decoder;
	return decoder;
}
IAudioProcessor* Celt11StereoFactory::GetProcessor(unsigned int uid)
{
	return NULL;
}


/////////////////////////////////////////////////////////////////////////
