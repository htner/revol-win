#pragma once

#include <MMSystem.h>
#include <string>
#include <common/xstring.h>

namespace helper
{
	class usystem
	{
	public:
		inline static LOGFONT	GetDefaultLF(void);

		inline static HDC		GetScreenDC(void);

		inline static void		AnalyseHotKey(DWORD dwKey, WORD& wVirtualKeyCode, WORD&	wModifiers);

		inline static void		ToHotKey(DWORD dwKey, WORD& wVirtualKeyCode, WORD&	wModifiers);

		inline static void		MakeHotKey(DWORD& dwKey, WORD& wVirtualKeyCode, WORD&	wModifiers);

		inline static void		ForceWindowShowOnTop(HWND hWnd);

		inline static WAVEFORMATEX GetWaveFormat(WORD wFormatTag, int nChannel, int nSample, int nBits);

		inline static bool		IsKeyPressed(int nKey);

		inline static void		OpenURL(LPCTSTR lpszURL);

		inline static xstring	GetKeyNameText(int nkey);

		inline static xstring	GetHotKeyName(int nkey);

		inline static xstring   GetSysInfo();

		inline static xstring	GetMacAddress();

		inline static INT32		time32 ();
	};
}
#include "usystem.hpp"