#ifndef _PKG_TAG_H
#define _PKG_TAG_H

#include "common/int_types.h"
#include <map>
#include <string>

typedef uint16_t IM_TAG_TYPE;
/*
typedef std::map<IM_TAG_TYPE, std::string> MapTag2String;
typedef MapTag2String::iterator Iter_MapTag2String;
typedef MapTag2String::const_iterator cIter_MapTag2String;
typedef std::pair<IM_TAG_TYPE, std::string> PairTag2String;
typedef MapTag2String::value_type vt_MapTag2String;

typedef std::map<std::string, uint32_t> MapString2Uint32;
typedef MapString2Uint32::iterator Iter_MapString2Uint32;
typedef MapString2Uint32::const_iterator cIter_MapString2Uint32;
typedef std::pair<std::string, uint32_t> PairString2Uint32;
typedef MapString2Uint32::value_type vt_MapString2Uint32;
*/
#define TAG_NO_USE	0x0000

// Tag for uinfo
#define UIT_UID			0x1000
#define UIT_ACCOUNT		0x0001
#define UIT_NICK        0x0002
//#define UIT_CONF_VER	0x0002
#define UIT_BIRTHDAY	0x0003
//#define UIT_SCRNAME	0x0004
#define UIT_GENDER	0x0005
//#define UIT_MOBILE_PHONE	0x0006
#define UIT_RESUME		0x0007
#define UIT_AREA	0x0008
#define UIT_PROVINCE	0x0009
#define UIT_CITY		0x000A
#define UIT_VALIDATE    0x000B
//#define UIT_MMCHAT_ID	0x000B
//#define UIT_PORTRAIT_IDX 0x000C
#define UIT_NAME		0x000D
#define UIT_PHONE		0x000E
#define UIT_FAX		0x000F
#define UIT_ADDRESS	0x0010
#define UIT_ZIP_CODE	0x0011
#define UIT_S_PUBLIC_KEY	0x0012
#define UIT_C_PUBLIC_KEY	0x0013
#define UIT_PREF_INFO	0x0014
#define UIT_PREF_BAN	0x0015
#define UIT_PREF_AUTH	0x0016
#define UIT_LAST_LOGIN	0x0017
#define UIT_EMAIL		0x0018
#define UIT_SPARE_FIELD	0x0019
#define UIT_CONF_BITS	0x001A
#define UIT_IDLE_TIME	0x001B
#define UIT_HOTKEY_MAINWIN	0x001C
#define UIT_HOTKEY_MSGWIN	0x001D
#define UIT_SUBJECT	0x001E
#define UIT_EMAIL_CHK	0x001F
#define UIT_EMAIL_PASS	0x0021
#define UIT_UPDATE_VER	0x0023
#define UIT_RESCODE	0x0025
#define UIT_VIDEOINFO	0x002B
#define UIT_MSGKEY		0x002C
#define UIT_COOKIE      0x002D
#define UIT_PASS	0x0031	
//#define UIT_PORT	0x0032
//#define UIT_LOCALIP 0x0033
//#define UIT_EXPIP 	0x0034
#define UIT_LAST_TIME 0x0032
#define UIT_LAST_IP 0x0033
#define UIT_VERSION   0x0034
#define UIT_USERJF 0x0035
#define UIT_SIGN 0x0036
#define UIT_PASSPORT 0x0037
#define UIT_INTRO 0x0038
#define UIT_FBID 0x0039
#define UIT_FBTOKEN 0x0040
#define UIT_MEDAL	0x0041
#define UIT_GOLD	0x0042
#define UIT_SILVER	0x0043
#define UIT_FLOWER	0x0044

#define UIT_JOIN_SESSION_PRIVACY 	0x0045 //privacy
#define UIT_PUBLIC_VISIT_SESSION  0x0046 //visit
#define UIT_PUBLIC_COLLECT_SESSION  0x0047 //collect



#define SIT_NAME 0x0100
#define SIT_SID 0x0101
#define SIT_ISPUB 0x0102
#define SIT_ISLIMIT 0x0103
#define SIT_BULLETIN 0x0104
#define SIT_LIMITPASSWD 0x0105
#define SIT_PID 0x0106
#define SIT_OWNER 0x0107
#define SIT_MEMBERLIMIT 0x0108   //在線人數上限
#define SIT_ISLIMITPASSWD 0x0109
#define SIT_CREATETIME 0x0110
#define SIT_CODECRATE 0x0111
#define SIT_BHASPASSWD 0x0112
#define SIT_STYLE 0x0113
#define SIT_MICROTIME 0x0114
#define SIT_ALIASESID 0x0115
#define SIT_TYPE 0x0116
#define SIT_TYPESTRING 0x0117
#define SIT_JIFEN 0x0118
#define SIT_PAIMING 0x0119
#define SIT_REALTIME 0x011a
#define SIT_BULLETINTIMESTAMP 0x011b
#define SIT_INTRODUCE 0x011c
#define SIT_VOICEQC 0x011d
#define SIT_INFO_OPERATOR 0x011e
#define SIT_ISPTYPE 0x011f


#define SIT_VTXTLENGTH 0x0120
#define SIT_VTXTINTERVAL 0x0121 //
#define SIT_VTXTFORMAT 0x0122
#define SIT_VTXTINITALWAIT 0x0123

#define SIT_VMICLIMITFREE 0x0124
#define SIT_VMICLIMITQUEUE 0x0125
#define SIT_VJOINJIFEN 0x0126
#define SIT_VMLISTLIMIT 0x0127

#define SIT_APPLYINFO 0x0128
#define SIT_APPLYUJIFEN 0x0129
#define SIT_APPLYUSJIFEN 0x012a

#define SIT_SEARCHLIMIT 0x012b

#define SIT_GRADE 0x012c
#define SIT_DISABLE 0x012d
#define SIT_APPLYAUTO 0x012e
#define SIT_CHANGEQUALITY 0x012f
#define SIT_RECEPTION 0x0130
#define SIT_PROPERTY 0x0131
#define SIT_ORDER 0x0132
#define SIT_HALL 0x0133
#define SIT_MEMBERCOUNT 0x0134
#define SIT_ONLINECOUNT 0x0135
#define SIT_MAXMEMBER   0x0136 //最高會員上限
#define SIT_SLOGAN 0x0137
#define SIT_LABLES 0x0138
#define SIT_FONDCOUNT 0x0139
#define SIT_FLASHURL 0x013a
#define SIT_NAVIGATION 0x013b
#define SIT_OPENPLUGIN 0x013c
#define SIT_SHOWFLOWER 0x013d
#define SIT_COLLECTIONCOUNT 0x013e


// tag for 广告
#define ADT_TAG		0x8000

#endif
