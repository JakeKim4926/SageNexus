#pragma once
#include "pch.h"
#include "app/domain/model/ExecutionRecord.h"

class ExecutionHistoryStore
{
public:
    ExecutionHistoryStore();

    BOOL SaveRecord(ExecutionRecord& record, CString& strError);
    BOOL LoadRecords(std::vector<ExecutionRecord>& arrRecords, CString& strError) const;

private:
    CString              BuildFilePath() const;
    BOOL                 EnsureDataDirectory(CString& strError) const;
    CString              GenerateRunId() const;
    CString              GetCurrentTimestamp() const;

    CString              SerializeRecords(const std::vector<ExecutionRecord>& arrRecords) const;
    CString              SerializeRecord(const ExecutionRecord& record) const;
    BOOL                 ParseRecords(const CString& strJson, std::vector<ExecutionRecord>& arrRecords, CString& strError) const;
    std::vector<CString> SplitJsonObjects(const CString& strContent) const;

    CString EscapeJsonString(const CString& str) const;
    CString UnescapeJsonString(const CString& str) const;
    CString ExtractStringField(const CString& strJson, const CString& strKey) const;
    int     ExtractIntField(const CString& strJson, const CString& strKey) const;
    BOOL    ExtractBoolField(const CString& strJson, const CString& strKey) const;

    static CString ReadFileAsWideString(const CString& strPath);
    static BOOL    WriteFileAsUtf8(const CString& strPath, const CString& strContent, CString& strError);
};
