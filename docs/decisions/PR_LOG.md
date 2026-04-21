# PR 작업 로그

---

## [2026-04-21] fix/webview2-no-cache-headers
- **목적**: WebView2 임베딩 리소스 캐시 방지 및 CRLF 파싱 버그로 인한 메뉴/카드 전체 숨김 수정
- **변경 내용**: `WebViewHost.cpp` WebResource 응답에 `Cache-Control: no-store` 헤더 추가. `SolutionProfile.cpp`의 `ParseProfileJson`에서 CRLF 파일의 `\r`을 제거하지 않아 `"true\r" == "true"` 비교가 false로 평가되던 버그 수정 → 모든 `show*` 메뉴가 FALSE 파싱되어 사이드바·대시보드 카드 전체 숨김되던 문제 해결
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/62
- **결과**: pending

---

## [2026-04-21] feature/dashboard-redesign
- **목적**: 대시보드를 stat 4종 카드 구조에서 히어로 영역(프로필명/타이틀/마지막 실행) + 메뉴 네비게이션 카드 구조로 재편
- **변경 내용**: `dashboard-hero` 추가(프로필명 22px, 타이틀 보조, 마지막 실행 한 줄), `dashboard-cards` 그리드에 7개 카드(데이터 뷰어/변환/내보내기/워크플로우/웹 추출/실행 이력/설정) 추가. `applyMenuVisibility`에서 nav-item과 대시보드 카드를 동일 기준으로 토글. `loadDashboardStats`를 마지막 실행 시각만 갱신하도록 단순화. i18n에서 `dashboard.stat.totalRuns/todayRuns/successRate/empty.*` 제거, `dashboard.lastRun.never` 추가
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/60
- **결과**: merged ✅

---

## [2026-04-21] fix/webview2-filter-source-kinds
- **목적**: 가상호스트 `https://app.sagenexus/` 로드 후 `초기화 중...` 오버레이에서 무한 대기하던 회귀 수정
- **변경 내용**: `AddWebResourceRequestedFilter`를 `ICoreWebView2_22::AddWebResourceRequestedFilterWithRequestSourceKinds`로 마이그레이션(최신 WebView2 런타임에서 script/stylesheet 서브리소스 커버). `PostMessageToWeb`를 `PostWebMessageAsString` → `ExecuteScript` + `CustomEvent` dispatch로 전환(가상호스트 하 JS message 이벤트 전달 불안정 회피). `bridge.js`에 `_dispatch` 헬퍼 및 `window.__bridgeReceive` 진입점 추가, 기존 postMessage 경로도 유지
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/59
- **결과**: merged ✅

---

## [2026-04-20] feature/login-dialog-polish
- **목적**: 최초 비밀번호 설정 화면에서 사용자가 설정 목적을 직관적으로 인지할 수 있도록 안내 문구 추가, 비밀번호 입력/설정 다이얼로그의 여백·버튼 크기 정리
- **변경 내용**: SageNexus.rc의 IDD_SET_PASSWORD에 보조 설명 2줄(사용 목적 / 분실 시 복구 불가) 추가, 다이얼로그 크기 220x110 → 280x180 확장, 라벨 폭/입력창 정리. IDD_LOGIN도 동일 톤으로 여백·버튼 크기 정리. 코드 로직 변경 없음
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/57
- **결과**: merged ✅

---

## [2026-04-18] feature/multi-company-profile-embedding
- **목적**: 회사별 프로필 JSON을 빌드 시점에 exe 리소스로 임베딩하고, Configuration으로 선택하여 배포 exe를 회사별로 분리
- **변경 내용**: profiles/{acme,beta,default}.json 추가, SageNexus.rc RCDATA로 profile.json 임베딩, SolutionProfile LoadFromResource 추가, 파일 기반 프로필 서명/저장 로직 제거, PreBuild 이벤트로 Configuration별 프로필 선택
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/56
- **결과**: merged ✅

---

