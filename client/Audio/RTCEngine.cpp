#include "RTCEngine.h"
#include "client/helper/utf8to.h"
#include "common/xstring.h"
#include <thread>
#include "system_wrappers\interface\trace.h"

class AERR : public webrtc::AudioDeviceObserver
{
public:
	void OnErrorIsReported(const ErrorCode error) override {
		int k = 0;
	}
	 void OnWarningIsReported(const WarningCode warning)  override {
		int k = 2;
	}

};

AERR kAERR;

RTCEngine::RTCEngine()
{
	audio_capture_.reset(new AudioCapture());
	auto tid = std::this_thread::get_id();
	int32_t id = *(int32_t*)&tid;
	//webrtc::Trace::CreateTrace();
	//webrtc::Trace::SetTraceFile("webrtc_trace", true);
	//webrtc::Trace::set_level_filter(webrtc::kTraceAll);
	
	audio_device_ = webrtc::CreateAudioDeviceModule(id, webrtc::AudioDeviceModule::kWindowsWaveAudio);
	if (!audio_device_ || audio_device_->Init() != 0) {
		player_id_ = -2;
		record_id_ = -2;
	}
	//int32_t a1 = audio_device_->ActiveAudioLayer(webrtc::AudioDeviceModule::kWindowsCoreAudio);
	//int32_t a2 = audio_device_->ActiveAudioLayer(webrtc::AudioDeviceModule::kWindowsWaveAudio);
	//int32_t a1 = audio_device_->ActiveAudioLayer(webrtc::AudioDeviceModule::kWindowsCoreAudio);
	audio_device_->RegisterEventObserver(&kAERR);
	bool ret = audio_device_->Initialized();
	player_id_ = -1;
	record_id_ = -1;
}


RTCEngine::~RTCEngine()
{
	StopAudio();
	audio_device_->Terminate();
	audio_device_ = nullptr;
}

void RTCEngine::SetCallbackEvent(struct IXAudioEngineEvent* piEvent){
	audio_capture_->SetCallbackEvent(piEvent);
	//piEvent_ = piEvent;
}

/*
void RTCEngine::callback(const int16_t* data, size_t size, int channel, int64_t timestamp) {
	piEvent_->SampleAudioData((short*)data, size, 0, channel, 0);
}
*/

int RTCEngine::StartAudio(int nSampleRate, int nChannels){
	return audio_capture_->Start(audio_device_.get(), nSampleRate, nChannels);
}

void RTCEngine::StopAudio(){
	audio_capture_->Stop(audio_device_.get());
}

void RTCEngine::SetAEC(bool isEnabled){
	audio_device_->EnableBuiltInAEC(isEnabled);
}
void RTCEngine::SetANS(bool isEnabled){
}
void RTCEngine::SetAGC(bool isEnabled){
	audio_device_->SetAGC(isEnabled);
}

double RTCEngine::GetPlayEnergy(){ 
	
	return 0.0;
}
double RTCEngine::GetRecordEnergy(){ return 0.0; }

double RTCEngine::GetVolumeOut(){ 
	uint16_t left = 0;
	uint16_t right = 0;
	audio_device_->WaveOutVolume(&left, &right);
	return (left + right) / 2;
}
double RTCEngine::GetVolumeIn(){ 
	uint32_t volume;
	audio_device_->SpeakerVolume(&volume);
	return volume;
}

bool RTCEngine::SetVolueIn(double dfVolume){ 
	audio_device_->SetSpeakerVolume(dfVolume);
	return false;
}
bool RTCEngine::SetVolueOut(double dfVolume){ 
	return audio_device_->SetWaveOutVolume(dfVolume, dfVolume) != 0;
}

double RTCEngine::GetVadIn(){ return 0.0; }
double RTCEngine::GetVadOut(){ return 0.0; }

int RTCEngine:: GetNumOfRecorder(){
	int16_t ret = audio_device_->RecordingDevices();
	int error = audio_device_->LastError();
	return ret;
}

int RTCEngine:: GetNumOfPlayer(){
	int16_t ret = audio_device_->PlayoutDevices();
	int error = audio_device_->LastError();
	return ret;
}

int RTCEngine:: GetRecorderInfo(int nIndex, WCHAR arrNameBuf[DEVICE_BUF_LEN]){
	char name[webrtc::kAdmMaxDeviceNameSize];
	char guid[webrtc::kAdmMaxGuidSize];
	int ret = audio_device_->RecordingDeviceName(nIndex, name, guid);
	xstring strParam = helper::net2app(name);
	int len = strParam.size() > DEVICE_BUF_LEN ? DEVICE_BUF_LEN : strParam.size();
	wcsncpy(arrNameBuf, strParam.c_str(), len);
	return ret;
}

int RTCEngine:: GetPlayerInfo(int nIndex, WCHAR arrNameBuf[DEVICE_BUF_LEN]){
	char name[webrtc::kAdmMaxDeviceNameSize];
	char guid[webrtc::kAdmMaxGuidSize];
	int ret = audio_device_->PlayoutDeviceName(nIndex, name, guid);
	xstring strParam = helper::net2app(name);
	int len = strParam.size() > DEVICE_BUF_LEN ? DEVICE_BUF_LEN : strParam.size();
	wcsncpy(arrNameBuf, strParam.c_str(), len);
	return ret;
}

int RTCEngine:: SetRecorderID(int nIndex){
	record_id_ = nIndex;
	return audio_device_->SetRecordingDevice(nIndex);
}

int RTCEngine:: SetPlayerID(int nIndex){
	player_id_ = nIndex;
	return audio_device_->SetPlayoutDevice(nIndex);
}


int RTCEngine:: GetRecorderID(){
	return record_id_;
}

int RTCEngine:: GetPlayerID(){
	return player_id_;
}

void RTCEngine::StartRecordForCPU(){}
void RTCEngine::StopRecordForCPU(){}