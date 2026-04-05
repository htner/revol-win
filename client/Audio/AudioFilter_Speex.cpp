#include "AudioFilter_Speex.h"
#include "speex/speex.h"
#include <math.h>

namespace audio
{
	extern void SplitChannle(const std::string& stereo, std::string &lCn, std::string& rCn);
	extern void MergeChannle(short* lCn,short* rCn,int nCount,std::string& stereo);

	SpeexProcessor::SpeexProcessor(void)
	{
		m_pSps[0]		= NULL;
		m_pSps[1]		= NULL;
		m_pSesEcho		= NULL;

		m_samplerate	= 0;
		m_framesize     = 0;
		m_isAGC			= false;
		m_isNS			= false;
	}

	void SpeexProcessor::SetRate(int framesize, int samplerate, int nChannels)
	{
		if(m_samplerate != samplerate || m_framesize != framesize || m_nChannles != nChannels)
		{
				m_nChannles		= nChannels;
				m_samplerate	= samplerate;
				m_framesize		= framesize;

				destroy();
				init();
		}
	}
	void SpeexProcessor::destroy()
	{
		if(NULL != m_pSps[0])
		{
			speex_preprocess_state_destroy(m_pSps[0]);
			m_pSps[0] = NULL;
		}

		if(NULL != m_pSps[1])
		{
			speex_preprocess_state_destroy(m_pSps[1]);
			m_pSps[1] = NULL;
		}

		if(NULL != m_pSesEcho)
		{
			speex_echo_state_destroy(m_pSesEcho);
		}
	}
	
	void SpeexProcessor::init()
	{
		m_pSps[0]			= speex_preprocess_state_init(m_framesize, m_samplerate);

		if(m_nChannles == 2)
			m_pSps[1]		= speex_preprocess_state_init(m_framesize, m_samplerate);

		m_vadVolume			= 30;
		m_nVADDelayCounter	= 0;
		
		EnableAGC(m_isAGC);
		EnableNS(m_isNS);
		CloseVAD();		
	}
	SpeexProcessor::~SpeexProcessor(void)
	{
		destroy();
	}	

	void SpeexProcessor::EnableDereVerb(BOOL isEnable)
	{			
		if(m_pSps[0])
			speex_preprocess_ctl(m_pSps[0], SPEEX_PREPROCESS_SET_DEREVERB, &isEnable);
		if(m_pSps[1])
			speex_preprocess_ctl(m_pSps[1], SPEEX_PREPROCESS_SET_DEREVERB, &isEnable);
	}

	void SpeexProcessor::EnableAGC(BOOL isAGC)
	{
		int nAGCMax = 29;
		int nAGCTarget = 30000;
		m_isAGC = isAGC;

		if(m_isAGC)
		{
			if(m_pSps[0]){
				speex_preprocess_ctl(m_pSps[0], SPEEX_PREPROCESS_SET_AGC, &m_isAGC);
				speex_preprocess_ctl(m_pSps[0], SPEEX_PREPROCESS_SET_AGC_TARGET, &nAGCTarget);
				speex_preprocess_ctl(m_pSps[0], SPEEX_PREPROCESS_SET_AGC_MAX_GAIN, &nAGCMax);
			}
			if(m_pSps[1]){
				speex_preprocess_ctl(m_pSps[1], SPEEX_PREPROCESS_SET_AGC, &m_isAGC);
				speex_preprocess_ctl(m_pSps[1], SPEEX_PREPROCESS_SET_AGC_TARGET, &nAGCTarget);
				speex_preprocess_ctl(m_pSps[1], SPEEX_PREPROCESS_SET_AGC_MAX_GAIN, &nAGCMax);
			}
		}else{
			if(m_pSps[0])
				speex_preprocess_ctl(m_pSps[0], SPEEX_PREPROCESS_SET_AGC, &m_isAGC);

			if(m_pSps[1])
				speex_preprocess_ctl(m_pSps[1], SPEEX_PREPROCESS_SET_AGC, &m_isAGC);
		}
	}

