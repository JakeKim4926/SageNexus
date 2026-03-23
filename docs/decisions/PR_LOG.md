# PR 작업 로그

SageNexus 프로젝트의 PR 생성 및 머지 이력을 기록한다.

---

## [2026-03-22] feature/phase1-shell
- **목적**: Phase 1 — Win32/WebView2 앱 셸 및 공통 뼈대 구축
- **변경 내용**: MainWindow, WebViewHost, BridgeDispatcher, SageApp, FileLogger, JsonConfigStore, DataTable, SolutionProfile, Web UI 셸(index.html/bridge.js/styles.css), vcxproj 전체 구성
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/2
- **결과**: merged

## [2026-03-22] develop → main (Phase 1 완료)
- **목적**: Phase 1 완료 시점 main 반영
- **변경 내용**: PR #2 (phase1-shell), PR #3 (project-folder-structure) 포함
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/4
- **결과**: merged

## [2026-03-23] feature/ui-design-tokens
- **목적**: sagenexus-ui 기준 디자인 토큰 교체 (웜 베이지/브라운 Light Mode)
- **변경 내용**: styles.css 전체 토큰 교체, Pretendard 폰트 적용, 0.5px 보더, 10px 카드 라운딩
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/7
- **결과**: merged

## [2026-03-23] feature/data-viewer-mvp
- **목적**: Phase 2 Step 2 — Import 브릿지 + Data Viewer MVP
- **변경 내용**: ImportBridgeHandler(openFileDialog/loadFile), MainWindow 핸들러 등록, Data Viewer 페이지(4상태·테이블 렌더링), 버튼/테이블/툴바 CSS
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/6
- **결과**: merged

## [2026-03-23] feature/csv-reader
- **목적**: Phase 2 Step 1 — CsvReader, ImportService, IDataReader 구현 및 앱 즉시 종료 버그 수정
- **변경 내용**: IDataReader 인터페이스, CsvReader(UTF-8/ANSI 폴백·쿼트 처리), ImportService(확장자 기반 분기), main.cpp MainWindow::Create 호출 누락 버그 수정
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/5
- **결과**: merged

## [2026-03-23] feature/transform-basic
- **목적**: Phase 2 Step 3 — 기본 변환 기능 (trim / renameColumn / replaceValue)
- **변경 내용**: TransformStep 도메인 모델, TransformService(3개 변환 규칙), TransformBridgeHandler(data.transform::applySteps), DataTable 뮤터블 접근자 추가, ImportBridgeHandler 공유 DataTable*로 전환, MainWindow DataTable 소유, Transform 페이지 UI(단계 빌더 + 결과 테이블)
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/8
- **결과**: merged

## [2026-03-22] refactor/project-folder-structure
- **목적**: coding-rules 표준 폴더 구조로 전체 소스 파일 재배치 및 VS Solution Explorer 필터 재구성
- **변경 내용**: App/Host/Domain/Infrastructure/Web → app/host, app/application, app/domain/model, app/infrastructure/logging, app/infrastructure/config, webui 구조로 이동. vcxproj, vcxproj.filters, Define.h, index.html 업데이트
- **PR 링크**: https://github.com/JakeKim4926/SageNexus/pull/3
- **결과**: merged
