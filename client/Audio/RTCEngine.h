#pragma once

typedef wchar_t WCHAR;

#include <memory>
#include "client\audio\IAudioEngine.h"
#include "RTCCore.h"
#include "talk/base/scoped_ptr.h"
#include "talk/base/scoped_ref_ptr.h"



//#define DEVICE_BUF_LEN 128

class RTCEngine : public IAudioEngine
{
public:


	RTCEngine();
	~RTCEngine();

	virtual void SetCallbackEvent(struct IXAudioEngineEvent* piEvent);

	virtual int StartAudio(int nSampleRate, int nChannels);
	virtual void StopAudio();


	virtual void SetAEC(bool isEnabled);
	virtual void SetANS(bool isEnabled);
	virtual void SetAGC(bool isEnabled);

	virtual double GetPlayEnergy();
	virtual double GetRecordEnergy();

	virtual double GetVolumeOut();
	virtual double GetVolumeIn();

	virtual bool SetVolueIn(double dfVolume);
	virtual bool SetVolueOut(double dfVolume);

	virtual double GetVadIn();
	virtual double GetVadOut();

	virtual int GetNumOfRecorder();
	virtual int GetNumOfPlayer();

	virtual int GetRecorderInfo(int nIndex, WCHAR arrNameBuf[DEVICE_BUF_LEN]);
	virtual int GetPlayerInfo(int nIndex, WCHAR arrNameBuf[DEVICE_BUF_LEN]);

	virtual int SetRecorderID(int nIndex);
	virtual int SetPlayerID(int nIndex);

	virtual int GetRecorderID();
	virtual int GetPlayerID();

	virtual void StartRecordForCPU();
	virtual void StopRecordForCPU();

	void callback(const int16_t* data, size_t size, int channel, int64_t timestamp);

private: 
	talk_base::scoped_refptr<webrtc::AudioDeviceModule> audio_device_;
	std::unique_ptr<AudioCapture> audio_capture_;
	int player_id_;
	int record_id_;
	struct IXAudioEngineEvent* piEvent_;


};

