// AudioCodec.cpp : 定义 DLL 应用程序的入口点。
//

#include "AudioResamplerFactor.h"
#include "AudioFilter_Speex.h"
#include "AudioFilter_Celt.h"
//#include "AudioFilter_Lame.h"
//#include "AudioFilter_AAC.h"
#include "AudioFilter_Opus.h"


audio::CAudioResamplerFactor*	 AudioResampler_Factor = NULL;

audio::IAudioResampler* AudioCreateResampler(int id)
{
	if(NULL == AudioResampler_Factor)
		AudioResampler_Factor = new audio::CAudioResamplerFactor();
	return AudioResampler_Factor->Create(id);
}

void ReleaseResampler(int id)
{
	if(NULL != AudioResampler_Factor)
		AudioResampler_Factor->Destroy(id);
}

void AudioResamplerDestroy(void)
{
	SAFE_DELETE(AudioResampler_Factor);
}

audio::IAudioFactory* CoGetAudioFactory(int type)
{
	static audio::SpeexFactory			__speexFactory;
	//static audio::Celt11StereoFactory	__celtFactory;
	//static audio::LameStereoFactory		__LameStereoFactory;
	//static audio::LameMonoFactory		__LameMonoFactory;
	static audio::SpeexVbrFactory		__speexVBRFactory;
	//static audio::AACFactory            __AACFactory;
	//static audio::AAC4KFactory			__AAC4KFactory;
	static audio::OpusFactory			__OpusFactory;
	switch(type){
		case audio::ACT_SPEEX:			return &__speexFactory;
		//case audio::ACT_LAME_MONO:		return &__LameMonoFactory;
		//case audio::ACT_LAME_STEREO:	return &__LameStereoFactory;
		//case audio::ACT_CELT11_STEREO:	return &__celtFactory;
		case audio::ACT_SPEEX_VBR:		return &__speexVBRFactory;
		//case audio::ACT_HEAAC:			return &__AACFactory;
		//case audio::ACT_HEAAC4K:		return &__AAC4KFactory;
		case audio::ACT_OPUS_MONO:
		case audio::ACT_OPUS_STEREO:			return &__OpusFactory;
		default:
			return NULL;
	}
}