## [2026-04-18] feature/deploy-profile-identity
- **목적**: 프로필 ID/이름을 개발자가 빌드 전에 설정하고 사용자는 변경 불가하도록 컴파일 타임 상수로 고정
- **변경 내용**: Define.h에 DEPLOY_PROFILE_ID/NAME 상수 추가, SolutionProfile/SageApp 하드코딩 제거
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/55
- **결과**: merged ✅

---

## [2026-04-18] refactor/embed-resources-into-exe
- **목적**: 배포 파일을 exe + WebView2Loader.dll만으로 최소화. webui, profile, sig 파일이 사용자에게 노출되지 않도록 구조 변경
- **변경 내용**: webui HTML/CSS/JS/ICO를 exe RCDATA로 임베딩, WebResourceRequested 가상 호스트 서빙 추가, profile/sig 경로를 AppData로 이동, PostBuildEvent 전면 제거
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/54
- **결과**: merged ✅

---

## [2026-04-18] feature/cleanup-build-output
- **목적**: 배포 산출물에서 런타임 불필요 파일(i18n 원본, 소스 리소스) 제거 및 Debug_x64 레거시 잔재 정리
- **변경 내용**: vcxproj PostBuildEvent를 런타임 필수 파일(webui/public, webui/src/core, webui/src/styles, resources/app.ico)만 복사하도록 변경; 레거시 폴더 rmdir 안전망 추가
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/53
- **결과**: merged ✅

---

## [2026-04-18] feature/settings-security-i18n-keys
- **목적**: 설정 화면 보안 섹션의 i18n 키 누락으로 사용자에게 키 문자열(`settings.section.security`)이 그대로 노출되는 문제 수정
- **변경 내용**: `index.html`의 `LOCALES`(ko/en) 및 `src/i18n/*.json`에 `settings.section.security`, `settings.security.changePassword`, `settings.security.ph.*`, `settings.security.btn.change`, `settings.security.err.*`, `settings.security.success` 키 추가
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/52
- **결과**: merged ✅

---

## [2026-04-18] feature/profile-security
- **목적**: B2B 배포 시 profile.json 변조 방지 및 회사별 접근 제어
- **변경 내용**: PBKDF2-SHA256 패스워드 인증(ProfileSecurity), Win32 로그인 다이얼로그(LoginDialog), HMAC-SHA256 프로필 서명 검증, 시작 시 로그인 흐름 추가, Settings 보안 섹션(비밀번호 변경) 추가, settings.security/changePassword 브릿지 핸들러
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/51
- **결과**: merged ✅

---

## [2026-04-18] feature/profile-menu-visibility
- **목적**: profile.json 설정으로 회사별 사이드바 메뉴를 선택적으로 제어
- **변경 내용**: MenuVisibility에 showWorkflow/showWebextract 추가, appReady 이벤트에 menuVisibility 포함, UI에서 applyMenuVisibility()로 nav-item 표시/숨김 처리
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/50
- **결과**: merged ✅

---

## [2026-04-18] feature/app-icon
- **목적**: 앱 아이콘 등록 및 타이틀바 좌측 표시, 초기 로딩 지연 해소
- **변경 내용**: resource.h / SageNexus.rc로 app.ico를 Win32 리소스 등록, MainWindow LoadIcon 리소스 기반으로 변경, topbar 좌측 32×32 아이콘 표시, topbar-title wrapper 도입으로 baseline 정렬 개선, CDN 폰트 non-blocking 로드로 변경하여 초기 로딩 23초 지연 해소, PostBuildEvent에 resources 폴더 복사 추가
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/49
- **결과**: merged ✅

---

## [2026-04-17] feature/custom-titlebar
- **목적**: WebView2 커스텀 토바가 있음에도 OS 기본 타이틀바가 함께 표시되는 문제 해결
- **변경 내용**: WS_POPUP | WS_THICKFRAME으로 창 스타일 변경하여 타이틀바 완전 제거, NonClientRegionSupportEnabled 활성화로 -webkit-app-region:drag 동작, 토바에 최소화/최대화/닫기 버튼 추가, DWM 그림자 복원
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/48
- **결과**: merged ✅

---

