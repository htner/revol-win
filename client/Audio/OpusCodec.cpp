#include "audio/IOpusCodec.h"
#include "third/opus/opus.h"
#include <assert.h>
#include <string>

class COpusDecoderImp:public IOpusDec
{
public:
	COpusDecoderImp():
	  _dec(NULL),
		  _samplerate(0),
		  _isFirst(true){}

	  ~COpusDecoderImp(){
		  if(_dec)
			  opus_decoder_destroy(_dec);
	  }

	  void Init(int channel, int samplerate)
	  {
		  if (!_dec || samplerate != _samplerate || _channel != channel)
		  {
			  if(_dec)
				  opus_decoder_destroy(_dec);

			  _samplerate = samplerate;
			  _channel = channel;
			  _dec = opus_decoder_create(samplerate, _channel, NULL);
			  _isFirst = true;
		  }
	  }

	  int Decode(const unsigned char* data, int len, short* pcm, int size)
	  {
		  if(size < _samplerate/50 || !_dec)
		  {
			  assert(0);
			  return -1;
		  }

		  if(_isFirst)
		  {
			  _beforePack.assign((char*)data,len);
			  memset(pcm, 0, size*sizeof(short));
			  _isFirst = false;
			  return _samplerate/50;
		  }

		  int pcmSize = 0;
		  if( _beforePack.empty() ) {
			  /* attempt to decode with in-band FEC from next packet */
			  pcmSize = opus_decode(_dec, data, len, pcm, size, 1);
		  } else {
			  /* regular decode */
			  pcmSize = opus_decode(_dec, (unsigned char*)_beforePack.data(), _beforePack.size(), pcm, size, 0);
		  }

		  if(data)
			  _beforePack.assign((char*)data,len);
		  else
			  _beforePack.clear();

		  return pcmSize;
	  }

	  void Destory(){
		  delete this;
	  }

protected:
	int					_samplerate;
	int				    _channel;
	OpusDecoder			*_dec;
	std::string			_beforePack;
	int					_isFirst;
};

class COpusEncoderImp:public IOpusEnc
{
public:
	COpusEncoderImp():
	  _enc(NULL),
		  _samplerate(0),
		  _bitrate(0){}

	  ~COpusEncoderImp(){
		  if(_enc)
			  opus_encoder_destroy(_enc);
	  }

	  int Encode(const short* pcm, int framesize,unsigned char* data, int payload)
	  {
		  if(framesize != _samplerate / 50 || !_enc)
		  {
			  assert(0);
			  return -1;
		  }

		  return opus_encode(_enc, pcm, framesize, data, payload);
	  }

	  void Init(int channel, int samplerate, float bitrate_bps)
	  {
		  if (!_enc || samplerate != _samplerate || bitrate_bps != _bitrate || _channel != channel)
		  {
			  if(_enc)
				  opus_encoder_destroy(_enc);

			  int error = 0;
			  if (channel == 1) {
				  _enc = opus_encoder_create(samplerate, 1, OPUS_APPLICATION_VOIP, &error);
			  } else {
				  _enc = opus_encoder_create(samplerate, 2, OPUS_APPLICATION_AUDIO, &error);
			  }

			  _samplerate = samplerate;
			  _channel = channel;
			  _bitrate = bitrate_bps;

			  opus_encoder_ctl(_enc, OPUS_SET_BITRATE(bitrate_bps));
			  opus_encoder_ctl(_enc, OPUS_SET_BANDWIDTH(OPUS_AUTO));
			  opus_encoder_ctl(_enc, OPUS_SET_VBR(1));
			  opus_encoder_ctl(_enc, OPUS_SET_VBR_CONSTRAINT(0));
			  opus_encoder_ctl(_enc, OPUS_SET_COMPLEXITY(10));
			  opus_encoder_ctl(_enc, OPUS_SET_INBAND_FEC(1));
			  //opus_encoder_ctl(enc, OPUS_SET_FORCE_MONO(forcemono));
			  opus_encoder_ctl(_enc, OPUS_SET_DTX(0));
			  opus_encoder_ctl(_enc, OPUS_SET_PACKET_LOSS_PERC(10));
		  }
	  }

	  virtual void Destory(){
		  delete this;
	  }

protected:
	OpusEncoder			*_enc;
	unsigned int		_samplerate;
	int				    _channel;
	float				_bitrate;
};


IOpusEnc* CreateOpusEnc(){
	return new COpusEncoderImp;
}

IOpusDec* CreateOpusDec(){
	return new COpusDecoderImp;
}