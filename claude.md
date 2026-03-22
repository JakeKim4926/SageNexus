# Claude Working Guide for SageNexus

이 문서는 SageNexus 프로젝트에서 Claude / AI 코딩 도구가 작업할 때 따라야 할 루트 운영 가이드다.  
코드 작성, 리팩토링, 문서화, 브랜치 운영, WebView2 UI 작업, 브릿지 연동, 개발 Phase 진행 시 아래 기준을 우선 적용한다.

---

## 프로젝트 방향

SageNexus는 C++ / Win32 / WebView2 기반의 업무 자동화 플랫폼이다.  
다음 구조를 전제로 설계한다.

- Core Platform + Functional Modules + Solution Profiles
- Layered Architecture + Plugin Architecture
- WebView2 UI는 state-driven 방식
- 내부 데이터는 `DataTable` 중심으로 통일
- 회사별로 필요한 기능만 선택적으로 조합 가능해야 함
- UI 언어와 결과물 언어를 분리하는 i18n 구조를 가짐

이 프로젝트는 모든 기능을 한 번에 넣는 범용 툴이 아니라,  
공통 코어 위에 필요한 기능만 조합하는 **회사별 맞춤형 자동화 플랫폼**이다.

---

## 우선 참조해야 할 Skill

SageNexus 작업에서는 아래 6개 skill을 역할에 따라 참조한다.

### 1. `coding-rules`
역할:
- C++ / Win32 / MFC / WebView2 기반 코드 작성 규칙
- 네이밍, 반환 규약, 예외 기준, 스레드 책임, 상수/enum 규칙
- `auto` 금지, 스마트 포인터 금지, 포인터 반환 금지 등 프로젝트 전반의 코딩 스타일 기준

먼저 참조해야 하는 경우:
- C++ 코드 작성
- 서비스 / 엔진 / 뷰 / 플러그인 계층 수정
- 클래스 추가, 함수 설계, 예외 처리, 반환 타입 결정

---

### 2. `git-workflow`
역할:
- Git 브랜치 전략
- 커밋 메시지 규칙
- PR 작성 규칙
- 머지 방향과 PR 로그 운영 기준

먼저 참조해야 하는 경우:
- 작업 브랜치 생성
- 커밋 정리
- PR 생성 / 머지
- Phase 완료 후 기록 정리

---

### 3. `SageNexus-plan`
역할:
- SageNexus 전체 개발 계획
- Phase 단위 목표, 완료 기준, 이월 항목
- 코어 / 플러그인 / 프로필 구조 기반 개발 순서

먼저 참조해야 하는 경우:
- 지금 어떤 단계의 작업인지 판단할 때
- 신규 기능이 현재 Phase 범위에 맞는지 판단할 때
- MVP 범위와 후순위 기능을 구분할 때
- 다음 단계 작업 우선순위를 정할 때

---

### 4. `webview2-ui`
역할:
- WebView2 내부 프론트엔드 구현 규칙
- 상태 관리, 폴더 구조, pages/components/features/core/store 분리 기준
- bridgeClient 사용 규칙
- loading / empty / error 상태 처리
- 프로필 기반 메뉴/라우팅 제어

먼저 참조해야 하는 경우:
- WebView2 페이지 추가/수정
- 상태 관리 구조 설계
- 공통 컴포넌트 분리
- 화면 로직과 브릿지 호출 구조 정리

---

### 5. `webview2-bridge`
역할:
- Native(C++/Win32/MFC) ↔ WebView2 UI 간 JSON 브릿지 규약
- command / response / event 구조
- requestId, error, progress, timeout, cancel 정책
- Dispatcher / Router / Handler 구조 기준

먼저 참조해야 하는 경우:
- 브릿지 메시지 추가
- target/action 설계
- progress event 설계
- 네이티브와 UI 간 응답 형식 조정

---

### 6. `sagenexus-ui`
역할:
- SageNexus WebView2 콘텐츠 영역의 디자인 시스템 및 제품 UI 기준
- 디자인 톤, 색상 토큰, 페이지 구성, 컴포넌트 패턴
- i18n 구조
- Interface Language / Output Language 분리
- Dashboard / Data Viewer / Transform / Export / History / Settings 기준

먼저 참조해야 하는 경우:
- 화면 목업을 실제 UI로 옮길 때
- 페이지별 레이아웃과 섹션 구성을 잡을 때
- 다국어 UI를 반영할 때
- 디자인 일관성을 맞출 때

---

## Skill 간 역할 구분