## [2026-04-16] feature/dv-resizable-table
- **목적**: Data Viewer 테이블 출력 영역 높이를 사용자가 드래그로 자유롭게 조절할 수 있도록 개선
- **변경 내용**: 테이블 하단 드래그 핸들 추가, 높이 localStorage 저장/복원, 창 리사이즈 시 자동 재조정
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/47
- **결과**: merged ✅

---

## [2026-04-15] feature/dv-column-alignment
- **목적**: Data Viewer 툴바 우측에 전역 정렬 버튼(좌/가운데/우) 추가
- **변경 내용**: 툴바를 toolbar-left/right 구조로 분리, 워드 스타일 SVG 아이콘 정렬 버튼 그룹 추가, 데이터 로드 시 버튼 표시, applyTableAlign()으로 전체 셀 정렬 일괄 변경, 새 파일 로드 시 초기화
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/46
- **결과**: merged ✅

---

## [2026-04-14] feature/data-viewer-table-ui
- **목적**: Data Viewer 테이블 UI를 엑셀 스타일로 개선 + 탭 구분자 CSV 파싱 버그 수정
- **변경 내용**:
  - 테이블 격자선(세로 border-right), 행 번호 컬럼(`#`) sticky 고정, 폰트 확대(13px), 셀 패딩 확대
  - 숫자 컬럼 자동 감지(90% 기준) → 헤더·데이터 모두 오른쪽 정렬
  - CsvReader에 `DetectDelimiter()` 추가 → 탭/쉼표 자동 감지 지원
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/44
- **결과**: merged

---

## [2026-04-12] fix/code-review
- **목적**: 전체 코드 리뷰 기반 구조 수정 — Blocker 2건 + Major 4건 + Minor 4건 해소
- **변경 내용**:
  - [Blocker] pch.h에 escape-aware JsonExtractString 공유 헬퍼 추가 → configJson round-trip 깨짐 수정
  - [Blocker] WorkflowService::ExecuteWorkflowCore에서 sendEmail recipients 키 "to"→"recipients" 통일
  - [Major] WorkflowService 이중 실행 경로(RunWorkflow/RunSync)를 ExecuteWorkflowCore로 통합
  - [Major] JobQueueService 스레드 안전성: volatile LONG + InterlockedExchange/CompareExchange, CRITICAL_SECTION으로 m_strRunningJobId 보호
  - [Major] WorkflowBridgeHandler가 JobQueueBridgeHandler::EnqueueWorkflow 경유하도록 수정 — 별도 WorkflowService 직접 실행 제거
  - [Major] 워크플로 완료 후 m_currentTable 미갱신 수정: WorkflowService::GetLastOutputTable + MainWindow::UpdateCurrentTableFromWorkflow
  - [Minor] SageApp::Shutdown 조기 종료 조건 오류 수정 (m_pConfigStore → m_pLogger)
  - [Minor] STEP_TYPE_* 상수 Define.h 추가, 전체 문자열 리터럴 직접 비교 제거
  - [Minor] ApiCallAction 기본 생성자 추가 (m_nTimeoutMs = 30000)
  - [Minor] 12개 BridgeHandler 중복 구현(JsonEscapeString/JsonExtractString/JsonExtractBool) 전량 제거, pch.h 공유 헬퍼로 통일
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/43
- **결과**: merged ✅

## [2026-04-11] feature/conditional-step
- **목적**: Phase 7 Step 3 — Workflow 조건 분기 (Conditional Step)
- **변경 내용**: ConditionStep 도메인 모델 (field/operator/value/thenStepId/elseStepId), WorkflowService 실행 루프 for→while 리팩토링, EvaluateCondition/ParseConditionStep/FindStepIndex 메서드 추가, MAX_STEP_ITERATIONS(1000) 무한루프 가드, ExecuteSteps/RunSync 양쪽 모두 condition step 처리 추가, WebUI Workflow 편집기 condition 전용 폼(field/operator select/value/thenStep/elseStep), makeStepSelector 헬퍼, ko/en i18n 키 14개 추가
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/42
- **결과**: merged ✅

