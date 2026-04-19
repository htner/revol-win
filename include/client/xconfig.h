#pragma once

#include "xbase.h"

NAMESPACE_BEGIN(xconfig)

struct IConfig
    : public IObject
{
    virtual LPCTSTR     GetConfigItem(__in LPCTSTR)                             = 0;

    virtual BOOL        SetConfigItem(__in LPCTSTR, __in LPCTSTR)               = 0;
    virtual BOOL        SetConfigItem(__in LPCTSTR, __in int)                   = 0;

    virtual BOOL        LoadConfig(__in LPCTSTR)                                = 0;
    virtual BOOL        SaveConfig(__in LPCTSTR)                                = 0;
};

extern "C" IConfig*     CoCreateConfig();

NAMESPACE_END(xconfig)