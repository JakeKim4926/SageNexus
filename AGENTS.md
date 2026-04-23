# AGENTS.md

This file is the root working guide for Codex/OpenAI agents in the SageNexus repository.
Use it as the top-level repo guide, then open the relevant skill under `.codex/skills/*/SKILL.md` before making changes.

## Product Direction

SageNexus is a C++ / Win32 / WebView2 based business automation platform.
Assume the following structure and constraints:

- Core Platform + Functional Modules + Solution Profiles
- Layered Architecture + Plugin Architecture
- WebView2 UI is state-driven
- Internal data is standardized around `DataTable`
- Each company should be able to combine only the functions it needs
- UI language and output language are intentionally separated

This is not a one-size-fits-all utility.
It is a company-specific automation platform built on a shared core.

## Skills To Use First

Codex should use the repository skills below based on the task.

### `coding-rules`
File: `.codex/skills/coding-rules/SKILL.md`

Use first for:

- C++ / Win32 / MFC / WebView2 code changes
- service / engine / view / plugin layer changes
- class design, function design, return types, exception handling

### `git-workflow`
File: `.codex/skills/git-workflow/SKILL.md`

Use first for:

- branch creation
- commit cleanup
- PR creation or merge
- phase completion logging

### `sagenexus-plan`
File: `.codex/skills/sagenexus-plan/SKILL.md`

Use first for:

- deciding whether work fits the current phase
- separating MVP scope from later scope
- deciding next implementation priority

### `webview2-ui`
File: `.codex/skills/webview2-ui/SKILL.md`

Use first for:

- adding or modifying WebView2 pages
- frontend state structure
- shared component extraction
- bridge client usage and page logic structure

### `webview2-bridge`
File: `.codex/skills/webview2-bridge/SKILL.md`

Use first for:

- native to WebView2 JSON message design
- `target` / `action` design
- response, progress, timeout, cancel behavior

### `sagenexus-ui`
File: `.codex/skills/sagenexus-ui/SKILL.md`

Use first for:

- converting mockups into actual product UI
- page layout and section design
- UI consistency and i18n-aware presentation

## Skill Separation

Use the skills with these boundaries:

- `sagenexus-ui`: what the product UI should show, page composition, visual and i18n behavior
- `webview2-ui`: how the frontend is implemented, state structure, component boundaries, bridge client usage
- `webview2-bridge`: how native and UI communicate through JSON contracts

In short:

- `sagenexus-ui` = product UI guide
- `webview2-ui` = frontend implementation guide
- `webview2-bridge` = native/UI contract guide

## Task-Based Reading Order

### C++ code work
1. `coding-rules`
2. `sagenexus-plan`
3. `webview2-bridge` if bridge work is involved

### Git / branch / PR work
1. `git-workflow`

### New feature work
1. `sagenexus-plan`
2. `coding-rules`
3. `webview2-ui`, `webview2-bridge`, `sagenexus-ui` as needed

### WebView2 page work
1. `sagenexus-ui`
2. `webview2-ui`
3. `webview2-bridge` if messaging is involved

### Native to WebView2 message work
1. `webview2-bridge`
2. `webview2-ui`
3. `coding-rules`

### Mockup to real UI work
1. `sagenexus-ui`
2. `webview2-ui`
3. `webview2-bridge` if messaging is involved

## Architecture Interpretation

Treat the project as these layers.

### Host / Presentation
Examples:

- `MainWindow`
- `WebViewHost`
- `NavigationController`
- `BridgeDispatcher`

Responsibilities:

- windows, messages, WebView2 hosting, UI connection

Must not directly do:

- file parsing
- data transformation
- document export
- long-running synchronous work

### Application
Examples:

- `ImportService`
- `TransformService`
- `ExportService`
- `ExecutionService`

Responsibilities:

- use case orchestration
- execution flow management
- validation and orchestration

### Domain
Examples:

- `DataTable`
- `TransformPlan`
- `ExecutionResult`
- `WorkflowDefinition`
- `SolutionProfile`

Responsibilities:

- core data model
- business rules
- structures with low dependence on UI or file formats

