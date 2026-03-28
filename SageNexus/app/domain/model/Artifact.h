#pragma once
#include "pch.h"

struct Artifact
{
    CString m_strId;
    CString m_strSourceName;
    CString m_strFilePath;
    CString m_strFormat;
    CString m_strCreatedAt;
    int     m_nRowCount    = 0;
    int     m_nColumnCount = 0;
};
