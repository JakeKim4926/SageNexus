#pragma once
#include "pch.h"

class JsonUtils
{
public:
    static std::string WideToUtf8Text(const CString& strWide);
    static CString Utf8ToWideText(const std::string& strUtf8);
    static CString ExtractString(const CString& strJson, const CString& strKey);
    static BOOL ExtractBool(const CString& strJson, const CString& strKey);
    static int ExtractInt(const CString& strJson, const CString& strKey, int nDefaultValue);
    static CString UnescapeText(const CString& strValue);
    static CString EscapeString(const CString& strValue);
    static std::string EscapeText(const CString& strValue);
    static BOOL HasArrayKey(const std::string& strJson, const std::string& strKey);
    static std::string ExtractArrayText(const std::string& strJson, const std::string& strKey);
    static void ExtractObjects(const std::string& strArrayText, std::vector<std::string>& outObjects);
    static CString ExtractObjectString(const std::string& strObject, const CString& strKey);
    static BOOL ExtractObjectBool(const std::string& strObject, const CString& strKey);

private:
    static size_t FindArrayEnd(const std::string& strJson, size_t nArrayStart);
};