### Infrastructure
Examples:

- `CsvReader`
- `XlsxReader`
- `HtmlReportExporter`
- `JsonConfigStore`
- `FileLogger`
- `PluginManager`

Responsibilities:

- concrete I/O
- external library integration
- file, settings, logging, system access

## Feature Placement Rules

Before adding a feature, decide whether it is:

1. a shared core capability
2. an optional functional module
3. specific to one customer profile

Do not push every feature into the core.
Always protect the ability to compose different feature sets per company.

Keep the `Plugin` and `SolutionProfile` structure intact.

## Data Model Rules

- Keep `DataTable` as the internal standard model
- Converge CSV, XLSX, and extracted web data into the same intermediate representation where possible
- Separate source column names from display names

Preferred concepts:

- `internalName`
- `sourceName`
- `displayNameKo`
- `displayNameEn`

This structure directly supports Transform `Source / Display` toggles and output language policy.

## WebView2 UI Rules

Treat WebView2 as the product frontend, not as a passive HTML view.
Always verify:

- the page remains state-driven
- native requests go through `bridgeClient`
- loading / empty / error states exist
- disabled plugin or profile states are considered
- shared tokens and shared components are reused
- `Interface Language` and `Output Language` remain separated

Default page set:

- Dashboard
- Data Viewer
- Transform
- Export
- History
- Settings

## i18n Rules

SageNexus separates interface language from output language.

### Interface Language
Applies to:

- menus
- buttons
- labels
- state messages
- page titles
- empty / loading / error messages

### Output Language
Applies to:

- column display names
- document template titles
- output headers

### Never translate

- actual file names
- Run ID
- error codes
- original column names
- customer source data values

Base files:

- `ko.json`
- `en.json`

## Bridge Rules

Always keep these rules for native/UI messaging:

- all communication uses JSON
- message types are `command`, `response`, `event`
- every command includes `requestId`
- every response keeps `success`, `payload`, `error`
- long-running work should consider progress events
- the UI should not know native internals
- native code should not know DOM structure

Forbidden:

- direct `window.chrome.webview.postMessage` usage from pages
- commands without `requestId`
- ending with progress only and no final response
- mixing `error` into `payload`

## Code Rules

Always keep these repo-level rules in mind:

- do not use `auto`
- do not use smart pointers
- do not return pointers from functions
- do not do heavy work on the UI thread
- do not access UI objects directly from worker threads
- do not prefix class names with `C`
- keep member names in `m_` + existing Hungarian style
- prefer `BOOL` for recoverable operations that can fail
- manage constants in `Define.h` and enums in `EnumDefine.h`

Open `coding-rules` before changing code.

## Git Rules

Always keep these repo-level rules in mind:

- do not commit directly to `main` or `develop`
- use `feature/*`, `fix/*`, `refactor/*`, `docs/*` branches
- default PR base is `develop`
- use squash merge by default
- delete merged branches
- keep `docs/decisions/PR_LOG.md` updated

Open `git-workflow` before branch, commit, PR, or merge work.

## Request Handling Order

When a task arrives, follow this order:

1. decide which layer the task belongs to
2. decide whether it belongs to core, plugin, or solution profile
3. check whether it fits the current `sagenexus-plan` phase
4. verify return types, exceptions, and thread ownership against `coding-rules`
5. if the task touches WebView2 UI, apply `sagenexus-ui` and `webview2-ui`
6. if the task touches messaging, apply `webview2-bridge`
7. consider branch, commit, and PR flow after the change

## Documentation Rules

When writing docs:

- prefer explicit structure and responsibility over abstract phrasing
- separate completion criteria, scope, and out-of-scope items
- keep docs connectable to phase docs, ADRs, and `PR_LOG`
- use existing project terminology consistently
- do not mix product UI policy with implementation rules

## Final Decision Heuristics

If there is no direct written rule, choose the option that best satisfies:

1. simpler structure
2. clearer responsibility boundaries
3. preserved UI responsiveness
4. preserved per-company feature composition
5. clear separation between interface language and output language
6. no unnecessary pollution of the shared core
