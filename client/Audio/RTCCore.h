#pragma once
typedef wchar_t WCHAR;
#include <functional>
#include <memory>
#include <vector>
#include <system_error>
#include <string>

#include "webrtc/modules/audio_device/include/audio_device.h"
#include "client\audio\IAudioEngine.h"
#include "audio/iaudioresampler.h"
//#include "webrtc/modules/audio_coding/codecs/opus/interface/opus_interface.h"
//#include "webrtc/modules/audio_codecs/opus/audio_encoder_opus.h"

class AudioCapture  : public webrtc::AudioTransport {

public:
	//using EncodedCallback = std::function<void(const int16_t* data, size_t size, int channel,  int64_t timestamp)>;

	AudioCapture() = default;
	~AudioCapture() { /*Stop();*/}

	bool Start(webrtc::AudioDeviceModule* audio_device_, int nSampleRate, int nChannels);
	void Stop(webrtc::AudioDeviceModule* audio_device_);


	void SetCallbackEvent(struct IXAudioEngineEvent* piEvent);

	int32_t RecordedDataIsAvailable(const void* audioSamples,
		const uint32_t nSamples,
		const uint8_t nBytesPerSample,
		const uint8_t nChannels,
		const uint32_t samplesPerSec,
		const uint32_t totalDelayMS,
		const int32_t clockDrift,
		const uint32_t currentMicLevel,
		const bool keyPressed,
		uint32_t& newMicLevel)  override {
		OnDataRecorded(audioSamples, nSamples, nBytesPerSample,
			nChannels, samplesPerSec);
		return 0;
	}

		// 其他接口保持默认实现
	virtual int32_t NeedMorePlayData(const uint32_t nSamples,
		const uint8_t nBytesPerSample,
		const uint8_t nChannels,
		const uint32_t samplesPerSec,
		void* audioSamples,
		uint32_t& nSamplesOut) override;


	void OnDataRecorded(const void* audioSamples, size_t nSamples,
		size_t nBytesPerSample, size_t nChannels,
		uint32_t sampleRate);

	
	//std::unique_ptr<AudioEncoder> opus_encoder_;
	// OpusEncInst* inst_;
	//EncodedCallback user_callback_;
	std::vector<int16_t> buffer_;

	struct IXAudioEngineEvent* piEvent_;

	uint32_t recording_sample_rate_;
	int recording_channel_;
	audio::IAudioResampler* recording_resampler_;
	std::string recording_resampler_buffer_;

	uint32_t playout_sample_rate_;
	int playout_channel_;
	audio::IAudioResampler* playout_resampler_;
	std::string playout_buffer_;
	std::string playout_resampler_buffer_;

	uint32_t net_sample_rate_;
	int net_channel_;
};