	void SpeexProcessor::EnableAEC(BOOL bEnable)
	{
	}
	void SpeexProcessor::DoAEC(const int16_t* speaker, int16_t* mic2out)
	{
		int16_t* out = new int16_t[m_framesize];
		speex_echo_cancellation(m_pSesEcho,mic2out, speaker, out);
		memcpy(mic2out, out, m_framesize * 2);
		delete []out;
	}
	void SpeexProcessor::OpenVAD(void)
	{
		m_checkVad = TRUE;
// 		int isVad = 1;
// 		int nStartProb = 99;
// 		int nContinue = 95;
// 
// 		if(m_pSps[0])
// 		{
// 			speex_preprocess_ctl(m_pSps[0], SPEEX_PREPROCESS_SET_VAD, &isVad);
// 			speex_preprocess_ctl(m_pSps[0], SPEEX_PREPROCESS_SET_PROB_START, &nStartProb);
// 			speex_preprocess_ctl(m_pSps[0], SPEEX_PREPROCESS_SET_PROB_CONTINUE, &nContinue);
// 		}
// 
// 		if(m_pSps[1])
// 		{
// 			speex_preprocess_ctl(m_pSps[1], SPEEX_PREPROCESS_SET_VAD, &isVad);
// 			speex_preprocess_ctl(m_pSps[1], SPEEX_PREPROCESS_SET_PROB_START, &nStartProb);
// 			speex_preprocess_ctl(m_pSps[1], SPEEX_PREPROCESS_SET_PROB_CONTINUE, &nContinue);
// 		}
	}

	void SpeexProcessor::CloseVAD(void)
	{
		m_checkVad = FALSE;
	}
	BOOL SpeexProcessor::Run(int16_t* in2out)
	{		
		BOOL isSpeech = false;
		int  nVadVolume = 35;

		if(m_isCustomVAD)
			nVadVolume = m_vadVolume;
		
		float sum=1.0f;
		for (int i=0;i<m_framesize;i++){
			sum += static_cast<float>(in2out[i] * in2out[i]);
		}
		
		float micLevel = sqrtf(sum / static_cast<float>(m_framesize));
		float dPeakMic=20.0f*log10f(micLevel / 32768.0f);
		if (dPeakMic < -96.0f)
			dPeakMic = -96.0f;

		float level =  (1.0f + dPeakMic / 96.0f);
		if(level > float((nVadVolume/2 + 30)/80.0f)){
			isSpeech  = true;
		}

		if(m_isAGC || m_isNS)
		{
			if(m_nChannles == 1){
				speex_preprocess_run(m_pSps[0], in2out);
			}else{
				std::string stereo((char*)in2out, m_framesize * 4);
				std::string lCn,rCn;
				SplitChannle(stereo, lCn, rCn);

				//isSpeech = 
				speex_preprocess_run(m_pSps[0], (short*)lCn.data());
				speex_preprocess_run(m_pSps[1],(short*)rCn.data());
				assert(rCn.size()/2 == m_framesize);
				MergeChannle((short*)lCn.data(),(short*)rCn.data(), rCn.size()/2, stereo);
				memcpy(in2out, stereo.data(), m_framesize * 4);
			}
		}

		if(TRUE == isSpeech)
			m_nVADDelayCounter = 0;
		else
			m_nVADDelayCounter++;

		if(m_nVADDelayCounter < 30)
			isSpeech = true;

		return isSpeech;
	}
	void SpeexProcessor::SetVADVolume(int volume, bool bCustomVAD)
	{
		if(NULL == m_pSps)
			return ;

		m_vadVolume = volume;
		m_isCustomVAD = bCustomVAD;
	}

	int SpeexProcessor::GetVadVolume(void)
	{
		if(NULL == m_pSps)
			return 0;

		return m_vadVolume;
	}

	void SpeexProcessor::EnableNS(BOOL isNS)
	{
		m_isNS = isNS;
		int nNS = -30;

		if(m_pSps[0])
		{
			speex_preprocess_ctl(m_pSps[0], SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &nNS);
			speex_preprocess_ctl(m_pSps[0], SPEEX_PREPROCESS_SET_DENOISE, &m_isNS);
		}

		if(m_pSps[1])
		{
			speex_preprocess_ctl(m_pSps[1], SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &nNS);
			speex_preprocess_ctl(m_pSps[1], SPEEX_PREPROCESS_SET_DENOISE, &m_isNS);
		}
	}

	/***********************************************************************************/
	SpeexEncoder::SpeexEncoder()
	{
		m_pEncoder = NULL;
	}
	SpeexEncoder::~SpeexEncoder()
	{
		destroy();
	}

