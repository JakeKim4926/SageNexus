# SageNexus

> Modular office automation platform for structured data workflows.

SageNexus는 반복적인 사무 업무를  
**입력 → 정리 → 실행 → 결과물 생성 → 이력 관리** 흐름으로 구조화하고,  
이를 **회사별 요구에 맞게 기능 단위로 조합할 수 있도록 설계된 업무 자동화 플랫폼**입니다.

---

## Overview

현업의 많은 업무는 비슷한 패턴을 반복합니다.

- CSV / XLSX / 웹페이지 등 다양한 형태의 데이터 입력
- 열 재배치, 값 치환, 포맷 정리 등 데이터 가공
- 필요 시 웹 데이터 추출 또는 실행 자동화
- CSV / XLSX / HTML / Word / PDF 등 결과물 생성
- 실행 로그 및 이력 관리

SageNexus는 이 흐름을 하나의 제품 안에서 다룰 수 있도록 설계된  
**조합형 업무 자동화 플랫폼**입니다.

---

## Why SageNexus?

많은 자동화 도구는 기능은 많지만, 실제 업무 흐름과는 잘 붙지 않습니다.

SageNexus는 다음 문제를 해결하는 데 초점을 맞춥니다.

- 입력 데이터 형식이 제각각이다
- 사람이 같은 정리 작업을 반복한다
- 회사마다 필요한 기능 구성이 다르다
- 자동화가 실행돼도 결과물과 이력이 남지 않는다

SageNexus는 이런 문제를 해결하기 위해  
**공통 코어 + 선택 모듈 + 회사별 프로필** 구조를 채택합니다.

---

## Core Concept

SageNexus는 모든 기능을 한 번에 넣는 범용 툴이 아닙니다.

대신 아래 구조를 따릅니다.

- **Core Platform**
- **Functional Modules / Plugins**
- **Solution Profiles**

즉 다양한 플랫폼에 적용가능합니다.

- 어떤 회사는 **웹 데이터 수집 + 엑셀 저장**
- 어떤 회사는 **재무제표 추출 + 문서 생성**
- 어떤 회사는 **데이터 정리 + 결과 export**

만 필요할 수 있습니다.

SageNexus는 이러한 차이를 **기능 조합형 구조**로 수용합니다.

---

## Architecture

### Native Layer
- C++
- Win32
- WebView2 Host
- File I/O
- Execution Engine
- Config / Logging / Plugin Management

### WebView2 UI Layer
- HTML / CSS / JavaScript
- State-driven UI
- Dashboard / Data Viewer / Transform / Export / History / Settings
- Interface Language / Output Language 분리

### Internal Structure
- Layered Architecture
- Plugin Architecture
- `DataTable` 중심 표준 데이터 모델
- JSON 기반 Native ↔ UI Bridge
- Solution Profile 기반 기능 제어

---

## Main Features

### Core
- App Shell
- WebView2 UI Host
- Config Manager
- Logger / Execution History
- Data Transform Engine
- Plugin Manager
- Profile Resolver

### Modules (구현 완료)
- CSV Import
- XLSX Import
- Data Transform (trim / rename / replace)
- Web Extract (WinHTTP + HTML 파싱)
- CSV Export
- XLSX Export
- HTML Report Export
- Workflow Builder (Step 편집 + 실행)

### Modules (예정)
- Word Export
- PDF Export
- Scheduler
- API Connector

---

## UI & i18n Policy

SageNexus는 **UI 언어**와 **결과물 언어**를 분리합니다.

### Interface Language
- 메뉴
- 버튼
- 라벨
- 상태 메시지

### Output Language
- 컬럼 표시명
- 문서 템플릿 제목
- 결과물 헤더

예를 들어 다음과 같은 조합이 가능합니다.

- UI는 English
- 결과물은 한국어

이 구조를 통해 내부 운영자용 UI와 고객사 전달용 결과물을 분리할 수 있습니다.

---

## Development Direction

SageNexus는 다음 방향으로 설계 및 구현을 진행합니다.

- Phase 기반 점진 개발
- MVP 우선 구현
- 코어와 플러그인 구조 먼저 정립
- CSV / XLSX 기반 데이터 처리 루프 → Web Extract / Workflow 순으로 완성
- 이후 Word / PDF Export, 이메일/API 액션, 운영성 강화 확장

---

## Planned Phases

### Phase 1 ✅
플랫폼 셸 및 공통 뼈대 구축

### Phase 2 ✅
CSV / XLSX 데이터 처리 MVP, Data Viewer, Transform, Export, 실행 이력

### Phase 3 ✅
플러그인 런타임, 메뉴 동적 제어, XLSX / HTML Export, Output Language, i18n 기초

### Phase 4 ✅
Workflow Builder, Web Extract 모듈, progress 브릿지

### Phase 5 (진행 예정)
Word / PDF Export, 이메일 / API 액션, i18n 고도화

### Phase 6 (계획)
운영성 강화, 배치 실행, 제품화 정리

---

## Project Principles

- 한 번에 완성형을 만들지 않는다
- 각 Phase마다 실행 가능한 빌드를 남긴다
- 공통 코어와 선택 기능을 분리한다
- 결과물과 실행 이력을 반드시 남긴다
- UI와 비즈니스 로직을 분리한다
- 내부 데이터는 표준 모델로 통일한다
- 코딩 규칙 / Git Workflow / UI 규칙 / 브릿지 규약을 문서로 고정한다

---

## Skill System

이 프로젝트는 아래 기준 문서를 바탕으로 운영됩니다.

- `coding-rules`
- `git-workflow`
- `SageNexus-plan`
- `webview2-ui`
- `webview2-bridge`
- `sagenexus-ui`

각 문서는 코드 스타일, Git 운영, 개발 단계, WebView2 UI 구조, 브릿지 메시지 규약, 제품 UI 기준을 정의합니다.

---

## Long-term Direction

SageNexus는 단순한 자동화 프로그램이 아니라,  
**고객사별로 다른 반복 업무를 기능 조합 형태로 대응할 수 있는 업무 자동화 플랫폼**을 목표로 합니다.

장기적으로는 다음과 같은 확장을 고려합니다.

- Word / PDF 고도화
- Workflow 시각 편집기
- Scheduler
- API / DB Connector
- 이메일 연동
- 템플릿 공유
- AI 기반 문서 요약 및 작성 보조

---

## Status

**Phase 4 완료 / Phase 5 진행 예정**

Phase 1~4 구현 완료. CSV/XLSX 입출력, Data Transform, Web Extract, Workflow Builder, Plugin Manager, i18n 기초까지 동작하는 빌드가 존재합니다.
현재 Word/PDF Export 및 액션 확장(Phase 5)을 준비 중입니다.

---