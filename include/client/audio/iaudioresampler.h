#pragma once

#include <string>
#include "macros.h"
#include "audio/samples.h"

#define MAX_RESAMPLER 30

const int NET_MUSIC_CLINE_ID = 199029;
const int HARDWARE_MUSIC_ID  = 874574;

namespace audio
{
	struct IAudioResampler
	{
		/*
		* 函数名称：IAudioResampler ::SetRate
		* 函数描述：设置resample的参数
		* 输入参数：chs, 声道数量;in_rate,输入的采样率; out_rate，输出的采样率
		* 输出参数：void
		* 返回值  ：成功返回true;否则返回false
		*/
		virtual bool SetRate(int chs, int in_rate, int out_rate) = 0;

		/*
		* 函数名称：IAudioResampler ::Resample
		* 函数描述：改变采样
		* 输入参数：input,源采样数据
		* 输出参数：output,resampler后的数据
		* 返回值  ：成功返回true;否则返回false
		*/
		virtual bool Resample(const char* p, size_t len, std::string& output) = 0;

		/*
		* 函数名称：IAudioResampler ::Release
		* 函数描述：删除自已
		* 输入参数：void
		* 输出参数：void
		* 返回值  ：void
		*/
		virtual void Release(void) = 0;
	};
}
audio::IAudioResampler* AudioCreateResampler(int id);
void ReleaseResampler(int id);
void AudioResamplerDestroy(void);