각 skill은 일부 겹치는 내용이 있지만 역할은 아래처럼 구분한다.

### `sagenexus-ui`
- 무엇을 보여줘야 하는가
- 어떤 톤과 구조를 가져야 하는가
- UI 언어 / 출력 언어를 어떻게 분리할 것인가
- 페이지별로 어떤 섹션과 상태가 필요한가

### `webview2-ui`
- WebView2 UI를 어떻게 구현할 것인가
- 상태 관리와 컴포넌트 구조를 어떻게 나눌 것인가
- bridgeClient를 어디서 호출할 것인가
- DOM 직접 조작을 어떻게 피할 것인가

### `webview2-bridge`
- 네이티브와 UI가 어떤 JSON 계약으로 통신할 것인가
- command / response / event를 어떻게 나눌 것인가
- requestId, error, progress를 어떻게 다룰 것인가

즉:
- `sagenexus-ui` = 제품 UI 기준서
- `webview2-ui` = 프론트엔드 구현 규칙서
- `webview2-bridge` = 네이티브-UI 통신 규약서

---

## 작업 유형별 참조 우선순위

### C++ 코드 작업
1. `coding-rules`
2. `SageNexus-plan`
3. 필요 시 `webview2-bridge`

### Git / 브랜치 / PR 작업
1. `git-workflow`

### 새로운 기능 추가
1. `SageNexus-plan`
2. `coding-rules`
3. 필요 시 `webview2-ui` / `webview2-bridge` / `sagenexus-ui`

### WebView2 페이지 작업
1. `sagenexus-ui`
2. `webview2-ui`
3. 필요 시 `webview2-bridge`

### 네이티브 ↔ WebView2 메시지 작업
1. `webview2-bridge`
2. `webview2-ui`
3. `coding-rules`

### 화면 목업을 실제 UI로 옮기는 작업
1. `sagenexus-ui`
2. `webview2-ui`
3. 필요 시 `webview2-bridge`

---

## 프로젝트 구조 해석 기준

SageNexus의 구조는 다음 계층을 따른다.

### Host / Presentation
예:
- `MainWindow`
- `WebViewHost`
- `NavigationController`
- `BridgeDispatcher`

역할:
- 창, 메시지, WebView2, UI 연결

직접 해서는 안 되는 것:
- 파일 파싱
- 데이터 변환
- 문서 export
- 장시간 동기 작업

---

### Application
예:
- `ImportService`
- `TransformService`
- `ExportService`
- `ExecutionService`

역할:
- 유스케이스 조합
- 실행 흐름 관리
- validation 및 orchestration

---

### Domain
예:
- `DataTable`
- `TransformPlan`
- `ExecutionResult`
- `WorkflowDefinition`
- `SolutionProfile`

역할:
- 핵심 데이터 모델
- 업무 규칙
- 포맷/화면에 덜 의존하는 순수 구조

---

### Infrastructure
예:
- `CsvReader`
- `XlsxReader`
- `HtmlReportExporter`
- `JsonConfigStore`
- `FileLogger`
- `PluginManager`

역할:
- 실제 입출력 구현
- 외부 라이브러리 연동
- 파일/설정/로그/시스템 접근

---

## 기능 추가 원칙

기능을 추가할 때는 먼저 아래를 판단한다.

1. 공통 코어 기능인가
2. 선택 기능 모듈인가
3. 특정 고객사 프로필에만 필요한가

모든 기능을 코어에 넣지 않는다.  
회사별 기능 조합 가능성을 항상 우선 고려한다.

예:
- 웹 수집 + 엑셀 저장만 필요한 회사
- 재무제표 추출 + 워드 보고서만 필요한 회사
- 단순 데이터 정리만 필요한 회사

이 차이를 수용할 수 있도록 `Plugin` / `SolutionProfile` 구조를 유지한다.

---

## 데이터 모델 원칙

- 내부 표준 데이터 모델은 `DataTable` 중심으로 유지한다
- CSV, XLSX, 웹 추출 결과는 가능한 한 동일한 중간 표현으로 수렴시킨다
- 컬럼은 원본명과 표시명을 분리한다

권장 개념:
- `internalName`
- `sourceName`
- `displayNameKo`
- `displayNameEn`

이 구조는 Transform의 `Source / Display` 토글과 Output Language 정책에 직접 연결된다.

---

## WebView2 UI 작업 원칙

WebView2는 단순 HTML 뷰가 아니라 제품의 실질적인 프론트엔드다.  
따라서 아래 기준을 항상 확인한다.

