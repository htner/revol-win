#pragma once

#include "audio/iaudioresampler.h"
#include "audioresampler.h"
#include <vector>

namespace audio
{
	class CAudioResamplerFactor
	{
	public:
		CAudioResamplerFactor(void);
		~CAudioResamplerFactor(void);
	protected:
		struct resampler
		{
		public:
			resampler(int id, audio::IAudioResampler* resampler)
			{
				m_id			= id;
				m_resampler		= resampler;
			}

			resampler(void)
			{
				m_id			= -1;
				m_resampler		= NULL;
			}
		public:
			int							m_id;
			audio::IAudioResampler*		m_resampler;
		};
	public:
		audio::IAudioResampler*			Create(int id);

		void Destroy(int id);
	protected:
		resampler						m_resampler[MAX_RESAMPLER];
	};
}