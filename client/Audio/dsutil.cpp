//-----------------------------------------------------------------------------
// File: DSUtil.cpp
//
// Desc: DirectSound framework classes for reading and writing wav files and
//       playing them in DirectSound buffers. Feel free to use this class 
//       as a starting point for adding extra functionality.
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <mmsystem.h>
#include "audio/DSUtil.h"
#include "audio/IAudioEngine.h"
#include "audio/IAudioCodec.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>

#ifndef SAFE_DELETE
	#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
	#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif

extern "C" struct IAudioEngine* GetAudioEngine();
/*****************************************************************************/
CSoundPlayer::CSoundPlayer():
m_pBuf(NULL)
, m_hWaveOut(NULL)
{}

CSoundPlayer::~CSoundPlayer()
{
	if(m_hWaveOut)
	{
		for(int i = 0; i < 100; i++)
		{
			if(!(m_waveHdr.dwFlags&WHDR_DONE))
				Sleep(20);		
			else
				break;
		}

		waveOutReset(m_hWaveOut);

		assert(m_waveHdr.dwFlags & WHDR_DONE);
		waveOutUnprepareHeader(m_hWaveOut,&m_waveHdr,sizeof(WAVEHDR));

		waveOutClose(m_hWaveOut);
		m_hWaveOut = NULL;
	}

	SAFE_DELETE_ARRAY(m_pBuf);
}

BOOL CSoundPlayer::Init(HWND hWnd, GUID devGuid)
{
	SAFE_DELETE_ARRAY(m_pBuf);
	
	return TRUE;
}

BOOL CSoundPlayer::PlayMemory(BYTE* p, int len,  WAVEFORMATEX& wfx)
{
	if(!Stop() || GetAudioEngine() == NULL)
		return false;

	int waveOutID = GetAudioEngine()->GetPlayerID();
	SAFE_DELETE_ARRAY(m_pBuf);
	m_pBuf = new char[len];
	memcpy(m_pBuf, p, len);

	if(MMSYSERR_NOERROR != waveOutOpen(	&m_hWaveOut,waveOutID,&wfx,
			(DWORD_PTR)NULL,(DWORD_PTR)this,CALLBACK_NULL))
	{
 		SAFE_DELETE_ARRAY(m_pBuf);
 		len = 0;
 		assert(0);
		return false;
	}

	memset(&m_waveHdr,0,sizeof(WAVEHDR));
	m_waveHdr.dwUser = 0;
	m_waveHdr.lpData = (char*)m_pBuf;
	m_waveHdr.dwBufferLength = len;

	if(MMSYSERR_NOERROR  !=  waveOutPrepareHeader(m_hWaveOut,&m_waveHdr,sizeof(WAVEHDR))
		|| MMSYSERR_NOERROR  != waveOutWrite(m_hWaveOut,&m_waveHdr,sizeof(WAVEHDR)))
	{
		SAFE_DELETE_ARRAY(m_pBuf);
		assert(0);
		return false;
	}

	return true;
}

// void CSoundPlayer::waveOut(HWAVEIN hwi, UINT uMsg, DWORD dwUser,DWORD dw1, DWORD dw2)
// {
// 	if(uMsg != MM_WOM_DONE )
// 		return;
// 
// 	CSoundPlayer* pPlayer = reinterpret_cast<CSoundPlayer*>(dwUser);
// 
// 	if(pPlayer)
// 	{
// 		SAFE_DELETE_ARRAY(pPlayer->m_pBuf);
// 	}
// }

BOOL CSoundPlayer::Stop()
{
	// Stop Play
	if(m_hWaveOut == NULL)
		return true;

	if(!(m_waveHdr.dwFlags&WHDR_DONE))
		return false;

	waveOutReset(m_hWaveOut);

	assert(m_waveHdr.dwFlags & WHDR_DONE);
	waveOutUnprepareHeader(m_hWaveOut,&m_waveHdr,sizeof(WAVEHDR));

	waveOutClose(m_hWaveOut);
	m_hWaveOut = NULL;

	SAFE_DELETE_ARRAY(m_pBuf);
	return true;
}

