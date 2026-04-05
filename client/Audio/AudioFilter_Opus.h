#ifndef __AUDIO_FILTER_OPUS_H__
#define __AUDIO_FILTER_OPUS_H__


#include "audio/iaudiocodec.h"
#include "common/xstring.h"
#include "audio/audiopacker.h"
#include "audio/IOpusCodec.h"

namespace audio{
	class OpusDecoder : public IAudioDecoder
	{
	public:
		OpusDecoder();
		virtual ~OpusDecoder();
		virtual BOOL Decode(const binString& in, binString& out) ;
		virtual BOOL DecodeLoss(binString& out);
		virtual void SetRate(int framesize, int channel, int samplerate);
		virtual void Destory(){
			delete this;
		}

	protected:
		IOpusDec*	_piDec;
		bool		_firstDec;
	};

	class OpusEncoder: public IAudioEncoder
	{
	public:
		OpusEncoder();
		virtual ~OpusEncoder();
		virtual void SetQuality(int quality);
		virtual BOOL Encode(const binString& in, binString& out);
		virtual void SetRate(int framesize, int channel, int samplerate);
		virtual void Destory(){
			delete this;
		}
	protected:
		IOpusEnc*	_piEnc;
	};

	class OpusFactory : public IAudioFactory
	{
	public:
		virtual ~OpusFactory();
		virtual IAudioEncoder*		GetEncoder(unsigned int uid);
		virtual IAudioDecoder*		GetDecoder(unsigned int uid);
		virtual IAudioProcessor*	GetProcessor(unsigned int uid);
		virtual void				ReleaseDecoder(bool isAll,unsigned int uid);
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

#endif //__AUDIO_FILTER_OPUS_H__
