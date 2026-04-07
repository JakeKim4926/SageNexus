# ADR-001: 설치 폴더 / 사용자 데이터 폴더 분리 정책

- **날짜**: 2026-04-07
- **상태**: 확정
- **관련 Phase**: Phase 6 Step 5

---

## 결정 요약

SageNexus는 설치 폴더(exe 위치)와 사용자 데이터 폴더(`%APPDATA%\SageNexus\`)를 분리한다.

---

## 배경

초기 구현에서는 설정 파일, 로그, 데이터, WebViewData를 모두 exe 위치 기준으로 저장했다.
이는 개발 환경에서는 문제가 없었지만, 실제 배포 시 다음 문제가 발생한다.

- 설치 폴더(`Program Files` 등)는 일반 사용자에게 쓰기 권한이 없는 경우가 많다.
- 사용자 간 데이터 분리가 불가능하다.
- 로그/데이터 파일이 설치 파일과 섞여 유지보수가 어렵다.

---

## 폴더 구조 정책

### 설치 폴더 (AppDir, exe 위치)

읽기 전용 리소스를 포함한다. 설치 시 기업 IT 부서 또는 패키징 도구가 배포한다.

```
<AppDir>\
  SageNexus.exe
  WebView2Loader.dll
  webui\                  (WebView2 HTML/JS/CSS 자산)
  templates\              (기본 Workflow 템플릿 JSON)
  profile.json            (회사 지급 SolutionProfile, 읽기 전용 기준)
```

### 사용자 데이터 폴더 (UserDataDir, %APPDATA%\SageNexus\)

실행 시 자동 생성된다. 사용자별로 분리된다.

```
%APPDATA%\SageNexus\
  settings.json           (인터페이스 언어, 윈도우 크기 등 사용자 설정)
  Data\
    execution_history.json
    workflows.json
    artifacts.json
  Logs\
    SageNexus_YYYYMMDD_HHMMSS.log
  WebViewData\            (WebView2 사용자 데이터)
```

---

## 구현 기준

### SageApp 경로 API

| 메서드 | 반환 | 용도 |
|---|---|---|
| `GetAppDir()` | exe 위치 | webui, templates, profile.json 로드 |
| `GetUserDataDir()` | `%APPDATA%\SageNexus\` | settings.json, WebViewData 경로 기반 |
| `GetDataDir()` | `UserDataDir\Data` | execution_history, workflows, artifacts |
| `GetLogDir()` | `UserDataDir\Logs` | 세션 로그 파일 |

### 경로 결정 규칙

- **읽기 전용 리소스** → `GetAppDir()`
- **쓰기 가능한 사용자 데이터** → `GetUserDataDir()` 또는 하위 경로
- Infrastructure 계층의 Store 클래스는 `GetDataDir()`을 직접 사용하고, `GetAppDir() + DATA_DIR_NAME` 형태를 사용하지 않는다.

---

## profile.json 취급

`profile.json`은 회사별로 배포되는 `SolutionProfile` 설정 파일이다.

- 설치 폴더에 위치한다 (`GetAppDir()\profile.json`).
- 기업 IT 부서 또는 패키징 도구가 회사별 설정으로 교체하여 배포한다.
- 앱 실행 시 설치 폴더에서 로드한다.
- 파일이 없으면 기본값으로 생성한다.
- Settings 페이지에서 플러그인 on/off를 변경하면 설치 폴더의 `profile.json`에 저장된다.

> 참고: 향후 단일 머신에 여러 사용자가 사용하는 환경이 필요하면
> `profile.json`도 `UserDataDir`로 이동하는 방안을 별도 ADR로 결정한다.

---

## 대안 검토

| 안 | 내용 | 기각 이유 |
|---|---|---|
| 모두 AppDir | 현재 구조 유지 | 쓰기 권한 문제, 사용자 분리 불가 |
| 모두 AppData | 설치 리소스까지 AppData 복사 | 설치 복잡도 증가, 버전 관리 어려움 |
| **분리 (채택)** | AppDir = 읽기 전용, AppData = 쓰기 가능 | 표준 Windows 배포 패턴, 권한 문제 없음 |

---

## 영향 범위

- `SageApp::InitializePaths()`: `SHGetFolderPathW(CSIDL_APPDATA)` 사용으로 변경
- `MainWindow::OnCreate()`: WebViewData 경로를 `GetUserDataDir()` 기준으로 변경
- `ArtifactStore`, `ExecutionHistoryStore`, `WorkflowStore`: `GetDataDir()` 직접 사용
- `JsonConfigStore`: `GetUserDataDir()` 기준으로 생성