## [2026-04-10] feature/api-connector
- **목적**: Phase 7 Step 2 — API Connector 고도화
- **변경 내용**: ApiConnector 도메인 모델 (connectorId/name/baseUrl/headersJson/authType/authValue), ApiConnectorService (connectors.json 저장·로드/Add/Remove/Update/Test/BuildAction), ApiConnectorBridgeHandler (connector::getConnectors/addConnector/removeConnector/updateConnector/testConnector), WorkflowService callApi step에 connectorId 기반 BuildAction 연동 (하위 호환 유지), Settings 페이지 API Connectors 섹션, Workflow callApi step 커넥터 선택 드롭다운, ko.json/en.json/인라인 LOCALES connector i18n 키 추가
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/41
- **결과**: merged ✅

## [2026-04-09] feature/phase7-step1-scheduler
- **목적**: Phase 7 Step 1 — Scheduler (예약 실행)
- **변경 내용**: ScheduledJob 도메인 모델, SchedulerService (scheduler.json 저장·로드 / AddJob / RemoveJob / ToggleJob / GetDueJobs / CalcNextRunAt), SchedulerBridgeHandler (scheduler::getJobs/addJob/removeJob/toggleJob), JobQueueBridgeHandler에 EnqueueWorkflow() 추가, MainWindow WM_TIMER(60초) + OnSchedulerTick, Define.h SCHEDULER_* 상수, Settings 페이지 Scheduler 섹션 (워크플로우 드롭다운 + 시각 입력 + 추가/삭제/토글), ko.json/en.json settings.scheduler.* 키 추가
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/40
- **결과**: merged ✅

SageNexus 프로젝트의 PR 생성 및 머지 이력을 기록한다.

---

## [2026-04-08] feature/phase6-step6-i18n-qa
- **목적**: Phase 6 Step 6 — UI/i18n QA 하드코딩 텍스트 수정 및 누락 키 동기화
- **변경 내용**: Transform 페이지 meta / Artifact 목록 meta 하드코딩 '행/열' → t('unit.rows'/'unit.cols') 수정, breadcrumb·status-profile data-i18n 추가, LOCALES에 status.profile.default 키 추가, ko.json/en.json에 Phase 6 Step 1~4 누락 키 일괄 동기화 (hs.summary.*, hs.queue.*, wf.template.*, error.SNX_* 등)
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/38
- **결과**: merged ✅

---

## [2026-04-07] feature/phase6-step5-deployment-paths
- **목적**: Phase 6 Step 5 — 설치 폴더/사용자 데이터 폴더 분리 + 배포 정책 문서화
- **변경 내용**: SageApp::InitializePaths() → SHGetFolderPathW(CSIDL_APPDATA) 기반으로 변경, GetUserDataDir() 추가(설치 폴더와 사용자 데이터 폴더 명시적 구분), settings.json/Logs/Data/WebViewData → %APPDATA%\SageNexus\ 기준으로 이동, profile.json은 설치 폴더 유지, ArtifactStore/ExecutionHistoryStore/WorkflowStore GetDataDir() 직접 사용으로 통일, ADR-001-deployment-paths.md 경로 분리 정책 확정, SolutionProfile-packaging.md 패키징 단위 문서화
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/37
- **결과**: merged ✅

---

## [2026-04-06] feature/phase6-step4-workflow-template
- **목적**: Phase 6 Step 4 — 기본 Workflow 템플릿 + 사용자 메시지(에러 코드 i18n) 정리
- **변경 내용**: WorkflowTemplate 도메인 모델, WorkflowService::GetTemplates()/CreateFromTemplate() (내장 템플릿 2종: CSV→XLSX, 웹추출→HTML), workflow.templates::getTemplates/createFromTemplate 브릿지 핸들러, Workflow 페이지 "템플릿에서 시작" 버튼 + 템플릿 선택 모달, resolveErrorMessage() 에러 코드 i18n 매핑, ko.json/en.json 에러 코드 전체 번역 키 추가
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/36
- **결과**: merged ✅

---

