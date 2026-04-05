#include "common/nsox/nulog.h"
#include "audio/AudioInput.h"
#include "atlbase.h"
#include "audio/dsound.h"
#include "windows.h"
#include "AudioFilter_Speex.h"
#include "AudioFilter_Celt.h"

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
		if (dwReason == DLL_PROCESS_ATTACH)
		{
				DisableThreadLibraryCalls(hInstance);
		}
		else if (dwReason == DLL_PROCESS_DETACH)
		{
				
		}
		return TRUE;    // ok
}





