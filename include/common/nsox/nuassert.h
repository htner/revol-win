#ifndef _NU_ASSERT_H
#define _NU_ASSERT_H

#ifdef NSOX_WIN32
#include "assert.h"
#define  nu_assert assert
#else
#define  nu_assert
#endif

#endif