	void SpeexEncoder::destroy()
	{
		if(NULL != m_pEncoder)
		{
			speex_encoder_destroy(m_pEncoder);
			m_pEncoder = NULL;
		}
	}

	void SpeexEncoder::init()
	{
		int mode = SpeexMode::GetMode(m_samplerate);
		m_pEncoder = speex_encoder_init(speex_lib_get_mode(mode));

		int iArg = 5;
		//speex_encoder_ctl(m_pEncoder,SPEEX_SET_COMPLEXITY, &iArg);
	}

	BOOL SpeexEncoder::Encode(const binString& in, binString& out)
	{	
		char buffer[200];
		ZeroMemory(buffer, sizeof(char) * 200); 

		SpeexBits bits;
		speex_bits_init(&bits);
		speex_bits_reset(&bits);

		speex_encode_int(m_pEncoder, (spx_int16_t*)in.data(), &bits);

		int nbBytes = speex_bits_write(&bits, buffer, 200);

		out.assign(buffer, nbBytes);

		speex_bits_destroy(&bits);
		return TRUE;
	}
	void SpeexEncoder::EnableVBR(BOOL bEnable)
	{
		if(NULL == m_pEncoder)
			return ;
		int arg = bEnable ? 1 : 0;
		speex_encoder_ctl(m_pEncoder, SPEEX_SET_VBR, &arg);
	}

	void SpeexEncoder::SetQuality(int quality)
	{
		if(NULL == m_pEncoder)
			return ;
		speex_encoder_ctl(m_pEncoder, SPEEX_SET_QUALITY, &quality);
	}
	void SpeexEncoder::SetRate(int framesize, int channel, int samplerate)
	{
		if(m_samplerate != samplerate || m_framesize != framesize)
		{
				m_samplerate = samplerate;
				m_framesize  = framesize;
				destroy();
				init();
		}
	}

	/***********************************************************************************/

	SpeexDecoder::SpeexDecoder()
	{
		m_pDecoder		= NULL;
		m_samplerate	= 0;
		m_framesize     = 0;
	}
	SpeexDecoder::~SpeexDecoder()
	{
		destroy();
	}
	void SpeexDecoder::destroy()
	{
		if(NULL != m_pDecoder)
		{
			speex_decoder_destroy(m_pDecoder);
			m_pDecoder = NULL;
		}
	}

	void SpeexDecoder::init()
	{
		int mode = SpeexMode::GetMode(m_samplerate);
		m_pDecoder = speex_decoder_init(speex_lib_get_mode(mode));
		int enhance = 1;
		speex_decoder_ctl(m_pDecoder, SPEEX_SET_ENH, &enhance);
	}
	BOOL SpeexDecoder::Decode(const binString& in, binString& out)
	{
		if(NULL == m_pDecoder)
			return false;

		short* buffer = new short[m_framesize];
		ZeroMemory(buffer, m_framesize*2);
		SpeexBits bits;
		speex_bits_init(&bits);
		
		speex_bits_read_from(&bits, (char *)in.data(), in.size());
		speex_decode_int(m_pDecoder, &bits, buffer);

		out.assign((char*)buffer, m_framesize*2);
		speex_bits_destroy(&bits);

		delete[] buffer;
		return true;
	}
	void SpeexDecoder::SetRate(int framesize, int channel, int samplerate)
	{
		if(m_samplerate != samplerate || m_framesize != framesize)
		{
				m_samplerate = samplerate;
				m_framesize  = framesize;
				destroy();
				init();
		}
			
	}

	BOOL SpeexDecoder::DecodeLoss(binString& out)
	{
		if(NULL == m_pDecoder)
			return FALSE;

		short* buffer = new short[m_framesize];
		ZeroMemory(buffer, m_framesize*2);

		speex_decode_int(m_pDecoder, NULL, buffer);
		out.assign((char*)buffer, m_framesize*2);
		
		delete[] buffer;
		return true;
	}

	void SpeexPacker::WriteDelayLog(int delay)
	{
#if 0
		if(!__isLog || 0 != __unpackCount % 100)
			return;

		FILE* pf = fopen("delay.log","w+");

		if(pf)
		{
			fprintf(pf,"%d \n",delay);
			fclose(pf);
		}
		char arrBuf[512] = {0};
		sprintf(arrBuf, "%d \n", delay);
		::OutputDebugStringA(arrBuf);
#endif
	}

