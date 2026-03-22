#pragma once

enum class AppState
{
    Initializing,
    Ready,
    Busy,
    Error
};

enum class WebViewState
{
    NotCreated,
    Creating,
    Ready,
    Failed
};

enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error
};

enum class BridgeMessageType
{
    Unknown,
    Command,
    Response,
    Event
};

enum class NavigationItem
{
    Dashboard,
    DataViewer,
    Transform,
    Export,
    History,
    Settings
};
