# PR 작업 로그

SageNexus 프로젝트의 PR 생성 및 머지 이력을 기록한다.

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
