#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <system_error>
#include <algorithm>
#include "audio/iaudioresampler.h"
#include "webrtc/modules/audio_device/include/audio_device.h"
#include "RTCCore.h"
#include "client/audio/iaudiocodec.h"
//#include "webrtc/modules/audio_coding/codecs/opus/interface/opus_interface.h"
//#include "webrtc/modules/audio_codecs/opus/audio_encoder_opus.h"


void AudioCapture::SetCallbackEvent(struct IXAudioEngineEvent* piEvent){
	piEvent_ = piEvent;
}

bool AudioCapture::Start(webrtc::AudioDeviceModule* audio_device_, int nSampleRate, int nChannels) {
	// 注册音频传输回调
	int reg = audio_device_->RegisterAudioCallback(this);
	net_sample_rate_ = nSampleRate;
	net_channel_ = nChannels;
	int start_ret;

	{ // playout setting
		audio_device_->SetPlayoutSampleRate(nSampleRate);
		auto a = audio_device_->SetPlayoutDevice(0);
		int set;
		if (nChannels == 2) {
			set = audio_device_->SetStereoPlayout(true);
		}
		else {
			set = audio_device_->SetStereoPlayout(false);
		}

		auto b = audio_device_->InitPlayout();
		bool is_stereo = false;
		playout_channel_ = 2;
		if (audio_device_->StereoPlayout(&is_stereo) == 0) {
			if (is_stereo) {
				playout_channel_ = 2;
			}
			else {
				playout_channel_ = 1;
			}
		}
		audio_device_->PlayoutSampleRate(&playout_sample_rate_);
		

		if (playout_sample_rate_ != net_sample_rate_) {
			playout_resampler_ = AudioCreateResampler(2);
			playout_resampler_->SetRate(playout_channel_, net_sample_rate_, playout_sample_rate_);
		}
		else {
			recording_resampler_ = nullptr;
		}
		auto c = audio_device_->StartPlayout();
	}
	// 初始化音频设备模块

	{ // recording setting
		uint32_t sample = 0;
		//auto a = audio_device_->SetRecordingDevice(0);

		int32_t  set = 0;
		if (nChannels == 2) {
			set = audio_device_->SetStereoRecording(true);
		}
		else {
			set = audio_device_->SetStereoRecording(false);
		}
		int set_ret = audio_device_->SetRecordingSampleRate(nSampleRate);
		auto b = audio_device_->InitRecording();
		int ret_error = audio_device_->LastError();
		bool available = false;
		//auto c = audio_device_->RecordingIsAvailable(&available);
		int get_ret = audio_device_->RecordingSampleRate(&recording_sample_rate_);

		bool is_stereo = true;
		recording_channel_ = 2;
		if (audio_device_->StereoRecording(&is_stereo) == 0) {
			if (is_stereo) {
				recording_channel_ = 2;
			}
			else {
				recording_channel_ = 1;
			}
		}
		webrtc::AudioDeviceModule::ChannelType channelt;
		int32_t ct_ret = audio_device_->RecordingChannel(&channelt);


		bool recording = audio_device_->Recording();
		start_ret = audio_device_->StartRecording();
		

		if (recording_sample_rate_ != net_sample_rate_) {
			recording_resampler_ = AudioCreateResampler(1);
			recording_resampler_->SetRate(recording_channel_, recording_sample_rate_, net_sample_rate_);
		}
		else {
			recording_resampler_ = nullptr;
		}
		ret_error = audio_device_->LastError();
	}

	return start_ret == 0;
}

void AudioCapture::Stop(webrtc::AudioDeviceModule* audio_device_) {
	if (audio_device_) {
		audio_device_->StopRecording();
		audio_device_->StopPlayout();
		//audio_device_->Terminate();
		//audio_device_ = nullptr;
	}
	//WebRtcOpus_EncoderFree(inst_);
	//inst_ = nullptr;
	buffer_.clear();
	ReleaseResampler(1);
	ReleaseResampler(2);
}

void AudioCapture::OnDataRecorded(const void* audioSamples, size_t nSamples,
	size_t nBytesPerSample, size_t nChannels,
	uint32_t sampleRate) {
	
	if (nBytesPerSample != 2) return;
	if (sampleRate != recording_sample_rate_) return;
	if (nChannels != recording_channel_) return;
	
	if (recording_resampler_) {
		const int16_t* data = static_cast<const int16_t*>(audioSamples);
		std::vector<int16_t> datas;
		datas.assign(data, data + nSamples * nChannels);
		std::string resample_buffer;
		if (!recording_resampler_->Resample((const char*)audioSamples, nSamples * nBytesPerSample * nChannels, resample_buffer)) {
			return;
		}
		recording_resampler_buffer_ += resample_buffer;
	}
	else {
		recording_resampler_buffer_.append((const char*)audioSamples, nSamples * nBytesPerSample * nChannels);
	}
	
	int frameSize = SPEEX_FRAME_SIZE(audio::ACT_OPUS_MONO, net_sample_rate_);
		
	if (frameSize * nChannels > recording_resampler_buffer_.size()) {
		return;
	}
	int send_size = recording_resampler_buffer_.size() / (frameSize * nChannels) * frameSize * nChannels;
	if (piEvent_){
		//const int16_t* data = static_cast<const int16_t*>(audioSamples);

		piEvent_->SampleAudioData((short*)recording_resampler_buffer_.data(), send_size / nBytesPerSample / nChannels, 0, nChannels, 0);
		recording_resampler_buffer_.erase(0, send_size);
	}

	/*
	else {
		if (piEvent_){
			const int16_t* data = static_cast<const int16_t*>(audioSamples);
			piEvent_->SampleAudioData((short*)data, nSamples, 0, nChannels, 0);
		}
	}
	*/
}


 int32_t AudioCapture::NeedMorePlayData(const uint32_t nSamples,
	const uint8_t nBytesPerSample,
	const uint8_t nChannels,
	const uint32_t samplesPerSec,
	void* audioSamples,
	uint32_t& nSamplesOut) {
	 
	 if (nBytesPerSample != 2) return -1;
	 if (samplesPerSec != playout_sample_rate_) return -1;
	 if (nChannels != playout_channel_) return -1;

	 if (playout_resampler_ && piEvent_) {
		 int get_max_samples = nSamples * nBytesPerSample * net_sample_rate_ * nChannels / playout_sample_rate_;
		 playout_buffer_.resize(get_max_samples);
		 bool ret = piEvent_->PlayAudioData((short*)playout_buffer_.data(), get_max_samples / nBytesPerSample / playout_channel_, 0, playout_channel_);
		 if (ret) {
			 if (!playout_resampler_->Resample((const char*)playout_buffer_.data(), get_max_samples, playout_resampler_buffer_)) {
				 nSamplesOut = 0;
				 return -1;
			 }
			 memcpy(audioSamples, playout_resampler_buffer_.data(), nSamples * nBytesPerSample * nChannels);
			 nSamplesOut = nSamples;
			 return 0;
		 }
	 }
	 else if (piEvent_) {
		 bool ret = piEvent_->PlayAudioData((short*)audioSamples, nSamples, 0, nChannels);
		 if (ret) {
			 nSamplesOut = nSamples;
		 }
	 }
	return 0;
}