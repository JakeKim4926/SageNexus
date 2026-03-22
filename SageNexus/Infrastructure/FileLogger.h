#pragma once
#include "pch.h"
#include "../EnumDefine.h"

class FileLogger
{
public:
    explicit FileLogger(const CString& strLogDir);
    ~FileLogger();

    BOOL Initialize();
    void Flush();

    void Log(LogLevel level, const CString& strMessage);
    void LogDebug(const CString& strMessage);
    void LogInfo(const CString& strMessage);
    void LogWarning(const CString& strMessage);
    void LogError(const CString& strMessage);

private:
    CString BuildLogLine(LogLevel level, const CString& strMessage) const;
    CString GetTimestamp() const;
    const char* GetLevelLabel(LogLevel level) const;

    CString      m_strLogDir;
    std::ofstream m_fileStream;
    std::mutex   m_mutex;
    BOOL         m_bInitialized;
};
