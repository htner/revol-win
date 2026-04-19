		//audio quality setting
		ISessionInfoBase* lpSessInfoBase = m_pSettingFrame->GetSessInfoBase();
		BOOL bChangeAudio = lpSessInfoBase->isChangeQuality();
		if (bChangeAudio)
		{
			protocol::session::VoiceQC qc = DEFAULT_QUALITY;
			if(UI_RADIOBOX(SpeedFirst)->IsSelected())
			{
				qc = MAKE_QUALITY(audio::ACT_OPUS_MONO,SAMPLE_RATE_16000);
			}else if(UI_RADIOBOX(QualityFirst)->IsSelected())
			{
				qc = MAKE_QUALITY(DEFAULT_MUSIC_TYPE,DEFAULT_MUSIC_RATE);
			}
			pSessInfo->setVoiceQC(qc);
		}


#define DEFAULT_AUDIO_RATE			SAMPLE_RATE_16000
#define DEFAULT_AUDIO_TYPE			audio::ACT_OPUS_MONO

#define DEFAULT_MUSIC_RATE			SAMPLE_RATE_48000
#define DEFAULT_MUSIC_TYPE			audio::ACT_OPUS_STEREO	
