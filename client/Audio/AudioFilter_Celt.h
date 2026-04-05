#pragma once

#include "macros.h"
#include "audio/samples.h"
#include "audio/iaudiocodec.h"
#include "celt/celt.h"
#include "common/xstring.h"
#include "audio/audiopacker.h"

namespace audio{	
	class CeltDecoder
	{
	public:
		CeltDecoder();
		virtual ~CeltDecoder();
		virtual BOOL Decode(const binString& in, binString& out) ;
		virtual BOOL DecodeLoss(binString& out);
		virtual void SetRate(int framesize, int samplerate, int nChannels);
	protected:
		void	init();
		void	destroy();
	protected:
		CELTDecoder*	__decoder;
		CELTMode*		__mode;
		int				m_nSampleRate;
		int				m_nFrameSize;
		int				m_nChannels;
	};

	class CeltEncoder
	{
	public:
		CeltEncoder();
		virtual ~CeltEncoder();
		virtual void SetQuality(int quality);
		virtual BOOL Encode(const binString& in, binString& out);
		virtual void SetRate(int framesize, int samplerate, int nChannels);
	protected:
		void	init();
		void	destroy();
	protected:
		CELTEncoder*	__encoder;
		CELTMode*		__mode;
		int				m_nSampleRate;
		int				m_nFrameSize;
		int				m_nChannels;
	};

	class Celt11EncoderMono:public IAudioEncoder
	{
	public:
		void SetQuality(int nQuality){m_cCeltEnc.SetQuality(nQuality);}
		BOOL Encode(const binString& in, binString& out){return m_cCeltEnc.Encode(in,out);}
		void SetRate(int nFrameSize, int nSampleRate){m_cCeltEnc.SetRate(nFrameSize, nSampleRate, 1);}
		void Destory(){delete this;}				
	private:
		CeltEncoder	m_cCeltEnc;
	};

	class Celt11EncoderStereo:public IAudioEncoder
	{
	public:
		void SetQuality(int nQuality){m_cCeltEnc.SetQuality(nQuality);}
		BOOL Encode(const binString& in, binString& out){return m_cCeltEnc.Encode(in,out);}
		void SetRate(int nFrameSize, int nSampleRate){m_cCeltEnc.SetRate(nFrameSize, nSampleRate, 2);}
		void Destory(){delete this;}				
	private:
		CeltEncoder	m_cCeltEnc;
	};

	class Celt11DecoderMono:public IAudioDecoder
	{
	public:
		BOOL Decode(const binString& in, binString& out){return m_cCeltDec.Decode(in,out);}
		BOOL DecodeLoss(binString& out){return m_cCeltDec.DecodeLoss(out);}
		void SetRate(int nFrameSize, int nSampleRate){m_cCeltDec.SetRate(nFrameSize, nSampleRate,1);}
		void Destory(){delete this;}
	private:
		CeltDecoder m_cCeltDec;
	};

	class Celt11DecoderStereo:public IAudioDecoder
	{
	public:
		BOOL Decode(const binString& in, binString& out){return m_cCeltDec.Decode(in,out);}
		BOOL DecodeLoss(binString& out){return m_cCeltDec.DecodeLoss(out);}
		void SetRate(int nFrameSize, int nSampleRate){m_cCeltDec.SetRate(nFrameSize, nSampleRate,2);}
		void Destory(){delete this;}
	private:
		CeltDecoder m_cCeltDec;
	};

	class Celt11StereoFactory : public IAudioFactory
	{
	public:
		virtual ~Celt11StereoFactory();
		virtual IAudioEncoder*		GetEncoder(unsigned int uid);
		virtual IAudioDecoder*		GetDecoder(unsigned int uid);
		virtual IAudioProcessor*	GetProcessor(unsigned int uid);
		virtual void				ReleaseDecoder(bool isAll, unsigned int uid);

		IAudioPacker* GetPacker()
		{
			static DefaultPacker packer;
			return &packer;
		}
	protected:
		std::map<unsigned int ,IAudioEncoder*>	__encoders;
		std::map<unsigned int ,IAudioDecoder*>	__decoders;
	};
}