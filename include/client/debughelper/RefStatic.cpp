#include "stdafx.h"
#include "RefStatic.h"

#ifdef REF_STATIC_MODULE

	#pragma init_seg (compiler)
	CRefStatic* CRefStatic::_root = NULL;
	CRefStatic g__RefStaticRoot__(_T("RefStaticRoot"));														

#endif // REF_STATIC_MODULE
