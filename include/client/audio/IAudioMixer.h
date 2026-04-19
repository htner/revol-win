#ifndef __I_AUDIO_MIXER_H__
#define __I_AUDIO_MIXER_H__

/**************************************************************************
混音器
注意：一個混音流程使用一個混音器實例，禁止多個混音流程使用一個混音器
***************************************************************************/
class IAudioMixer
{
public:
/*********************************************
說明：混音處理
audio1: 語音信號輸入1， 並用於輸出
audio2: 語音信號輸入2
len: audio1與audio2的長度，必須保持一致
*********************************************/
	virtual void mix(short* audio1, const short* audio2, int len) = 0;

/*********************************************
說明：重置混音器
*********************************************/
	virtual void reset() = 0;

/*********************************************
說明：銷毀實例
*********************************************/
	virtual void destroy() = 0;
};

#ifdef X_AUDIO_ENGINE_EXPORTS
#define X_AUDIO_ENGINE_API __declspec(dllexport)
#else
#define X_AUDIO_ENGINE_API __declspec(dllimport)
#endif

//extern "C" X_AUDIO_ENGINE_API IAudioMixer* createAudioMixer();
extern "C" IAudioMixer* createAudioMixer();

#endif //__I_AUDIO_MIXER_H__
