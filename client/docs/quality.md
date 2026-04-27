#define GET_SAMPLET_TYPE(quality) 	((quality) >> 24)
#define GET_SAMPLET_RATE(quality) 	((quality) & 0xffffff)
#define MAKE_QUALITY(sampleType,sampleRate)			(((sampleType) << 24) | ((sampleRate) &0xffffff))

#define QC_HIGH 151042944        		//HEAAC4K 
#define QC_NORMAL 167788160      		//OPUS_MONO_16K

void AudioSender::SetQuality(uint32_t quality)
{
	if(_qc == quality)
		return;

	_qc = quality;
	int sampleRate,sampleType;
	if(quality){
		sampleType	= GET_SAMPLET_TYPE(quality);
		sampleRate	= GET_SAMPLET_RATE(quality);
		
		//sampleType	= quality >> 24;;
		//sampleRate	= quality & 0xffffff;
	}else
	{
		sampleType	= DEFAULT_AUDIO_TYPE;//audio::ACT_SPEEX;
		sampleRate	= DEFAULT_AUDIO_RATE;//SAMPLE_RATE_16000;
	}	

	if(sampleRate == 16000 && GetRcEnvConfigInt(_T("RCAudio"), _T("forceOpus"), 0))
	{
		sampleType = audio::ACT_OPUS_MONO;
	}

	int nChannels = 1;
	if(sampleType == audio::ACT_LAME_STEREO || sampleType == audio::ACT_CELT11_STEREO || \
		sampleType == audio::ACT_HEAAC4K || sampleType == audio::ACT_HEAAC || sampleType == audio::ACT_OPUS_STEREO)
		nChannels = 2;

	if( _sampleType != sampleType ||
		_sampleRate != sampleRate ||
		_numChannels != nChannels)
	{
		_sampleType = sampleType;
		_sampleRate = sampleRate;
		_numChannels = nChannels;

		if(_piSoundChannger)
		{
			_piSoundChannger->destroy();
			_piSoundChannger = NULL;
		}

		Stop();
		_listVoice.clear();
		_broadcastBuf.clear();
		_sampleBuf.clear();
		_listBuf.clear();

		UpdateAudioMode(true);
	}

}