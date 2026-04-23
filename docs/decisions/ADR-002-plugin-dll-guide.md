# ADR-002: 플러그인 DLL 빌드 및 배포 가이드

- **날짜**: 2026-04-23
- **상태**: 확정
- **관련 Phase**: Phase 7 Step 1

---

## 결정 요약

SageNexus의 플러그인은 DLL로 빌드하여 앱 실행 시 동적으로 로드한다.
고객사 전용 기능은 `IPlugin` 인터페이스를 구현한 DLL을 별도 private 레포에서 빌드하고,
배포 폴더의 `plugins\` 디렉터리에 드랍하는 방식으로 납품한다.

---

## 배경

Phase 6까지는 내장 플러그인만 메타데이터(ID + Name)로 등록했다.
Phase 7에서 DLL 동적 로딩 인프라(`PluginLoader`, `PluginManager`)가 완성되어
외부 DLL 플러그인을 코어 바이너리 교체 없이 추가할 수 있게 됐다.

---

## ABI 규약

모든 SageNexus 플러그인 DLL은 아래 세 가지 C 스타일 함수를 반드시 export해야 한다.

```cpp
extern "C" __declspec(dllexport) int      SagePlugin_GetAbiVersion();
extern "C" __declspec(dllexport) IPlugin* SagePlugin_Create();
extern "C" __declspec(dllexport) void     SagePlugin_Destroy(IPlugin* pPlugin);
```

### ABI 버전

- 현재 버전: `1` (`Define.h`의 `PLUGIN_ABI_VERSION`)
- 코어 로드 시 `SagePlugin_GetAbiVersion()` 반환값과 `PLUGIN_ABI_VERSION`이 일치하지 않으면 로드를 거부하고 해당 플러그인만 비활성화한다.
- ABI 버전은 `IPlugin` 인터페이스 시그니처가 변경될 때만 올린다.

### IPlugin 인터페이스

플러그인이 구현해야 하는 인터페이스는 다음과 같다.

```cpp
class IPlugin
{
public:
    virtual ~IPlugin() {}
    virtual int            GetAbiVersion() const = 0;  // PLUGIN_ABI_VERSION 반환
    virtual const CString& GetPluginId()   const = 0;  // 플러그인 고유 ID
    virtual const CString& GetPluginName() const = 0;  // UI 표시명
};
```

`IPlugin.h`는 코어 레포(`SageNexus/App/domain/interfaces/IPlugin.h`)에 있다.
외부 레포에서 DLL을 빌드할 때는 이 헤더를 include path에 추가하거나 동일 내용으로 복사한다.
단, 헤더를 복사하는 경우 ABI 버전 상수(`PLUGIN_ABI_VERSION`)와 vtable 레이아웃이 코어와 반드시 일치해야 한다.

> 참고: 외부 플러그인이 pch.h 의존 없이 헤더를 사용할 수 있도록
> 독립 SDK 헤더(`SagePluginSDK.h`)를 별도 제공하는 방안은 Phase 7 Step 2에서 결정한다.

---

## 플러그인 DLL 프로젝트 구성

### 빌드 설정

| 항목 | 값 |
|---|---|
| ConfigurationType | DynamicLibrary |
| PlatformToolset | v145 |
| CharacterSet | Unicode |
| UseOfAtl | Static |
| Platform | x64 |
| AdditionalIncludeDirectories | `$(SolutionDir)SageNexus` (코어 헤더 참조 시) |

### 필수 소스 구성

```
SageNexus-Plugin-<CompanyName>/
  pch.h / pch.cpp          ← PCH
  <CompanyName>Plugin.h    ← IPlugin 구현체 선언
  <CompanyName>Plugin.cpp  ← IPlugin 구현 + C export 함수
```

### 구현 예시

```cpp
class AcmePlugin : public IPlugin
{
public:
    AcmePlugin();
    int            GetAbiVersion() const override;
    const CString& GetPluginId()   const override;
    const CString& GetPluginName() const override;
private:
    CString m_strPluginId;
    CString m_strPluginName;
};

extern "C" __declspec(dllexport) int SagePlugin_GetAbiVersion()
{
    return PLUGIN_ABI_VERSION;
}

extern "C" __declspec(dllexport) IPlugin* SagePlugin_Create()
{
    return new AcmePlugin();
}

extern "C" __declspec(dllexport) void SagePlugin_Destroy(IPlugin* pPlugin)
{
    delete pPlugin;
}
```

---

## 배포 경로

```
<AppDir>\
  SageNexus.exe
  plugins\
    SageNexus-Plugin-Acme.dll    ← 고객사 전용 DLL 드랍 위치
    SageNexus-Plugin-Beta.dll
```

앱 시작 시 `PluginManager::LoadPluginsFromDirectory()`가 `plugins\*.dll`을 스캔하여 자동 로드한다.

---

## 로드 실패 정책

- DLL 로드 실패 (`LoadLibraryW` 오류, ABI 버전 불일치, export 함수 없음)는 해당 플러그인만 비활성화한다.
- 앱은 계속 실행된다.
- 실패 원인은 로그에 기록되며, `PluginManager::LoadPluginsFromDirectory()` 반환값(`BOOL`)과 `strErrors`로 확인할 수 있다.

---

## 레포 구성 원칙

- `SageNexus` (코어): 공통 코어 + `IPlugin` 인터페이스. 고객사 전용 코드 0줄.
- `SageNexus-Plugin-<CompanyName>` (고객사 private): `IPlugin` 구현체 + 전용 profile + 전용 리소스.

포크 금지. 모든 고객사 차이는 DLL 플러그인과 `SolutionProfile`로 수용한다.

---

## 대안 검토

| 안 | 내용 | 기각 이유 |
|---|---|---|
| 빌드 타임 링크 | 고객사 코드를 코어와 함께 컴파일 | 고객사마다 별도 빌드 필요, 코어 바이너리 공유 불가 |
| 포크 | 고객사별 레포 복제 | 유지보수 비용 지수 증가, 플러그인 아키텍처 무효화 |
| **DLL 동적 로딩 (채택)** | 고객사 DLL만 교체 | 코어 바이너리 공유, 독립 빌드·배포 가능 |

---

## 영향 범위

- `PluginLoader`: `LoadLibraryW` / `FreeLibrary` 기반 로드/언로드 구현 완료
- `PluginManager`: 내장 플러그인과 DLL 플러그인을 `m_arrPlugins` 단일 목록으로 통합 관리
- `SageApp`: 초기화 시 `LoadPluginsFromDirectory(GetAppDir() + L"\\plugins")` 호출
