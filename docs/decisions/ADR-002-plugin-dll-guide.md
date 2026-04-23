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

### 필수 export 함수

모든 SageNexus 플러그인 DLL은 아래 세 가지 C 링키지 함수를 반드시 export해야 한다.

```cpp
extern "C" __declspec(dllexport) int      SagePlugin_GetAbiVersion();
extern "C" __declspec(dllexport) IPlugin* SagePlugin_Create();
extern "C" __declspec(dllexport) void     SagePlugin_Destroy(IPlugin* pPlugin);
```

### ABI 버전

- 현재 버전: `1` (`Define.h`의 `PLUGIN_ABI_VERSION`)
- 코어 로드 시 `SagePlugin_GetAbiVersion()` 반환값과 `PLUGIN_ABI_VERSION`이 일치하지 않으면 해당 플러그인만 비활성화한다.
- ABI 버전은 아래 항목 중 하나라도 변경될 때 올린다.
  - `IPlugin` 인터페이스 메서드 추가/제거/시그니처 변경
  - export 함수 시그니처 변경
  - 공개 구조체 레이아웃 변경
  - 문자열/메모리 소유권 계약 변경

### IPlugin 인터페이스

```cpp
class IPlugin
{
public:
    virtual ~IPlugin() {}
    virtual int     GetAbiVersion() const = 0;  // PLUGIN_ABI_VERSION 반환
    virtual LPCWSTR GetPluginId()   const = 0;  // 플러그인 고유 ID (정적 문자열)
    virtual LPCWSTR GetPluginName() const = 0;  // UI 표시명 (정적 문자열)
};
```

`GetPluginId()` / `GetPluginName()`은 DLL 소유의 정적 문자열 포인터를 반환한다.
호출자는 반환된 포인터를 직접 저장하지 않고 즉시 복사해야 한다.
`SagePlugin_Destroy()` 호출 이후에는 이 포인터가 유효하지 않을 수 있다.

### 예외 규칙

export 함수(`SagePlugin_GetAbiVersion`, `SagePlugin_Create`, `SagePlugin_Destroy`)는
예외를 DLL 경계 밖으로 던지면 안 된다.
내부에서 발생한 예외는 반드시 DLL 내에서 catch하고 적절한 반환값으로 변환한다.

`SagePlugin_Create()`가 NULL을 반환하면 코어는 로드 실패로 처리한다.

### 언로드 규칙

`FreeLibrary` 호출 전에 반드시 아래 조건이 충족되어야 한다.

- 해당 DLL에서 생성한 모든 `IPlugin` 인스턴스의 `SagePlugin_Destroy()` 완료
- 해당 DLL 코드에서 실행 중인 스레드 없음
- 해당 DLL이 등록한 콜백/핸들 정리 완료

`PluginLoader::Unload()`는 위 순서를 보장하도록 구현되어 있다.

### 빌드 일치 요건

DLL과 코어는 아래 설정이 반드시 일치해야 한다.

| 항목 | 요구값 |
|---|---|
| Compiler | MSVC v145 이상 |
| Platform | x64 |
| CharacterSet | Unicode |
| CRT | 동적 링크 (`/MD` 또는 `/MDd`) |
| UseOfAtl | Static |
| ExceptionHandling | `/EHsc` |

설정이 다르면 ABI 버전이 같더라도 런타임 크래시가 발생할 수 있다.

---

## DLL 로드 보안

`PluginLoader`는 `LoadLibraryExW`에 아래 플래그를 사용한다.

```cpp
LoadLibraryExW(strDllPath, NULL,
    LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
```

이를 통해 DLL search order 하이재킹을 방지하고,
명시된 경로의 DLL과 시스템 DLL만 로드한다.

---

## 플러그인 DLL 프로젝트 구성

### 빌드 설정

| 항목 | 값 |
|---|---|
| ConfigurationType | DynamicLibrary |
| PlatformToolset | v145 이상 |
| CharacterSet | Unicode |
| UseOfAtl | Static |
| RuntimeLibrary | MultiThreadedDLL (`/MD`) |
| Platform | x64 |

### 헤더 참조 방식

플러그인은 별도 제공되는 버전 고정 SDK 헤더를 기준으로 빌드한다.
코어 레포 헤더를 직접 참조하거나 복사하지 않는다.

> SDK 헤더 배포 방식은 Phase 7 Step 2에서 확정한다.
> Step 2 완료 전까지는 코어 레포를 submodule 또는 sibling 경로로 참조한다.

### 필수 소스 구성

```
SageNexus-Plugin-<CompanyName>/
  pch.h / pch.cpp                 ← PCH
  <CompanyName>Plugin.h           ← IPlugin 구현체 선언
  <CompanyName>Plugin.cpp         ← IPlugin 구현 + export 함수
```

### 구현 예시

```cpp
class AcmePlugin : public IPlugin
{
public:
    int     GetAbiVersion() const override { return PLUGIN_ABI_VERSION; }
    LPCWSTR GetPluginId()   const override { return L"acme";            }
    LPCWSTR GetPluginName() const override { return L"Acme 전용 기능";  }
};

extern "C" __declspec(dllexport) int SagePlugin_GetAbiVersion()
{
    return PLUGIN_ABI_VERSION;
}

extern "C" __declspec(dllexport) IPlugin* SagePlugin_Create()
{
    try { return new AcmePlugin(); }
    catch (...) { return nullptr; }
}

extern "C" __declspec(dllexport) void SagePlugin_Destroy(IPlugin* pPlugin)
{
    try { delete pPlugin; }
    catch (...) {}
}
```

---

## 배포 경로

```
<AppDir>\
  SageNexus.exe
  plugins\
    SageNexus-Plugin-Acme.dll
    SageNexus-Plugin-Beta.dll
```

앱 시작 시 `PluginManager::LoadPluginsFromDirectory()`가 `plugins\*.dll`을 스캔하여 자동 로드한다.

---

## 로드 실패 정책

- DLL 로드 실패 (`LoadLibraryExW` 오류, ABI 버전 불일치, export 함수 없음, `SagePlugin_Create()` NULL 반환)는 해당 플러그인만 비활성화한다.
- 앱은 계속 실행된다.
- 실패 원인은 로그에 기록되며 `strErrors`로 회수할 수 있다.

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
| C 스타일 opaque handle | void* + 평면 함수 | MFC/C++ 환경에서 오히려 복잡도 증가, 채택 안 함 |
| **C++ virtual interface + DLL (채택)** | IPlugin* 직접 전달 | 빌드 설정 일치 조건 아래 충분히 안전, 코드 단순 |

---

## 영향 범위

- `IPlugin.h`: `const CString&` → `LPCWSTR` 로 변경 (CString을 ABI 경계 밖으로 제거)
- `PluginLoader.cpp`: `LoadLibraryW` → `LoadLibraryExW` + 보안 플래그 적용
- `PluginManager.cpp`: `LPCWSTR` → `CString` 복사 (암묵 변환으로 기존 코드 유지)
