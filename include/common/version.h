#ifndef YY_VERSION_H_
#define YY_VERSION_H_

#define PROTOCOL_VERSION 200811040


#ifdef WIN32
#define __BUILD_MACH__ "win"
#else
#define __BUILD_MACH__ "unkown"
#endif

#ifdef REX_TW
#define __VERSION_REGION__ "tw"
#elif REX_US
#define __VERSION_REGION__ "us"
#else
#define __VERSION_REGION__ "union"
#endif

#endif /*VERSION_H_*/
