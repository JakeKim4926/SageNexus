#pragma once
#include "pch.h"

struct PluginCommandInfo
{
    LPCWSTR m_pszTarget;
    LPCWSTR m_pszAction;

    PluginCommandInfo()
        : m_pszTarget(L"")
        , m_pszAction(L"")
    {}
};

struct PluginPageInfo
{
    LPCWSTR m_pszPageId;
    LPCWSTR m_pszPageName;
    LPCWSTR m_pszEntryPath;

    PluginPageInfo()
        : m_pszPageId(L"")
        , m_pszPageName(L"")
        , m_pszEntryPath(L"")
    {}
};

struct PluginHistoryInfo
{
    LPCWSTR m_pszTarget;
    LPCWSTR m_pszAction;
    LPCWSTR m_pszOperationType;

    PluginHistoryInfo()
        : m_pszTarget(L"")
        , m_pszAction(L"")
        , m_pszOperationType(L"")
    {}
};

class IPlugin
{
public:
    virtual ~IPlugin() {}

    virtual int     GetAbiVersion() const = 0;
    virtual LPCWSTR GetPluginId()   const = 0;
    virtual LPCWSTR GetPluginName() const = 0;

    virtual int  GetCommandCount() const
    {
        return 0;
    }

    virtual BOOL GetCommandInfo(int /*nIndex*/, PluginCommandInfo& /*outInfo*/) const
    {
        return FALSE;
    }

    virtual BOOL HandleCommand(
        const CString& /*strTarget*/,
        const CString& /*strAction*/,
        const CString& /*strRequestId*/,
        const CString& /*strPayloadJson*/,
        CString& /*strResponseJson*/,
        CString& strError)
    {
        strError = L"Plugin command is not implemented.";
        return FALSE;
    }

    virtual int GetPageCount() const
    {
        return 0;
    }

    virtual BOOL GetPageInfo(int /*nIndex*/, PluginPageInfo& /*outInfo*/) const
    {
        return FALSE;
    }

    virtual int GetHistoryInfoCount() const
    {
        return 0;
    }

    virtual BOOL GetHistoryInfo(int /*nIndex*/, PluginHistoryInfo& /*outInfo*/) const
    {
        return FALSE;
    }
};
