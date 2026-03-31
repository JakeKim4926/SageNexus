#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <atlbase.h>
#include <atlstr.h>
#include <wrl.h>
#include <wrl/event.h>

#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <sstream>
#include <mutex>
#include <map>
#include <cassert>

#include <WebView2.h>

// UTF-8 변환 유틸리티
inline std::string WideToUtf8(const CString& strWide)
{
    int nLen = WideCharToMultiByte(CP_UTF8, 0, strWide, -1, nullptr, 0, nullptr, nullptr);
    if (nLen <= 1)
        return "";

    std::string strUtf8(nLen - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, strWide, -1, &strUtf8[0], nLen, nullptr, nullptr);
    return strUtf8;
}

inline CString Utf8ToWide(const std::string& strUtf8)
{
    int nLen = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
    if (nLen <= 1)
        return L"";

    CString strWide;
    MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, strWide.GetBuffer(nLen), nLen);
    strWide.ReleaseBuffer();
    return strWide;
}
