#include "../pch.h"
#include "FileLogger.h"

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
    strFileName.Format(L"SageNexus_%04d%02d%02d.log",
        st.wYear, st.wMonth, st.wDay);

    CString strPath = m_strLogDir + L"\\" + strFileName;

    // UTF-8 BOM 포함 로그 파일 오픈 (append 모드)
    m_fileStream.open(WideToUtf8(strPath), std::ios::app | std::ios::out);
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

CString FileLogger::BuildLogLine(LogLevel level, const CString& strMessage) const
{
    CString strLine;
    strLine.Format(L"[%s] [%hs] %s",
        (LPCWSTR)GetTimestamp(),
        GetLevelLabel(level),
        (LPCWSTR)strMessage);
    return strLine;
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