## [2026-04-06] feature/phase6-step3-exception-log-retry
- **목적**: Phase 6 Step 3 — 예외/로그/복구 흐름 정리
- **변경 내용**: FileLogger 세션별 로그 파일(YYYYMMDD_HHMMSS.log), JobQueueService::RetryJob() 추가, execution.queue::retryJob 브릿지 핸들러, History 큐 UI 재실행 버튼 + 에러 메시지 레이아웃 개선 + cancelJob 클로저 버그 수정
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/35
- **결과**: merged ✅

## [2026-04-05] hotfix — Phase 6 Step 1/2 빌드 오류 수정 (develop 직접)
- **목적**: Phase 6 Step 1/2 작업 이후 발생한 빌드 오류 3건 수정
- **변경 내용**: ApiCallService.h에 winhttp.h include 추가(HINTERNET 타입 미인식), WorkflowBridgeHandler::GetCurrentStepName public으로 이동, RunSync EmailAction 필드명 m_strTo → m_strRecipients 수정
- **PR 링크**: develop 직접 커밋 (b53e3ec, cf1f474, e4f30f9)
- **결과**: merged ✅

## [2026-04-05] feature/phase6-step2-progress-summary
- **목적**: Phase 6 Step 2 — 진행률/취소/성공-실패 집계 고도화
- **변경 내용**: WorkflowService m_strCurrentStepName 추적 + GetCurrentStepName(), JobQueueService/BridgeHandler GetCurrentStepName() 노출, MainWindow OnWorkflowProgress에 percent+stepName 포함, execution.queue::cancelAll 추가, execution.summary::getSummary 추가(total/success/failed), Workflow cancel 버튼 cancelAll 통합, History 페이지 집계 카드 UI + summary-card CSS
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/34
- **결과**: merged ✅

## [2026-04-05] feature/phase6-step1-job-queue
- **목적**: Phase 6 Step 1 — JobQueue (작업 큐 + 배치 실행)
- **변경 내용**: ExecutionJob 도메인 모델, JobStatus enum, WM_JOB_QUEUE_CHANGED, JobQueueService(CRITICAL_SECTION 큐 + 워커 스레드 순차 실행 + cancel), WorkflowService::RunSync(동기 실행), JobQueueBridgeHandler(execution.queue::enqueue/getQueue/cancelJob), MainWindow WM_JOB_QUEUE_CHANGED → bridge:queue:changed 이벤트, History 페이지 큐 섹션 + i18n
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/33
- **결과**: merged ✅

## [2026-04-04] feature/phase5-step5-i18n
- **목적**: Phase 5 Step 5 — i18n 고도화 + Output Language QA + Phase 5 마무리
- **변경 내용**: ko.json/en.json 전체 키 동기화 (workflow/webextract/상태 누락 키 추가), 인라인 LOCALES에 신규 키 100여 개 추가, 하드코딩 텍스트 data-i18n 속성 및 t() 호출로 전면 교체 (Dashboard/DataViewer/Transform/Export/History/Settings/Workflow/WebExtract), 플러그인명 i18n(plugin.name.* 키), WebExtract Output Language 적용(getColDisplayName 사용), bridgeClient.request → sendCommand 버그 수정, Word/PDF Output Language 적용 확인
- **PR 링크**: pending
- **결과**: pending

## [2026-04-03] feature/api-call-action
- **목적**: Phase 5 Step 4 — API 전송 액션 (WinHTTP POST)
- **변경 내용**: ApiCallAction 도메인 모델, ApiCallService(WinHTTP HTTP 요청, 커스텀 헤더/바디/타임아웃), ApiCallBridgeHandler(workflow.api::callApi), WorkflowService callApi step 추가, Workflow 편집기 callApi 폼 추가
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/30
- **결과**: pending

## [2026-04-03] feature/phase5-step3-email-action
- **목적**: Phase 5 Step 3 — 이메일 발송 액션 (MAPI 기반)
- **변경 내용**: EmailAction 도메인 모델, EmailService(Simple MAPI MAPISendMailW), EmailBridgeHandler(workflow.email::sendEmail), WorkflowService sendEmail step 추가, Workflow 편집기 sendEmail 폼 추가
- **PR 링크**: pending
- **결과**: pending

