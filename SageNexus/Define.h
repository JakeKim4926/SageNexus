#pragma once

// ============================================================
// App 메시지
// ============================================================
#define WM_WEBVIEW_READY        (WM_USER + 1)
#define WM_BRIDGE_MESSAGE       (WM_USER + 2)
#define WM_WORKFLOW_PROGRESS    (WM_USER + 3)
#define WM_WORKFLOW_COMPLETE    (WM_USER + 4)
#define WM_JOB_QUEUE_CHANGED    (WM_USER + 5)
#define WM_BRIDGE_DEFERRED_CMD  (WM_USER + 6)

// ============================================================
// Scheduler
// ============================================================
constexpr UINT SCHEDULER_TIMER_ID  = 1001;
constexpr UINT SCHEDULER_TICK_MS   = 60000;
constexpr const wchar_t* SCHEDULER_FILE_NAME = L"scheduler.json";

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
constexpr const wchar_t* WEB_VIRTUAL_HOST         = L"https://app.sagenexus/index.html";

// ============================================================
// Config / Log
// ============================================================
constexpr const wchar_t* PROFILE_FILE_NAME   = L"profile.json";
constexpr const wchar_t* CONFIG_FILE_NAME    = L"settings.json";
constexpr const wchar_t* DATA_DIR_NAME       = L"Data";
constexpr const wchar_t* LOG_DIR_NAME        = L"Logs";
constexpr const wchar_t* LOG_MOVED_DIR_NAME  = L"Moved";
constexpr const wchar_t* LOG_ERROR_DIR_NAME  = L"Error";
constexpr const wchar_t* LOG_VALUE_TRUE      = L"true";
constexpr const wchar_t* LOG_VALUE_FALSE     = L"false";
constexpr const wchar_t* LOG_EXECUTION_LINE_FORMAT = L"[%s] runId=%s operation=%s success=%s source=%s rows=%d columns=%d output=%s error=%s";

// ============================================================
// User Data (AppData)
// ============================================================
// 설치 폴더: exe, WebView2Loader.dll, profile.json, plugins.
// WebView2 UI 리소스는 exe에 임베딩된다.
// 사용자 데이터 폴더: settings.json, credentials.dat, Logs, Data, WebViewData
constexpr const wchar_t* USER_DATA_APP_NAME  = L"SageNexus";

// ============================================================
// Bridge
// ============================================================
constexpr int BRIDGE_REQUEST_TIMEOUT_MS = 30000;

// ============================================================
// Security
// ============================================================
constexpr const wchar_t* CREDENTIALS_FILE_NAME = L"credentials.dat";

// ============================================================
// Import
// ============================================================
constexpr int CSV_MAX_PREVIEW_ROWS = 1000;

// ============================================================
// API Call
// ============================================================
constexpr int API_DEFAULT_TIMEOUT_MS = 30000;

// ============================================================
// API Connector
// ============================================================
constexpr const wchar_t* CONNECTORS_FILE_NAME = L"connectors.json";

// ============================================================
// Plugin
// ============================================================
constexpr int PLUGIN_ABI_VERSION             = 3;
constexpr const wchar_t* PLUGINS_DIR_NAME    = L"plugins";

// ============================================================
// Workflow Step Types
// WorkflowStep::m_strStepType에 사용하는 문자열 상수
// ============================================================
constexpr const wchar_t* STEP_TYPE_IMPORT      = L"import";
constexpr const wchar_t* STEP_TYPE_TRANSFORM   = L"transform";
constexpr const wchar_t* STEP_TYPE_EXPORT      = L"export";
constexpr const wchar_t* STEP_TYPE_WEB_EXTRACT = L"webExtract";
constexpr const wchar_t* STEP_TYPE_SEND_EMAIL  = L"sendEmail";
constexpr const wchar_t* STEP_TYPE_CALL_API    = L"callApi";
constexpr const wchar_t* STEP_TYPE_CONDITION   = L"condition";
