#include "audioresampler.h"

namespace audio
{
	CAudioResampler::CAudioResampler(void)
	{
		m_resampler = NULL;
		m_nChannels = 0;
		m_nInRate	= 0;
		m_nOutRate	= 0;
	}

	CAudioResampler::~CAudioResampler(void)
	{
		Clear();
	}

	bool CAudioResampler::SetRate(int chs, int in_rate, int out_rate)
	{
		if(!m_resampler || chs!=m_nChannels || in_rate != m_nInRate || m_nOutRate != out_rate )
		{
			Clear();
			int err = 0;
			m_nChannels	= chs;
			m_nInRate	= in_rate;
			m_nOutRate	= out_rate;


			m_resampler = speex_resampler_init(chs, in_rate, out_rate, 3, &err); 
			assert(m_resampler);
			return (NULL == m_resampler);
		}

		return true;
	}

	bool CAudioResampler::Resample(const char* p, size_t len,  std::string& output)
	{
#define OLEN (16000*4)
		spx_uint32_t ilen			= len / (sizeof(spx_int16_t) * m_nChannels);
		spx_uint32_t olen			= OLEN/ m_nChannels;
		spx_int16_t ob[OLEN];
		ZeroMemory(ob, sizeof(spx_int16_t) * olen);
	//	int err = speex_resampler_process_int(m_resampler, 0, (spx_int16_t*)(p), &ilen, ob, &olen); 
		int err = speex_resampler_process_interleaved_int(m_resampler, (short*)p, &ilen, ob, & olen);
		if (olen == 0) {
			assert(olen);
		}
		output.assign((char*)ob, sizeof(spx_int16_t) * olen * m_nChannels);
		return 0 != olen;
	}
	void CAudioResampler::Clear(void)
	{
		if(NULL != m_resampler)
		{
			speex_resampler_destroy(m_resampler);
		}
		m_resampler = NULL;
	}
}