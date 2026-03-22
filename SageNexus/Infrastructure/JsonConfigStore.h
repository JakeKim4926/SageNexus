#pragma once
#include "pch.h"

// Phase 1: 단순 flat JSON 오브젝트 읽기/쓰기
// {"key": "value", "key2": 42, "key3": true} 형태만 지원
class JsonConfigStore
{
public:
    explicit JsonConfigStore(const CString& strConfigDir);

    BOOL Load();
    BOOL Save() const;

    CString GetString(const CString& strKey, const CString& strDefault = L"") const;
    int     GetInt(const CString& strKey, int nDefault = 0) const;
    BOOL    GetBool(const CString& strKey, BOOL bDefault = FALSE) const;

    void SetString(const CString& strKey, const CString& strValue);
    void SetInt(const CString& strKey, int nValue);
    void SetBool(const CString& strKey, BOOL bValue);

private:
    CString BuildFilePath() const;
    std::string BuildJsonText() const;
    void ParseJsonText(const std::string& strJson);

    // 값은 모두 문자열로 저장, 타입은 필요 시 변환
    std::map<std::string, std::string> m_mapEntries;
    CString m_strConfigDir;
};
