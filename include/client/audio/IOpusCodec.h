#ifndef __I_OPUS_CODEC_H__
#define __I_OPUS_CODEC_H__

class IOpusEnc{
public:
	virtual void Init(int channel, int samplerate, float bitrate_bps) = 0;

	virtual int Encode(const short* pcm, int framesize,unsigned char* data, int payload) = 0;

	virtual void Destory() = 0;
};


class IOpusDec{
public:
	virtual void Init(int channel, int samplerate) = 0;

	virtual int Decode(const unsigned char* data, int len, short* pcm, int size) = 0;

	virtual void Destory() = 0;
};

IOpusDec * CreateOpusDec();
IOpusEnc * CreateOpusEnc();

#endif //__I_OPUS_CODEC_H__
