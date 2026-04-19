#pragma once

#include "PUBFUNC.h"

#include <string>

namespace helper
{
	class  usound
	{
	public:
		static int CalcSoundVolume(const char* p, UINT len);
		static int ToVisibleVolume(int vol);
	};
}

#include "usound.hpp"