	bool SpeexPacker::pack(const framelist& frames, std::string& voice)
	{
		for(int i=0; i<frames.size(); i++){
			voice += frames[i];
		}
		sox::PackBuffer bf;
		sox::Pack pk(bf);
		uint32_t time = 5353;
		pk << time << voice;
		voice =  std::string(pk.data(), pk.size());
		return true;
	}
	bool SpeexPacker::unpack(const std::string& voice, framelist& frames)
	{
		std::string tmpVc;
		if(!voice.empty())
		{
			try{
				sox::Unpack up(voice.data(), voice.size());
				uint32_t aaa;
				up >> aaa >> tmpVc;

				WriteDelayLog(timeGetTime() - aaa);
			}catch(sox::UnpackError& e){
				// log unpack error
				//	return false;
			}
		}

		int framesize = tmpVc.size()/2;
		for(int i=0; i<2; ++i){
			std::string frame;
			frame.assign(tmpVc.data() + i*framesize, framesize);
			frames.push_back(frame);
		}
		return true;
	}

	

	/******************************************************************************/

SpeexFactory::~SpeexFactory()
{
		std::map<unsigned int ,IAudioEncoder*>::iterator eitr = __encoders.begin();
		for(;eitr != __encoders.end(); ++ eitr){
				eitr->second->Destory();
		}

		std::map<unsigned int ,IAudioDecoder*>::iterator ditr = __decoders.begin();
		for(;ditr != __decoders.end(); ++ ditr){
				ditr->second->Destory();
		}

		std::map<unsigned int ,IAudioProcessor*>::iterator pitr = __prosessors.begin();
		for(;pitr != __prosessors.end(); ++ pitr){
				pitr->second->Destory();
		}

}

void SpeexFactory::ReleaseDecoder(bool isAll,unsigned int uid)
{
	if(isAll)
	{
		std::map<unsigned int,IAudioDecoder*>::iterator ditr = __decoders.begin();
		for(;ditr != __decoders.end(); ++ ditr){
			ditr->second->Destory();
		}
		__decoders.clear();
	}else{
		std::map<unsigned int,IAudioDecoder*>::iterator it = __decoders.find(uid);

		if(it != __decoders.end())
		{
			it->second->Destory();
			__decoders.erase(it);
		}
	}
}

IAudioEncoder*	SpeexFactory::GetEncoder(unsigned int uid)
{
		std::map<unsigned int,IAudioEncoder*>::iterator itr = __encoders.find(uid);
		if(itr != __encoders.end()){
			return itr->second;
		}
		IAudioEncoder* encoder = new SpeexEncoder();
		__encoders[uid] = encoder;
		return encoder;
}
IAudioDecoder*	SpeexFactory::GetDecoder(unsigned int uid)
{
		std::map<unsigned int,IAudioDecoder*>::iterator itr = __decoders.find(uid);
		if(itr != __decoders.end()){
			return itr->second;
		}
		IAudioDecoder* decoder = new SpeexDecoder();
		__decoders[uid] = decoder;
		return decoder;
}
IAudioProcessor* SpeexFactory::GetProcessor(unsigned int uid)
{		
		std::map<unsigned int,IAudioProcessor*>::iterator itr = __prosessors.find(uid);
		if(itr != __prosessors.end()){
			return itr->second;
		}
		IAudioProcessor* processor = new SpeexProcessor();
		__prosessors[uid] = processor;
		return processor;
}
IAudioPacker* SpeexFactory::GetPacker()
{
	static SpeexPacker packer;
	return &packer;
}


IAudioEncoder*	SpeexVbrFactory::GetEncoder(unsigned int uid)
{
	std::map<unsigned int,IAudioEncoder*>::iterator itr = __encoders.find(uid);
	if(itr != __encoders.end()){
		return itr->second;
	}
	IAudioEncoder* encoder = new SpeexEncoderVBR();
	__encoders[uid] = encoder;
	return encoder;
}

IAudioPacker* SpeexVbrFactory::GetPacker()
{
	static DefaultPacker packer;
	return &packer;
}
	
}