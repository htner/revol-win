#ifdef __cplusplus
#ifdef _DEBUG
#ifdef _WIN32
//#define _CRTDBG_MAP_ALLOC 
#include <cstdlib> 
#include <crtdbg.h>
#include <xdebug>
#include <xlocale>
#include <xiosbase>
#include <xtree>
#include <xlocnum>
#include <xmemory>
#include <list>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif //_WIN32
#endif //_DEBUG
#endif //__cplusplus
