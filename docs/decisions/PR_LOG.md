# PR 작업 로그

SageNexus 프로젝트의 PR 생성 및 머지 이력을 기록한다.

---

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
