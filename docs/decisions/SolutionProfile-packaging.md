# SolutionProfile 패키징 단위 문서

- **날짜**: 2026-04-07
- **상태**: 확정
- **관련 Phase**: Phase 6 Step 5

---

## 개요

SageNexus는 단일 바이너리를 여러 회사에 배포하되,
각 회사에 필요한 기능만 활성화하는 구조를 목표로 한다.

`SolutionProfile`은 이 차이를 수용하는 배포 단위 설정 파일이다.

---

## SolutionProfile이란

`SolutionProfile`은 다음을 정의한다.

- 활성화할 플러그인 목록
- 표시할 메뉴/페이지 항목
- 기본 Interface Language
- 기본 Output Language
- 프로필 이름과 ID

파일 형식: `profile.json` (UTF-8 JSON)

---

## 배포 패키징 단위

### 구성 요소

SageNexus 고객사 배포 패키지는 다음 두 요소로 구성된다.

```
패키지/
  SageNexus.exe            (공통 바이너리)
  WebView2Loader.dll       (공통)
  webui/                   (공통 WebView2 자산)
  templates/               (공통 기본 Workflow 템플릿)
  profile.json             [회사별 교체]
```

`profile.json` 하나만 교체하면 해당 회사에 맞는 기능 세트로 앱이 동작한다.

### 회사별 profile.json 예시

**예시 A: 웹 수집 + 엑셀 저장**
```json
{
  "profileId": "company-a",
  "profileName": "Company A",
  "defaultInterfaceLanguage": "ko",
  "defaultOutputLanguage": "ko",
  "showDataViewer": true,
  "showTransform": false,
  "showExport": true,
  "showHistory": true,
  "showSettings": true,
  "plugin_import": true,
  "plugin_transform": false,
  "plugin_export": true,
  "plugin_history": true,
  "plugin_workflow": true,
  "plugin_webextract": true
}
```

**예시 B: 재무 데이터 정리 + Word 보고서**
```json
{
  "profileId": "company-b",
  "profileName": "Company B",
  "defaultInterfaceLanguage": "en",
  "defaultOutputLanguage": "en",
  "showDataViewer": true,
  "showTransform": true,
  "showExport": true,
  "showHistory": true,
  "showSettings": false,
  "plugin_import": true,
  "plugin_transform": true,
  "plugin_export": true,
  "plugin_history": true,
  "plugin_workflow": false,
  "plugin_webextract": false
}
```

---

## 필드 정의

| 필드 | 타입 | 설명 |
|---|---|---|
| `profileId` | string | 프로필 고유 식별자 |
| `profileName` | string | 표시 이름 (Settings 페이지에 표시됨) |
| `defaultInterfaceLanguage` | `"ko"` or `"en"` | 앱 시작 시 기본 UI 언어 |
| `defaultOutputLanguage` | `"ko"` or `"en"` | 기본 Output Language (결과물 헤더 등) |
| `showDataViewer` | bool | Data Viewer 메뉴 표시 여부 |
| `showTransform` | bool | Transform 메뉴 표시 여부 |
| `showExport` | bool | Export 메뉴 표시 여부 |
| `showHistory` | bool | History 메뉴 표시 여부 |
| `showSettings` | bool | Settings 메뉴 표시 여부 |
| `plugin_<id>` | bool | 해당 플러그인 활성화 여부 |

---

## 패키징 절차 (개발자 기준)

1. 기본 빌드를 수행한다 (`Debug_x64` 또는 Release 빌드).
2. 배포 폴더를 구성한다:
   ```
   SageNexus.exe
   WebView2Loader.dll
   webui/          (소스의 webui/public/ 내용 복사)
   templates/      (소스의 resources/templates/ 내용 복사)
   profile.json    (고객사 설정 파일)
   ```
3. `profile.json`을 고객사 요구에 맞게 작성한다.
4. 배포 폴더를 압축하거나 인스톨러로 패키징한다.

> 현재 Phase에서 인스톨러 자동화는 구현하지 않는다. 수동 패키징 절차를 따른다.

---

## templates/ 폴더 구조 (향후)

현재 기본 Workflow 템플릿은 코드 내 인메모리로 번들되어 있다.
향후 외부 파일로 분리할 때 아래 구조를 사용한다.

```
templates/
  csv-to-xlsx.json        (CSV 정리 → XLSX 저장)
  webextract-to-html.json (웹 추출 → HTML 보고서)
```

이 파일들은 `GetAppDir()\templates\` 에서 로드한다.

---

## 제약 사항

- `profile.json`은 현재 단일 파일 기준이다. 계층 상속(기본 프로필 + 회사 프로필)은 구현하지 않는다.
- 플러그인 비활성화는 UI 숨김 기준이다. 런타임 완전 제거는 이 단계에서 구현하지 않는다.
- 사용자가 Settings에서 플러그인 토글 시 설치 폴더의 `profile.json` 저장을 시도한다.
- 설치 폴더에 쓰기 권한이 없으면 저장은 실패하고 메모리 상태는 디스크 기준으로 복구된다.
- 배포 후 원본 `profile.json` 보호 정책이 필요하면 별도 ADR로 다룬다.
