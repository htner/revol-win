//-----------------------------------------------------------------------------
// File: DSUtil.h
//
// Desc: 
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef DSUTIL_H
#define DSUTIL_H

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>

//-----------------------------------------------------------------------------
// Classes used by this header
//-----------------------------------------------------------------------------
class CSoundPlayer
{
public:
	CSoundPlayer();
	virtual ~CSoundPlayer();

	BOOL Init(HWND hWnd, GUID devGuid);
	//BOOL Play(LPTSTR waveFile);
	BOOL PlayMemory(BYTE* p, int len,WAVEFORMATEX& pwfx);
	BOOL Stop();
	BOOL IsPlaying();
protected:
	char  *m_pBuf;
	HWAVEOUT m_hWaveOut;
	WAVEHDR  m_waveHdr;

// 	static void CALLBACK waveOut(HWAVEIN hwi, UINT uMsg, 
// 							DWORD dwUser,DWORD dw1, DWORD dw2);

};

const int CACHE_FRAME_CNT = 8;
class CCheckMic{
public:
	CCheckMic();
	~CCheckMic();

	BOOL Start(WAVEFORMATEX &wfx);
	void Stop();
	BOOL IsSampling();
	double GetLevel();
private:
	static void CALLBACK waveIn(HWAVEIN hwi, UINT uMsg, 
				DWORD dwUser,DWORD dw1, DWORD dw2);

	WAVEHDR m_arWaveHdr[CACHE_FRAME_CNT];
	char*	m_arBuffer[CACHE_FRAME_CNT];
	HWAVEIN m_hWaveIn;

	int AllocAudioMem();
	void FreeAudioMem();

	int m_nFrameSize;
	bool m_isStart;
	int m_nLevel;
};

#endif // DSUTIL_H
