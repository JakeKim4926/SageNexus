#pragma once
#include "pch.h"

class IPlugin
{
public:
    virtual ~IPlugin() {}

    virtual int     GetAbiVersion() const = 0;
    virtual LPCWSTR GetPluginId()   const = 0;
    virtual LPCWSTR GetPluginName() const = 0;
};