- state-driven 구조를 유지하는가
- `bridgeClient`를 통해 네이티브 요청을 보내는가
- loading / empty / error 상태가 존재하는가
- 프로필/플러그인 비활성 상태를 고려하는가
- 공통 토큰과 공통 컴포넌트를 재사용하는가
- `Interface Language`와 `Output Language`를 분리하는가

기본 페이지 세트:
- Dashboard
- Data Viewer
- Transform
- Export
- History
- Settings

---

## i18n 원칙

SageNexus는 UI 언어와 결과물 언어를 분리한다.

### Interface Language
적용 대상:
- 메뉴
- 버튼
- 라벨
- 상태 메시지
- 페이지 타이틀
- Empty / Loading / Error 문구

### Output Language
적용 대상:
- 컬럼 표시명
- 문서 템플릿 제목
- 결과물 헤더

### 번역하지 않는 것
- 실제 파일명
- Run ID
- 오류 코드
- 원본 컬럼명
- 고객사 원본 데이터 값

기본 파일:
- `ko.json`
- `en.json`

---

## 브릿지 작업 원칙

브릿지 작업 시 아래를 항상 유지한다.

- 모든 통신은 JSON 사용
- 메시지 유형은 `command`, `response`, `event`
- 모든 command는 `requestId` 포함
- response는 `success`, `payload`, `error` 구조 유지
- long-running 작업은 progress event 필요성을 검토
- UI는 네이티브 내부 구현을 몰라야 한다
- Native는 DOM 구조를 몰라야 한다

금지:
- 페이지에서 직접 `window.chrome.webview.postMessage` 호출
- requestId 없는 command
- progress만 보내고 final response 없이 종료
- error를 payload에 뒤섞는 방식

---

## 코드 작업 원칙

코드 작성 시 아래를 항상 확인한다.

- `auto` 사용 금지
- 스마트 포인터 사용 금지
- 포인터 반환 금지
- UI 스레드에서 무거운 작업 금지
- 워커 스레드에서 UI 직접 접근 금지
- 클래스명 앞 `C` 금지
- 멤버 변수는 `m_` + 헝가리안 표기법 사용
- 실패 가능한 함수는 `BOOL` 반환 우선
- 상수는 `Define.h`, enum은 `EnumDefine.h` 기준 관리

---

## Git 작업 원칙

Git 작업 시 아래 원칙을 따른다.

- `main` / `develop` 직접 커밋 금지
- 작업 브랜치는 `feature/*`, `fix/*`, `refactor/*`, `docs/*` 사용
- PR 기본 목적지는 `develop`
- PR 생성 시 `--base develop` 명시
- 기본 머지 방식은 squash merge
- 머지 후 브랜치 삭제
- `docs/decisions/PR_LOG.md` 기록 유지

---

## 작업 요청 처리 순서

작업 요청을 받으면 아래 순서로 판단한다.

1. 요청이 어느 계층에 속하는지 판단한다
2. 코어 / 플러그인 / 프로필 중 어디에 속하는지 판단한다
3. 현재 개발 단계가 `SageNexus-plan`의 어느 Phase인지 확인한다
4. 반환 타입, 예외, 스레드 책임이 `coding-rules`에 맞는지 검토한다
5. WebView2 UI라면 `sagenexus-ui`와 `webview2-ui` 기준을 적용한다
6. 브릿지 연동이 있다면 `webview2-bridge`를 적용한다
7. 변경 후 Git 브랜치/커밋/PR 흐름까지 고려한다

---

## 문서 작업 원칙

문서 작성 시 다음을 유지한다.

- 추상적인 표현보다 구조와 책임을 명확히 쓴다
- 완료 기준, 범위, 제외 범위를 구분한다
- Phase 문서와 ADR, PR_LOG와 연결될 수 있게 작성한다
- 기존 프로젝트 용어를 일관되게 사용한다
- 제품 UI 정책과 구현 규칙을 혼동하지 않는다

---

## 최종 판단 기준

문서에 없는 상황에서는 아래 기준을 따른다.

1. 구조가 단순해지는가
2. 책임이 분리되는가
3. UI 응답성을 해치지 않는가
4. 회사별 기능 조합 가능성을 해치지 않는가
5. UI 언어와 결과물 언어를 명확히 분리하는가
6. 나중에 확장해도 코어가 오염되지 않는가

이 기준을 만족하는 방향으로 코드를 작성하고 수정한다.
