#pragma once

#ifdef _DEBUG
	#include "debugTrace.hpp"

	#define CALC_RUNTIME_THRESHODE(name,threshold)	CalcRuntime	__##name##__(_T(#name),threshold);
	#define CALC_RUNTIME(name)						CALC_RUNTIME_THRESHODE(name,0)

#else
	#define CALC_RUNTIME(name)	
	#define CALC_RUNTIME_THRESHODE(name,threshold)
#endif // _DEBUG

