#include "AudioResamplerFactor.h"

namespace audio
{
	CAudioResamplerFactor::CAudioResamplerFactor(void)
	{
	}

	CAudioResamplerFactor::~CAudioResamplerFactor(void)
	{
		for(int i=0; i<MAX_RESAMPLER; ++i)
		{
			if(m_resampler[i].m_resampler != NULL)
				SAFE_RELEASE(m_resampler[i].m_resampler);
		}
	}

	audio::IAudioResampler*	CAudioResamplerFactor::Create(int id)
	{
		for(int i=0; i<MAX_RESAMPLER; ++i)
		{
			if(m_resampler[i].m_id == id)
			{
				ASSERT(m_resampler[i].m_resampler);
				return m_resampler[i].m_resampler;
			}
		}

		for(int i=0; i<MAX_RESAMPLER; ++i)
		{
			if(m_resampler[i].m_id == -1)
			{
				m_resampler[i].m_id = id;
				m_resampler[i].m_resampler = new CAudioResampler();
				ASSERT(m_resampler[i].m_resampler);
				return m_resampler[i].m_resampler;
			}
		}
		assert(false);
		return NULL;
	}

	void CAudioResamplerFactor::Destroy(int id)
	{
		for(int i=0; i<MAX_RESAMPLER; ++i)
		{
			if(m_resampler[i].m_id == id)
			{
				m_resampler[i].m_id = -1;
				SAFE_RELEASE(m_resampler[i].m_resampler);
				return ;
			}
		}
	}
}
