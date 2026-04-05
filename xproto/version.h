/*
 * version.h
 *
 */
//#include "../client/autover/xproto.autover"
#define BUILD_VER 0
#define VER_Major                   1
#define VER_Minor                   0
#define VER_Release                 12702
#define VER_Build                   BUILD_VER
#define VER_Version                 MAKELONG( MAKEWORD( VER_Major, VER_Mino r), VER_Release )

#define _Stringizing( v )           #v
#define _VerJoin( a, b, c, d )      _Stringizing( a.b.c.d )

#define STR_BuildDate               TEXT(__DATE__)
#define STR_BuildTime               TEXT(__TIME__)
#define STR_BuilDateTime            TEXT(__DATE__) TEXT(" ") TEXT(__TIME__)
#define STR_Version                 TEXT(_VerJoin(VER_Major, VER_Minor, VER_Release, VER_Build))
#define STR_AppName                 TEXT("xproto")
#define STR_Author                  TEXT("Team Rex")
#define STR_Corporation             TEXT("REX.COM")
#define STR_Web                     TEXT("www.rex.com")
#define STR_Email                   TEXT("support@rex.com")
#define STR_WebUrl                  TEXT("http://") STR_Web
#define STR_EmailUrl                TEXT("mailto:") STR_Email TEXT("?Subject=") STR_AppName
#define STR_Description             TEXT("Rex X-Proto Kernel Component")
#define STR_Copyright               TEXT("Copyright (C) 2009-2010 ") STR_Corporation TEXT(", All rights reserved")
#define EXT_Exe                     TEXT(".exe")
#define EXT_Dll                     TEXT(".dll")