## [2026-04-01] feature/pdf-export
- **목적**: Phase 5 Step 2 — PDF Export (HTML → Edge headless → PDF)
- **변경 내용**: PdfExporter(HTML 임시 파일 생성 + msedge headless --print-to-pdf, Edge 경로 자동 탐색), ExportService::ExportToPdf(), ExportBridgeHandler exportPdf action, Export 페이지 PDF 옵션 추가
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/28
- **결과**: merged ✅

## [2026-04-01] feature/word-export
- **목적**: Phase 5 Step 1 — Word Export (docx, OpenXML)
- **변경 내용**: WordExporter(OpenXML docx 직접 생성), ExportService::ExportToWord(), ExportBridgeHandler exportWord action, Export 페이지 Word 옵션 추가, WebExtractService 삼항 연산자 타입 모호성 수정
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/27
- **결과**: merged ✅

## [2026-03-29] feature/web-extract-module
- **목적**: Phase 4 Step 5 — Web Extract 모듈 MVP
- **변경 내용**: WebExtractService(WinHTTP HTTP/HTTPS + HTML 테이블 파싱 + CSS 선택자 지원), WebExtractBridgeHandler(webExtract::fetchAndExtract), WorkflowService webExtract 스텝 타입 추가, SageApp webextract 플러그인 등록, WebUI 웹 추출 페이지 + Workflow 편집기 webExtract 폼
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/26
- **결과**: merged ✅

## [2026-03-29] feature/workflow-step-editor
- **목적**: Phase 4 Step 4 — Workflow Step 편집기 + 실제 실행 연결
- **변경 내용**: WorkflowService ExecuteSteps에서 ImportService/TransformService/ExportService 실제 호출, ExtractConfigString/ParseTransformSteps 구현, WebUI Workflow 상세 편집기(step 추가/삭제, 타입별 config 폼 — import/transform/export, saveDetail → updateWorkflow 브릿지)
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/25
- **결과**: merged ✅

## [2026-03-29] feature/workflow-bridge-ui
- **목적**: Phase 4 Step 3 — WorkflowBridgeHandler + Workflow 페이지 기초
- **변경 내용**: WorkflowBridgeHandler(CRUD 6 action + run/cancel), MainWindow WM_WORKFLOW_PROGRESS/COMPLETE 처리 → SendEvent, SageApp workflow 플러그인 등록, WebUI Workflow 페이지(목록/empty/error/progress bar) + bridge:workflow:progress/complete 이벤트 처리 + i18n
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/24
- **결과**: merged ✅

## [2026-03-29] feature/workflow-service
- **목적**: Phase 4 Step 2 — WorkflowService + 실행 엔진 + progress 브릿지 구조
- **변경 내용**: WM_WORKFLOW_PROGRESS/WM_WORKFLOW_COMPLETE 메시지 상수, NavigationItem::Workflow, WorkflowService(CRUD + 워커 스레드 실행 + cancel + 이력 기록), progress PostMessage 구조
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/23
- **결과**: merged ✅

## [2026-03-29] feature/workflow-model
- **목적**: Phase 4 Step 1 — WorkflowDefinition 모델 + WorkflowStore 구현
- **변경 내용**: WorkflowStep 구조체(id/stepType/name/configJson), WorkflowDefinition 구조체(id/name/description/createdAt/updatedAt/steps[]), WorkflowStore(workflows.json CRUD — SaveWorkflow/LoadWorkflows/DeleteWorkflow), vcxproj·filters 등록
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/22
- **결과**: merged ✅

## [2026-03-28] feature/phase3-step7-settings-i18n
- **목적**: Phase 3 Step 7 — Settings 페이지 완성 + i18n 기초 적용
- **변경 내용**: SettingsBridgeHandler에 getInterfaceLanguage/setInterfaceLanguage 핸들러 추가, ko.json/en.json 기초 번역 리소스 신규 생성(webui/src/i18n/), LOCALES 객체+t()/applyLocale() 함수 추가, 사이드바 nav·모든 페이지 타이틀에 data-i18n 적용, Settings 페이지 솔루션 프로필/언어/플러그인 3섹션으로 재편, Interface Language select UI 추가, bridge:appReady 시 interfaceLanguage 초기 로드
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/20
- **결과**: merged ✅

