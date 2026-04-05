#pragma once

#include "audio/iaudiocodec.h"
#include "speex/speex_preprocess.h"
#include "speex/speex.h"
#include "speex/speex_echo.h"
#include "common/xstring.h"
#include "audio/audiopacker.h"

namespace audio
{
	class SpeexProcessor : public IAudioProcessor
	{
	public:
		SpeexProcessor();
		virtual ~SpeexProcessor();
		 virtual void		SetRate(int framesize, int samplerate, int nChannels);
		 virtual void		SetVADVolume(int volume, bool isCustomVAD);
		 virtual int		GetVadVolume(void);		
		 virtual void		CloseVAD(void);			
		 virtual void		OpenVAD(void);			
		 virtual void		EnableAGC(BOOL isAGC);			
		 virtual void		EnableNS(BOOL isNS);
		 
		 virtual BOOL		Run(int16_t* inout);
		 virtual void Destory(){
			 delete this;
		 }
		 void  EnableAEC(BOOL bEnable);
		 void  DoAEC(const int16_t* speaker, int16_t* mic2out);

	protected:
		 void				init();
		 void				destroy();
		 void				EnableDereVerb(BOOL isEnable);
	protected:
		SpeexPreprocessState* m_pSps[2];
		SpeexEchoState		* m_pSesEcho;
		int				m_nVADDelayCounter;
		int				m_vadVolume;
		bool			m_isCustomVAD;
		
		int				m_samplerate;
		int				m_framesize;
		int				m_nChannles;
		BOOL			m_checkVad;
		BOOL			m_isAGC;
		BOOL			m_isNS;
	};

	struct SpeexMode
	{
		static int GetMode(int samplerate)
		{
			switch(samplerate)
			{
			case SAMPLE_RATE_32000:
				 return SPEEX_MODEID_UWB;
			case SAMPLE_RATE_16000:
				 return SPEEX_MODEID_WB;
			 case SAMPLE_RATE_8000:
				 return SPEEX_MODEID_NB;
			 default:
				 return 0;
			}			
		}
	};

	class SpeexDecoder : public IAudioDecoder
	{
	public:
		SpeexDecoder();
		virtual ~SpeexDecoder();
		virtual BOOL Decode(const binString& in, binString& out) ;
		virtual BOOL DecodeLoss(binString& out);
		virtual void SetRate(int framesize, int channel, int samplerate);
		virtual void Destory(){
			 delete this;
		 }
	protected:
		 void				init();
		 void				destroy();
	protected:
		void*	m_pDecoder;
		int		m_samplerate;
		int		m_framesize;
	};

	class SpeexEncoder: public IAudioEncoder
	{
	public:
		SpeexEncoder();
		virtual ~SpeexEncoder();
		virtual void SetQuality(int quality);
		virtual BOOL Encode(const binString& in, binString& out);
		virtual void SetRate(int framesize, int channel, int samplerate);
		virtual void Destory(){
			 delete this;
		}
		virtual void EnableVBR(BOOL bEnable);
	protected:
		 virtual void		init();
		 void				destroy();
	protected:
		void*	m_pEncoder;
		int		m_samplerate;
		int		m_framesize;
	};

	class SpeexEncoderVBR : public SpeexEncoder
	{
	protected:
		virtual void init(){
			SpeexEncoder::init();
			EnableVBR(true);
		}
	};

	class SpeexPacker : public IAudioPacker
	{
	public:
		SpeexPacker():__unpackCount(0)
		{
#if 0
			extern int GetRcEnvConfigInt(LPCTSTR lpSesstion,LPCTSTR lpKey,int nDefValue);
			__isLog = GetRcEnvConfigInt(_T("RCAudio"), _T("delayLog"), 0);
#endif
		}
		virtual bool pack(const framelist& frames, std::string& voice) ;
		virtual bool unpack(const std::string& voice, framelist& frames);
	private:
		void WriteDelayLog(int delay);
		
		unsigned int __unpackCount;
		bool  __isLog;
	};

	class SpeexFactory : public IAudioFactory
	{
	public:
		virtual ~SpeexFactory();
		virtual IAudioEncoder*		GetEncoder(unsigned int uid);
		virtual IAudioDecoder*		GetDecoder(unsigned int uid);
		virtual IAudioProcessor*	GetProcessor(unsigned int uid);
		virtual IAudioPacker*		GetPacker();
		virtual void				ReleaseDecoder(bool isAll,unsigned int uid);
	protected:
		std::map<unsigned int ,IAudioEncoder*>	__encoders;
		std::map<unsigned int ,IAudioDecoder*>	__decoders;
		std::map<unsigned int ,IAudioProcessor*>	__prosessors;
	};

	class SpeexVbrFactory : public SpeexFactory
	{
	public:
		virtual IAudioEncoder*		GetEncoder(unsigned int uid);
		virtual IAudioPacker*		GetPacker();
	};
}