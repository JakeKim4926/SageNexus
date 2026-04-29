#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <shlobj.h>
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
#include <set>
#include <cassert>

#include <WebView2.h>

std::string WideToUtf8(const CString& strWide);
CString Utf8ToWide(const std::string& strUtf8);
CString JsonExtractString(const CString& strJson, const CString& strKey);
BOOL JsonExtractBool(const CString& strJson, const CString& strKey);
CString UnescapeJsonString(const CString& str);
CString JsonEscapeString(const CString& str);
