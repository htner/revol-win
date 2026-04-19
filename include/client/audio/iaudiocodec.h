#pragma once 

/*
*** audio codec include
*** support g729
*** and support add more filter, just like g723, speex, and more

*/



#include <vector>
#include "macros.h"
#include "samples.h"



namespace audio
{
	enum ACT
	{
		//ACT_G729			= 0,
		//ACT_G723			= 1,
		ACT_SPEEX			= 2,
		ACT_CELT			= 3,
		ACT_LAME_MONO		= 4,
		ACT_LAME_STEREO		= 5,
		ACT_SPEEX_VBR		= 6,
		ACT_CELT11_STEREO	= 7,
		ACT_HEAAC			= 8,
		ACT_HEAAC4K         = 9,
		ACT_OPUS_MONO		= 10,
		ACT_OPUS_STEREO		= 11,
	};

	typedef __int16 int16_t;

	struct IAudioProcessor
	{
		virtual void		SetRate(int framesize, int samplerate, int nChannels) = 0;
		virtual void		SetVADVolume(int volume,bool bCustomVAD) = 0;
		virtual int			GetVadVolume(void)		 = 0;
		virtual void		CloseVAD(void)			 = 0;
		virtual void		OpenVAD(void)			 = 0;
		virtual void		EnableAGC(BOOL isAGC)	 = 0;			
		virtual void		EnableNS(BOOL isNS)		 = 0;	
		virtual BOOL		Run(int16_t* in2out)	 = 0;
		virtual void		Destory()				 = 0;
	};

	struct IAudioDecoder
	{
		virtual BOOL Decode(const binString& in, binString& out) = 0;
		virtual BOOL DecodeLoss(binString& out)					 = 0;
		virtual void SetRate(int framesize, int channel, int samplerate)		= 0;
		virtual void Destory()									= 0;
	};

	struct IAudioEncoder
	{
		virtual void SetQuality(int quality)						= 0;
		virtual BOOL Encode(const binString& in, binString& out)	= 0;
		virtual void SetRate(int framesize, int channel, int samplerate) = 0;
		virtual void Destory()										= 0;
	};

	typedef std::vector<std::string> framelist;
	struct IAudioPacker
	{
		virtual bool pack(const framelist& frames, std::string& voice) = 0;
		virtual bool unpack(const std::string& voice, framelist& frames) = 0;
	};

	struct IAudioFactory
	{
		virtual IAudioEncoder*		GetEncoder(unsigned int uid)   = 0;
		virtual IAudioDecoder*		GetDecoder(unsigned int uid)   = 0;
		virtual IAudioProcessor*	GetProcessor(unsigned int uid) = 0;
		virtual IAudioPacker*		GetPacker()				  = 0;
		virtual void				ReleaseDecoder(bool isAll,unsigned int uid)=0;
	};
	
};

audio::IAudioFactory* CoGetAudioFactory(int type);




