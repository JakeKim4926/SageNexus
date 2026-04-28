#pragma once
#include "pch.h"
#include "EnumDefine.h"
#include "app/domain/model/ExecutionRecord.h"

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
    void LogExecutionRecord(const ExecutionRecord& record);

private:
    CString BuildLogLine(LogLevel level, const CString& strMessage) const;
    CString BuildExecutionLogLine(const ExecutionRecord& record) const;
    CString BuildDailyLogPath(const CString& strFolderName) const;
    CString GetDateFileName() const;
    CString GetTimestamp() const;
    const char* GetLevelLabel(LogLevel level) const;
    BOOL AppendDailyLog(const CString& strFolderName, const CString& strLine);
    BOOL EnsureDirectory(const CString& strPath) const;

    CString      m_strLogDir;
    std::ofstream m_fileStream;
    std::mutex   m_mutex;
    BOOL         m_bInitialized;
};
