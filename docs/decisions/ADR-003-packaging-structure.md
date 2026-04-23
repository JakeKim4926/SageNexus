# ADR-003: 고객사별 패키징 구조

- **날짜**: 2026-04-23
- **상태**: 확정
- **관련 Phase**: Phase 7 Step 2

---

## 결정 요약

SageNexus는 `profile.json` 기반 빌드 구성과 패키징 스크립트를 통해
고객사별 독립 배포 폴더를 생성한다.

---

## 빌드 구성 체계

### VS 빌드 구성 명명 규칙

| 구성 | 대상 |
|---|---|
| `Debug\|x64` / `Release\|x64` | 기본 (default profile) |
| `Debug-Acme\|x64` / `Release-Acme\|x64` | Acme 고객사 |
| `Debug-Beta\|x64` / `Release-Beta\|x64` | Beta 고객사 |
| `Debug-Taechang\|x64` / `Release-Taechang\|x64` | 태창 고객사 |

### 빌드 전 처리 (PreBuildEvent)

각 고객사 구성은 빌드 전에 해당 profile을 자동 선택한다.

```
copy /Y "$(ProjectDir)profiles\$(CompanyProfile).json" "$(ProjectDir)profile.json"
```

`CompanyProfile` 속성은 각 빌드 구성에 `acme` / `beta` / `taechang` 등으로 정의된다.

### 빌드 후 처리 (PostBuildEvent)

고객사 구성은 빌드 후 `plugins\` 폴더를 자동 생성한다.

```
if not exist "$(OutDir)plugins\" mkdir "$(OutDir)plugins\"
```

---

## Profile 파일 구조

Profile은 `SageNexus/profiles/<profileId>.json`에 위치한다.

```
SageNexus/profiles/
  default.json
  acme.json
  beta.json
  taechang.json
```

Profile이 제어하는 항목:
- 메뉴/페이지 표시 여부 (`showDataViewer`, `showWorkflow` 등)
- 플러그인 활성 여부 (`plugin_import`, `plugin_transform` 등)
- 기본 Interface Language / Output Language

---

## 배포 폴더 구조

패키징 스크립트(`scripts/package.ps1`) 실행 결과:

```
deploy/<profile>/
  SageNexus.exe
  WebView2Loader.dll
  profile.json          ← 해당 고객사 profile
  webui\                ← WebView2 프론트엔드
  resources\            ← 아이콘 등 리소스
  plugins\              ← 고객사 전용 DLL 드랍 위치
    SageNexus-Plugin-Taechang.dll  (고객사 DLL 배포 시)
```

---

## 패키징 스크립트 사용법

```powershell
# 태창 고객사 Release 패키징
.\scripts\package.ps1 -Profile taechang -BuildType Release

# 기본 구성 Debug 패키징
.\scripts\package.ps1 -Profile default -BuildType Debug
```

스크립트는 다음을 수행한다.
1. 해당 빌드 출력 폴더 존재 확인
2. `deploy/<profile>/` 폴더 생성 (기존 폴더 덮어씀)
3. 필수 파일 복사 (exe, dll, profile.json, webui, resources)
4. `plugins/` 폴더 생성 및 DLL 복사

---

## 설치 폴더 / 사용자 데이터 폴더 최종 검증 (2026-04-23)

### 설치 폴더 (AppDir) 검증

| ADR-001 항목 | 실제 빌드 출력 | 결과 |
|---|---|---|
| `SageNexus.exe` | ✅ 있음 | 일치 |
| `WebView2Loader.dll` | ✅ 있음 | 일치 |
| `webui\` | ✅ 있음 | 일치 |
| `profile.json` | ✅ 있음 | 일치 |
| `templates\` | ❌ 없음 | **불일치** |

`templates\`는 ADR-001에 명시됐으나 현재 미구현 상태다.
Workflow 템플릿 번들 기능이 실제로 사용될 때 추가한다. 현재 배포에서는 해당 폴더 없이 동작한다.

### 사용자 데이터 폴더 (UserDataDir) 검증

| ADR-001 항목 | 코드 구현 | 결과 |
|---|---|---|
| `%APPDATA%\SageNexus\` | `SHGetFolderPathW(CSIDL_APPDATA)` + `USER_DATA_APP_NAME` | ✅ 일치 |
| `settings.json` | `JsonConfigStore` → `GetUserDataDir()` | ✅ 일치 |
| `Data\` | `m_strDataDir` = `UserDataDir + DATA_DIR_NAME` | ✅ 일치 |
| `Logs\` | `m_strLogDir` = `UserDataDir + LOG_DIR_NAME` | ✅ 일치 |
| `WebViewData\` | `GetUserDataDir() + WEBVIEW_USER_DATA_FOLDER` | ✅ 일치 |

사용자 데이터 폴더 구조는 ADR-001 정책과 완전히 일치한다.

---

## 고객사 DLL 배포 정책

### 배포 방식

1. 고객사 private 레포에서 플러그인 DLL 빌드 (`Release-<Company>|x64`)
2. 빌드 출력 DLL → `<BuildOutput>/plugins/` 에 복사
3. `scripts/package.ps1` 실행 → `deploy/<profile>/plugins/` 에 자동 포함
4. `deploy/<profile>/` 전체를 고객사 머신에 설치

### DLL 파일명 규칙

```
SageNexus-Plugin-<CompanyName>.dll
```

예: `SageNexus-Plugin-Taechang.dll`

### DLL 서명 정책

현재 단계에서는 코드 서명(Code Signing)을 적용하지 않는다.
운영 환경 배포 시 아래 조건이 충족되면 서명을 도입한다.

- 고객사 머신에 SmartScreen 또는 UAC 경고가 문제가 되는 경우
- 배포 DLL의 무결성 검증이 계약상 요구되는 경우

도입 시 MSVC Code Signing (Authenticode)을 사용한다.

### 업데이트 방식

DLL 단독 교체:
1. 신규 버전 DLL을 고객사 `plugins\` 폴더에 덮어씀
2. 앱 재시작 시 신규 DLL이 자동 로드됨
3. ABI 버전이 다르면 로드 실패 → 해당 플러그인만 비활성화, 앱은 계속 실행

전체 패키지 교체:
- `scripts/package.ps1` 재실행 → `deploy/<profile>/` 폴더 재생성 → 배포

---

## 신규 고객사 추가 절차

1. `SageNexus/profiles/<company>.json` 작성
2. `SageNexus.vcxproj`에 `Debug-<Company>|x64` / `Release-<Company>|x64` 구성 추가
3. `scripts/package.ps1`의 `ValidateSet`에 `<company>` 추가
4. 고객사 private 레포 생성 → DLL 빌드 → `plugins/`에 드랍

---

## 영향 범위

- `SageNexus.vcxproj`: Debug-Taechang / Release-Taechang 구성 추가
- `SageNexus/profiles/taechang.json`: 태창 profile 추가
- `scripts/package.ps1`: 패키징 스크립트 신규 작성
- `SageNexus.slnx`: Taechang 플랫폼 구성 추가
