#ifndef __I_AUDIO_ENGINE_H__
#define __I_AUDIO_ENGINE_H__

const int DEVICE_BUF_LEN = 128;
struct IXAudioEngineEvent{
	virtual void SampleAudioData(const short* pCleanAudioData,	
		int nNumSample,
		int nSamplingFreq,
		int nChannels,
		int nVad
		) = 0;

	virtual  bool PlayAudioData(short* pPlayData, 
		int nNumSample,
		int nSamplingFreq,
		int nChannels
		) = 0;

	virtual void OnAudioChanged(int eDev,double dfValue) = 0;
};

struct IAudioEngine{
public:	
	virtual void SetCallbackEvent(struct IXAudioEngineEvent* piEvent) = 0;

	virtual int StartAudio(int nSampleRate, int nChannels) = 0;
	virtual void StopAudio() = 0;


	virtual void SetAEC( bool isEnabled) = 0; 
	virtual void SetANS( bool isEnabled) = 0;
	virtual void SetAGC( bool isEnabled) = 0;

	virtual double GetPlayEnergy() = 0;
	virtual double GetRecordEnergy() = 0;

	virtual double GetVolumeOut() = 0;
	virtual double GetVolumeIn() = 0;

	virtual bool SetVolueIn(double dfVolume) = 0;
	virtual bool SetVolueOut(double dfVolume) = 0;

	virtual double GetVadIn() = 0;
	virtual double GetVadOut() = 0;

	virtual int GetNumOfRecorder() = 0;
	virtual int GetNumOfPlayer()   = 0;

	virtual int GetRecorderInfo(int nIndex, WCHAR arrNameBuf[DEVICE_BUF_LEN]) = 0;
	virtual int GetPlayerInfo(int nIndex, WCHAR arrNameBuf[DEVICE_BUF_LEN]) = 0;

	virtual int SetRecorderID(int nIndex) = 0;
	virtual int SetPlayerID(int nIndex) = 0;

	virtual int GetRecorderID() = 0;
	virtual int GetPlayerID() = 0;

	virtual void StartRecordForCPU() = 0;
	virtual void StopRecordForCPU() =0;
};

#ifdef X_AUDIO_ENGINE_EXPORTS
#define X_AUDIO_ENGINE_API  __declspec(dllexport)
#else
#define X_AUDIO_ENGINE_API  __declspec(dllimport)
#endif

//extern "C" X_AUDIO_ENGINE_API struct IAudioEngine* GetAudioEngine();
extern "C" struct IAudioEngine* GetAudioEngine();
//extern  struct IAudioEngine* GetEngine();
#endif //__I_AUDIO_ENGINE_H__