## [2026-03-28] feature/output-language
- **목적**: Phase 3 Step 6 — Output Language 반영
- **변경 내용**: settings.language 브릿지(getOutputLanguage/setOutputLanguage), CsvWriter/XlsxWriter/HtmlReportExporter에 strLang 파라미터 추가, ExportService/ExportBridgeHandler 연결, Import/Transform 브릿지 columns에 displayNameKo/En 분리 전달, WebUI Settings 결과물 언어 select UI, Data Viewer/Transform 테이블 컬럼 헤더 언어 반영
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/19
- **결과**: merged ✅

## [2026-03-27] feature/html-export
- **목적**: Phase 3 Step 5 — HTML Report Export + Artifact 모델
- **변경 내용**: Artifact 도메인 모델, HtmlReportExporter(자체 포함 HTML), ArtifactStore(artifacts.json 영속화), ExportService::ExportToHtml(), exportHtml/getArtifacts 브릿지, Export 페이지 결과물 목록 UI
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/18
- **결과**: merged ✅

## [2026-03-26] feature/xlsx-export
- **목적**: Phase 3 Step 4 — XLSX Export 구현
- **변경 내용**: XlsxWriter(Office Open XML + PowerShell ZipFile::CreateFromDirectory), ExportService::ExportToXlsx(), ExportBridgeHandler exportXlsx action, Export 페이지 형식 선택 UI(CSV/XLSX), SettingsBridgeHandler m_strPayload 버그 수정
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/17
- **결과**: merged ✅

## [2026-03-26] feature/plugin-menu-control
- **목적**: Phase 3 Step 3 — 플러그인 활성화 제어 및 사이드바 동적 렌더링
- **변경 내용**: getPlugins/togglePlugin 브릿지 커맨드 추가, 사이드바 메뉴 플러그인 활성화 여부 기반 동적 렌더링, Settings 플러그인 on/off 토글 UI 구현
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/16
- **결과**: merged ✅

## [2026-03-26] feature/plugin-manager
- **목적**: Phase 3 Step 2 — IPlugin 인터페이스 및 PluginManager 구현
- **변경 내용**: IPlugin 인터페이스, PluginManager, SolutionProfile plugin_* 파싱, SageApp 내장 플러그인 4개 등록
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/15
- **결과**: merged ✅

## [2026-03-25] feature/solution-profile-load
- **목적**: Phase 3 Step 1 — SolutionProfile 파일 로드 및 Settings 브릿지 연동
- **변경 내용**: LoadFromFile() 구현, profile.json 자동 생성, SettingsBridgeHandler 신규, Settings 페이지 프로필 정보 표시
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/14
- **결과**: merged ✅

## [2026-03-22] feature/phase1-shell
- **목적**: Phase 1 — Win32/WebView2 앱 셸 및 공통 뼈대 구축
- **변경 내용**: MainWindow, WebViewHost, BridgeDispatcher, SageApp, FileLogger, JsonConfigStore, DataTable, SolutionProfile, Web UI 셸(index.html/bridge.js/styles.css), vcxproj 전체 구성
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/2
- **결과**: merged

## [2026-03-24] develop → main (Phase 2 완료)
- **목적**: Phase 2 완료 시점 main 반영
- **변경 내용**: Step 1~7 포함 (CSV/XLSX 읽기, Data Viewer, Transform, CSV Export, 실행 이력, 날짜 포맷 정규화, Dashboard 통계)
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/13
- **결과**: merged ✅

## [2026-03-24] feature/xlsx-date-format-dashboard
- **목적**: Phase 2 Step 7 — XLSX 날짜 포맷 정규화 및 Dashboard 통계 표시
- **변경 내용**: XlsxReader에 styles.xml 파싱·IsDateNumFmtId·SerialToDate 추가(Excel serial → YYYY-MM-DD), Dashboard stat cards를 getHistory로 실제 이력 통계 표시
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/12
- **결과**: merged ✅

