#pragma once
#include "location/project_def.h"

#define SEQ_SKIP_PAGE(seqno)	(seqno += SEQPAGE)

#define SEQ_ADD(seqno)			(seqno += PACKET_SEQ_INCREMENT)

#define SEQORIGINAL				1000

#define SEQINTERVAL				 (SEQPAGE / PACKET_SEQ_INCREMENT)

#define SEQ_SKIP_COUNT(cur, old) ((cur - old + 1) / PACKET_SEQ_INCREMENT)

#define SEQ_PROTECT_SKIP(count, old, seq) if(abs(count) >= SEQINTERVAL || (old == 0) || seq == SEQORIGINAL) count = 1;

#define ALIGN_SEQ(seqno)			seqno = seqno / 2 * 2;



#define SPEEX_TIME_PER_FRAME		20
inline int SPEEX_FRAME_SAMPLES(int type, int rate)
{
	if(rate == 48000) 
		return 1024;
	if (type == 10 || type == 11) {
		return (rate / 1000 * SPEEX_TIME_PER_FRAME);
	}

	return (rate/1000 * SPEEX_TIME_PER_FRAME);
}

#define SPEEX_FRAME_SIZE(type, rate)		(SPEEX_FRAME_SAMPLES(type, rate) * 2)
#define SPEEX_RATE(x)				(x)

#if defined(REX_TW) || defined(REX_MAL)
#define FRAMES_PER_PACKET			2
#define DEFAULT_PACK_TIME			80.0
#else
#define FRAMES_PER_PACKET			1
#define DEFAULT_PACK_TIME			20.0
#endif

#define SPEEX_PACKET_SIZE(type, rate)		(SPEEX_FRAME_SIZE(type, rate)*FRAMES_PER_PACKET)


#define SAMPLE_RATE_8000			8000
#define SAMPLE_RATE_16000			16000
#define SAMPLE_RATE_32000			32000
#define SAMPLE_RATE_44100			44100
#define SAMPLE_RATE_48000			48000

/*#define DEFAULT_SAMPLE_RATE			SAMPLE_RATE_32000
#define DEFAULT_FRAME_SIZE			SPEEX_FRAME_SIZE(DEFAULT_SAMPLE_RATE)
#define DEFAULT_PACKET_SIZE			SPEEX_PACKET_SIZE(DEFAULT_SAMPLE_RATE)
#define DEFAULT_SPEEX_MODE			SPEEX_MODEID_UWB
#define DEFAULT_FRAME_SAMPLES		SPEEX_FRAME_SAMPLES(DEFAULT_SAMPLE_RATE)*/
//#define DEFAULT_PLAY_CHANNELS		1		
#define DEFAULT_AUDIO_RATE			SAMPLE_RATE_16000
#define DEFAULT_AUDIO_TYPE			audio::ACT_OPUS_MONO

#define DEFAULT_MUSIC_RATE			SAMPLE_RATE_48000
#define DEFAULT_MUSIC_TYPE			audio::ACT_OPUS_STEREO	


#define GET_SAMPLET_TYPE(quality) 	((quality) >> 24)
#define GET_SAMPLET_RATE(quality) 	((quality) & 0xffffff)
#define MAKE_QUALITY(sampleType,sampleRate)			(((sampleType) << 24) | ((sampleRate) &0xffffff))
