#include "pch.h"
#include "FileLogger.h"
#include "Define.h"

FileLogger::FileLogger(const CString& strLogDir)
    : m_strLogDir(strLogDir)
    , m_bInitialized(FALSE)
{
}

FileLogger::~FileLogger()
{
    if (m_fileStream.is_open())
        m_fileStream.close();
}

BOOL FileLogger::Initialize()
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    CString strFileName;
    strFileName.Format(L"SageNexus_%04d%02d%02d_%02d%02d%02d.log",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);

    CString strPath = m_strLogDir + L"\\" + strFileName;

    m_fileStream.open(WideToUtf8(strPath), std::ios::out);
    if (!m_fileStream.is_open())
        return FALSE;

    m_bInitialized = TRUE;
    LogInfo(L"========== SageNexus Started ==========");
    return TRUE;
}

void FileLogger::Flush()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fileStream.is_open())
        m_fileStream.flush();
}

void FileLogger::Log(LogLevel level, const CString& strMessage)
{
    if (!m_bInitialized)
        return;

    std::string strLine = WideToUtf8(BuildLogLine(level, strMessage));
    std::lock_guard<std::mutex> lock(m_mutex);
    m_fileStream << strLine << "\n";
    m_fileStream.flush();
}

void FileLogger::LogDebug(const CString& strMessage)   { Log(LogLevel::Debug,   strMessage); }
void FileLogger::LogInfo(const CString& strMessage)    { Log(LogLevel::Info,    strMessage); }
void FileLogger::LogWarning(const CString& strMessage) { Log(LogLevel::Warning, strMessage); }
void FileLogger::LogError(const CString& strMessage)   { Log(LogLevel::Error,   strMessage); }

void FileLogger::LogExecutionRecord(const ExecutionRecord& record)
{
    if (!m_bInitialized)
        return;

    CString strFolderName = record.m_bSuccess ? LOG_MOVED_DIR_NAME : LOG_ERROR_DIR_NAME;
    AppendDailyLog(strFolderName, BuildExecutionLogLine(record));
}

CString FileLogger::BuildLogLine(LogLevel level, const CString& strMessage) const
{
    CString strLine;
    strLine.Format(L"[%s] [%hs] %s",
        (LPCWSTR)GetTimestamp(),
        GetLevelLabel(level),
        (LPCWSTR)strMessage);
    return strLine;
}

CString FileLogger::BuildExecutionLogLine(const ExecutionRecord& record) const
{
    CString strLine;
    strLine.Format(LOG_EXECUTION_LINE_FORMAT,
        (LPCWSTR)record.m_strTimestamp,
        (LPCWSTR)record.m_strRunId,
        (LPCWSTR)record.m_strOperationType,
        record.m_bSuccess ? LOG_VALUE_TRUE : LOG_VALUE_FALSE,
        (LPCWSTR)record.m_strSourceName,
        record.m_nRowCount,
        record.m_nColumnCount,
        (LPCWSTR)record.m_strOutputPath,
        (LPCWSTR)record.m_strErrorMessage);
    return strLine;
}

CString FileLogger::BuildDailyLogPath(const CString& strFolderName) const
{
    CString strFolderPath = m_strLogDir + L"\\" + strFolderName;
    return strFolderPath + L"\\" + GetDateFileName();
}

CString FileLogger::GetDateFileName() const
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    CString strFileName;
    strFileName.Format(L"%04d-%02d-%02d.log", st.wYear, st.wMonth, st.wDay);
    return strFileName;
}

CString FileLogger::GetTimestamp() const
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    CString strTime;
    strTime.Format(L"%04d-%02d-%02d %02d:%02d:%02d.%03d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    return strTime;
}

const char* FileLogger::GetLevelLabel(LogLevel level) const
{
    switch (level)
    {
    case LogLevel::Debug:   return "DEBUG";
    case LogLevel::Info:    return "INFO ";
    case LogLevel::Warning: return "WARN ";
    case LogLevel::Error:   return "ERROR";
    default:                return "?????";
    }
}

BOOL FileLogger::AppendDailyLog(const CString& strFolderName, const CString& strLine)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    CString strFolderPath = m_strLogDir + L"\\" + strFolderName;
    if (!EnsureDirectory(strFolderPath))
        return FALSE;

    CString strPath = BuildDailyLogPath(strFolderName);
    std::ofstream fileStream;
    fileStream.open(WideToUtf8(strPath), std::ios::out | std::ios::app);
    if (!fileStream.is_open())
        return FALSE;

    fileStream << WideToUtf8(strLine) << "\n";
    fileStream.close();
    return TRUE;
}

BOOL FileLogger::EnsureDirectory(const CString& strPath) const
{
    if (CreateDirectoryW(strPath, nullptr))
        return TRUE;

    DWORD dwErr = GetLastError();
    return (dwErr == ERROR_ALREADY_EXISTS) ? TRUE : FALSE;
}
