#ifndef __INTERFACE_SEC_GAIN_H__
#define __INTERFACE_SEC_GAIN_H__

class ISecGain{
public:
	virtual ~ISecGain(){};

	virtual void SetSecGain(float gain) = 0;
	virtual void Process(short* data, int len) = 0;
};

#ifdef X_AUDIO_ENGINE_EXPORTS
#define X_AUDIO_ENGINE_API __declspec(dllexport)
#else
#define X_AUDIO_ENGINE_API __declspec(dllimport)
#endif

//extern "C" X_AUDIO_ENGINE_API ISecGain*CreateSecGain();
//extern "C" X_AUDIO_ENGINE_API void DestroySecGain(ISecGain *);

extern "C" ISecGain*CreateSecGain();
extern "C" void DestroySecGain(ISecGain *);

#endif //__INTERFACE_SEC_GAIN_H__
