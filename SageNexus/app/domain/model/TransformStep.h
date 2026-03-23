#pragma once
#include "pch.h"

struct TransformStep
{
    CString m_strType;    // "trim" | "renameColumn" | "replaceValue"
    CString m_strColumn;  // 대상 컬럼 internalName ("" = 전체 컬럼)
    CString m_strParam1;  // renameColumn: 새 표시명 / replaceValue: 찾을 값
    CString m_strParam2;  // replaceValue: 바꿀 값
};
