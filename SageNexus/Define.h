#pragma once

// ============================================================
// App 메시지
// ============================================================
#define WM_WEBVIEW_READY        (WM_USER + 1)
#define WM_BRIDGE_MESSAGE       (WM_USER + 2)

// ============================================================
// Window
// ============================================================
constexpr int WINDOW_DEFAULT_WIDTH  = 1280;
constexpr int WINDOW_DEFAULT_HEIGHT = 800;
constexpr int WINDOW_MIN_WIDTH      = 900;
constexpr int WINDOW_MIN_HEIGHT     = 600;

constexpr const wchar_t* WINDOW_CLASS_NAME = L"SageNexusMainWindow";
constexpr const wchar_t* WINDOW_TITLE      = L"SageNexus";

// ============================================================
// WebView2
// ============================================================
constexpr const wchar_t* WEBVIEW_USER_DATA_FOLDER = L"WebViewData";
constexpr const wchar_t* WEB_ENTRY_FILE           = L"Web\\index.html";

// ============================================================
// Config / Log
// ============================================================
constexpr const wchar_t* CONFIG_FILE_NAME = L"settings.json";
constexpr const wchar_t* DATA_DIR_NAME    = L"Data";
constexpr const wchar_t* LOG_DIR_NAME     = L"Logs";

// ============================================================
// Bridge
// ============================================================
constexpr int BRIDGE_REQUEST_TIMEOUT_MS = 30000;