BOOL CSoundPlayer::IsPlaying()
{
	if(m_pBuf)
		return TRUE;

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
/////CCheckMic
///////////////////////////////////////////////////////////////////////////////
CCheckMic::CCheckMic():
m_isStart(false)
, m_nLevel(0)
, m_nFrameSize(0)
, m_hWaveIn(NULL)
{
	memset(m_arBuffer, 0, sizeof(m_arBuffer));
	memset(m_arWaveHdr, 0, sizeof m_arWaveHdr);
}

CCheckMic::~CCheckMic()
{

}

BOOL CCheckMic::Start(WAVEFORMATEX &wfx)
{
	return false;
	Stop();

	m_nFrameSize = wfx.nSamplesPerSec * 20 * wfx.nChannels/ 1000;
	
	if(GetAudioEngine() == NULL || AllocAudioMem()!=0)
		return false;

	int waveInID = GetAudioEngine()->GetRecorderID();

	if(MMSYSERR_NOERROR != waveInOpen(&m_hWaveIn,waveInID,	&wfx,
		(DWORD_PTR)waveIn,	(DWORD_PTR)this,	CALLBACK_FUNCTION))
	{
		assert(0);
		FreeAudioMem();
		return false;
	}

	for( int i = 0; i< CACHE_FRAME_CNT; i++)
	{
		memset(&m_arWaveHdr[i],0,sizeof(WAVEHDR));
		m_arWaveHdr[i].dwUser = i;
		m_arWaveHdr[i].lpData = m_arBuffer[i];
		m_arWaveHdr[i].dwBufferLength = sizeof(short) * m_nFrameSize;

		memset(m_arWaveHdr[i].lpData,0,m_arWaveHdr[i].dwBufferLength);
		if(MMSYSERR_NOERROR  != waveInPrepareHeader(m_hWaveIn,&m_arWaveHdr[i],sizeof(WAVEHDR))
			|| MMSYSERR_NOERROR  != waveInAddBuffer(m_hWaveIn,&m_arWaveHdr[i],sizeof(WAVEHDR)))
		{
			assert(0);
			FreeAudioMem();
			m_isStart = false;
			return false;
		}
	}

	m_isStart = true;
	waveInStart(m_hWaveIn);

	return true;
}

void CCheckMic::Stop()
{
	if(m_hWaveIn == NULL)
		return;

	m_isStart = false;
	
	waveInReset(m_hWaveIn);
	
	for(int i = 0; i < CACHE_FRAME_CNT;i++)
	{
		assert(m_arWaveHdr[i].dwFlags & WHDR_DONE);
		waveInUnprepareHeader(m_hWaveIn,&m_arWaveHdr[i],sizeof(WAVEHDR));
	}	

	waveInClose(m_hWaveIn);
	m_hWaveIn = NULL;
	FreeAudioMem();
}

BOOL CCheckMic::IsSampling()
{
	return m_isStart;
}

double CCheckMic::GetLevel()
{
	if(m_nLevel < 0)
		m_nLevel = 0;
	else if(m_nLevel > 100)
		m_nLevel = 100;
	return static_cast<double>(m_nLevel) / 100.0;
}

int	CalcSoundVolume2(const audio::int16_t* p, UINT len)
{
	float sum=1.0f;
	for (int i=0;i<len;i++){
		sum += static_cast<float>(p[i] * p[i]);
	}

	float micLevel = sqrtf(sum / static_cast<float>(len));
	float dPeakMic=20.0f*log10f(micLevel / 32768.0f);
	if (dPeakMic < -96.0f)
		dPeakMic = -96.0f;

	float level =  (1.0f + dPeakMic / 96.0f);
	float vol = (level*80.0f-30.f)*2;

	if(vol > 100)
		vol = 100;
	else if(vol < 0)
		vol = 0;
	return vol;
}

void CCheckMic::waveIn(HWAVEIN hwi, UINT uMsg, 
					 DWORD dwUser,DWORD dw1, DWORD dw2)
{
	CCheckMic *pcWaveIn = reinterpret_cast<CCheckMic*>(dwUser);
	if(pcWaveIn == NULL || !pcWaveIn->m_isStart)
	{
		return;
	}

	if( uMsg == MM_WIM_CLOSE || uMsg == WM_QUIT)
	{
		pcWaveIn->m_isStart = false;
		return;
	}
	else if( uMsg == MM_WIM_OPEN )
		return;

	if(uMsg != MM_WIM_DATA)
		return;

	WAVEHDR *pHdr = reinterpret_cast<WAVEHDR*>(dw1);
	
	int nSize = pHdr->dwBufferLength / 2;
	short *pwData = reinterpret_cast<short*>(pHdr->lpData);

	pcWaveIn->m_nLevel = CalcSoundVolume2(pwData,nSize);

	assert(pHdr->dwBufferLength == pHdr->dwBytesRecorded);
	memset(pHdr->lpData, 0, pHdr->dwBufferLength);
	if(MMSYSERR_NOERROR != waveInAddBuffer(pcWaveIn->m_hWaveIn,pHdr,sizeof(WAVEHDR)))
	{
		assert(0);
	}

	return;
}


int CCheckMic::AllocAudioMem()
{
	for(int i = 0; i < CACHE_FRAME_CNT; i++)
	{  
		m_arBuffer[i] = (char*) new char[sizeof(short) * m_nFrameSize];
		if(m_arBuffer[i] == NULL)
		{
			assert(0);
			return -1;
		}
		memset(m_arBuffer[i], 0, sizeof(short) * m_nFrameSize);
	}

	return 0;
}

void CCheckMic::FreeAudioMem()
{
	for(int i = 0; i < CACHE_FRAME_CNT; i++)
	{  
		if( NULL != m_arBuffer[i] )
		{
			delete[] m_arBuffer[i];
			m_arBuffer[i] = NULL;
		}
	}
}