## [2026-03-24] feature/execution-history
- **목적**: Phase 2 Step 6 — 실행 이력 기록 및 History 페이지 구현
- **변경 내용**: ExecutionRecord 도메인 모델, ExecutionHistoryStore(JSON 파일 저장/로드), HistoryService, HistoryBridgeHandler(history.query::getHistory), Import/Transform/Export 핸들러 이력 기록 추가, History 페이지 UI(4상태)
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/11
- **결과**: merged ✅

## [2026-03-24] feature/xlsx-reader
- **목적**: Phase 2 Step 5 — XLSX 파일 읽기 지원 (XlsxReader)
- **변경 내용**: XlsxReader(PowerShell Expand-Archive ZIP 추출·sharedStrings·sheet1 XML 파싱·희소 컬럼 처리), ImportService XLSX 라우팅 추가, ImportBridgeHandler 파일 필터 업데이트(csv+xlsx), vcxproj/filters 등록
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/10
- **결과**: merged ✅

## [2026-03-23] feature/csv-export
- **목적**: Phase 2 Step 4 — CSV Export (저장 다이얼로그 + CsvWriter + Export 페이지)
- **변경 내용**: CsvWriter(UTF-8 BOM·RFC4180 쿼팅), ExportService, ExportBridgeHandler(artifact.export::exportCsv), MainWindow 핸들러 등록, Export 페이지 UI(요약 카드·완료/에러 상태)
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/9
- **결과**: merged ✅

## [2026-03-23] feature/transform-basic
- **목적**: Phase 2 Step 3 — 기본 변환 기능 (trim / renameColumn / replaceValue)
- **변경 내용**: TransformStep 도메인 모델, TransformService(3개 변환 규칙), TransformBridgeHandler(data.transform::applySteps), DataTable 뮤터블 접근자 추가, ImportBridgeHandler 공유 DataTable*로 전환, MainWindow DataTable 소유, Transform 페이지 UI(단계 빌더 + 결과 테이블)
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/8
- **결과**: merged ✅

## [2026-03-23] feature/ui-design-tokens
- **목적**: sagenexus-ui 기준 디자인 토큰 교체 (웜 베이지/브라운 Light Mode)
- **변경 내용**: styles.css 전체 토큰 교체, Pretendard 폰트 적용, 0.5px 보더, 10px 카드 라운딩
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/7
- **결과**: merged ✅

## [2026-03-23] feature/data-viewer-mvp
- **목적**: Phase 2 Step 2 — Import 브릿지 + Data Viewer MVP
- **변경 내용**: ImportBridgeHandler(openFileDialog/loadFile), MainWindow 핸들러 등록, Data Viewer 페이지(4상태·테이블 렌더링), 버튼/테이블/툴바 CSS
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/6
- **결과**: merged ✅

## [2026-03-23] feature/csv-reader
- **목적**: Phase 2 Step 1 — CsvReader, ImportService, IDataReader 구현 및 앱 즉시 종료 버그 수정
- **변경 내용**: IDataReader 인터페이스, CsvReader(UTF-8/ANSI 폴백·쿼트 처리), ImportService(확장자 기반 분기), main.cpp MainWindow::Create 호출 누락 버그 수정
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/5
- **결과**: merged ✅

## [2026-03-22] develop → main (Phase 1 완료)
- **목적**: Phase 1 완료 시점 main 반영
- **변경 내용**: PR #2 (phase1-shell), PR #3 (project-folder-structure) 포함
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/4
- **결과**: merged ✅

## [2026-03-22] refactor/project-folder-structure
- **목적**: coding-rules 표준 폴더 구조로 전체 소스 파일 재배치 및 VS Solution Explorer 필터 재구성
- **변경 내용**: App/Host/Domain/Infrastructure/Web → app/host, app/application, app/domain/model, app/infrastructure/logging, app/infrastructure/config, webui 구조로 이동. vcxproj, vcxproj.filters, Define.h, index.html 업데이트
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/3
- **결과**: merged
