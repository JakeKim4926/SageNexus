#pragma once
#include "pch.h"

class IPlugin
{
public:
    virtual ~IPlugin() {}

    virtual int            GetAbiVersion() const = 0;
    virtual const CString& GetPluginId() const   = 0;
    virtual const CString& GetPluginName() const = 0;
};
