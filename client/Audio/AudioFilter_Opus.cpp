#include "AudioFilter_Opus.h"
#include <atlsync.h>
namespace audio{

#ifdef _DEBUG
#pragma comment(lib, "opuslib_d.lib")
#else
#pragma comment(lib, "opuslib.lib")
#endif 

static CCriticalSection s_opusLock;

OpusDecoder::OpusDecoder()
{
	_piDec = CreateOpusDec();
	_firstDec = true;
}

OpusDecoder::~OpusDecoder()
{
	if(_piDec)
		_piDec->Destory();
}

BOOL OpusDecoder::Decode(const binString& in, binString& out) 
{
	if(!_piDec)
		return FALSE;

	const int BUF_LEN = 1024;
	short arrBuf[BUF_LEN];
	s_opusLock.Enter();
	int decSize = _piDec->Decode((const unsigned char*)in.data(), in.size(), arrBuf, BUF_LEN);
	s_opusLock.Leave();
	if(decSize <= 0)
		return FALSE;

	_firstDec = false;
	out.assign((char*)arrBuf, decSize*2);
	return TRUE;
}

BOOL OpusDecoder::DecodeLoss(binString& out)
{
	if(!_piDec)
		return FALSE;

	const int BUF_LEN = 1024;
	short arrBuf[BUF_LEN] = {0};
	if(_firstDec)
	{
		out.assign((char*)arrBuf, 320*2);
		return true;
	}
	
	s_opusLock.Enter();
	int decSize = _piDec->Decode(NULL, 0, arrBuf, BUF_LEN);
	s_opusLock.Leave();

	if(decSize <= 0)
		return FALSE;

	out.assign((char*)arrBuf, decSize*2);
	return TRUE;
}

void OpusDecoder::SetRate(int framesize, int channel, int samplerate)
{
	if(!_piDec)
		return;

	_piDec->Init(channel, samplerate);
}

OpusEncoder::OpusEncoder()
{
	_piEnc = CreateOpusEnc();
}

OpusEncoder::~OpusEncoder()
{
	if(_piEnc)
		_piEnc->Destory();
}

void OpusEncoder::SetQuality(int quality)
{
}

BOOL OpusEncoder::Encode(const binString& in, binString& out)
{
	if(in.size() != 320 * 2 || !_piEnc)
	{
		assert(0);
		return FALSE;
	}

	short* pwData = (short*)in.data();
	int size = in.size()/2;
	unsigned char arrBuf[1024] = {0};
	s_opusLock.Enter();
	int encLen = _piEnc->Encode(pwData, size, arrBuf, 1024);
	s_opusLock.Leave();
	out.assign((char*)arrBuf, encLen);
	return TRUE;
}

void OpusEncoder::SetRate(int framesize, int channel, int samplerate)
{
	if (framesize != 320 || samplerate != 16000 || !_piEnc)
	{
		assert(0);
		return;
	}

	_piEnc->Init(channel, samplerate, 30000);
}
///////////////////////////////////////////////////////////////////////////////////////////
OpusFactory::~OpusFactory()
{
	std::map<unsigned int ,IAudioEncoder*>::iterator eitr = __encoders.begin();
	for(;eitr != __encoders.end(); ++ eitr){
		eitr->second->Destory();
	}

	std::map<unsigned int ,IAudioDecoder*>::iterator ditr = __decoders.begin();
	for(;ditr != __decoders.end(); ++ ditr){
		ditr->second->Destory();
	}

}

void OpusFactory::ReleaseDecoder(bool isAll,unsigned int uid)
{
	if(isAll)
	{
		std::map<unsigned int ,IAudioDecoder*>::iterator ditr = __decoders.begin();
		for(;ditr != __decoders.end(); ++ ditr){
			ditr->second->Destory();
		}
		__decoders.clear();
	}else{
		std::map<unsigned int ,IAudioDecoder*>::iterator it = __decoders.find(uid);

		if(it != __decoders.end())
		{
			it->second->Destory();
			__decoders.erase(it);
		}
	}
}

IAudioEncoder*	OpusFactory::GetEncoder(unsigned int uid)
{
	std::map<unsigned int ,IAudioEncoder*>::iterator itr = __encoders.find(uid);
	if(itr != __encoders.end()){
		return itr->second;
	}
	IAudioEncoder* encoder = new OpusEncoder();
	__encoders[uid] = encoder;
	return encoder;
}

IAudioDecoder*	OpusFactory::GetDecoder(unsigned int uid)
{
	std::map<unsigned int ,IAudioDecoder*>::iterator itr = __decoders.find(uid);
	if(itr != __decoders.end()){
		return itr->second;
	}
	IAudioDecoder* decoder = new OpusDecoder();
	__decoders[uid] = decoder;
	return decoder;
}

IAudioProcessor* OpusFactory::GetProcessor(unsigned int uid)
{
	return NULL;
}


///////////////////////////////////////////////////////////////////////////////////////////


};