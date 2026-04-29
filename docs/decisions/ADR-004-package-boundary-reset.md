# ADR-004: Core / Plugin / Deploy Boundary Reset

- **Date**: 2026-04-29
- **Status**: Accepted
- **Related Phase**: Phase 8 stabilization

## Decision

SageNexus separates build output, plugin output, and runnable deployment output.

The runnable application must be assembled only under `deploy/<profile>/`.
`Debug_x64/` and `Release_x64/` are build output folders, not customer package folders.

## Target Structure

```text
D:\Projects\SageNexus\
  Debug_x64\
  Release_x64\
  scripts\package.ps1
  deploy\
    taechang\
      SageNexus.exe
      WebView2Loader.dll
      profile.json
      plugins\
        SageNexus-Plugin-Taechang.dll
        web\
        tools\
        templates\
        rules\

D:\Projects\SageNexus-Plugin-Taechang\
  Debug_x64\
  Release_x64\
  web\
  tools\
  templates\
  rules\
```

## Responsibility Boundaries

### SageNexus Core

SageNexus owns the shared platform.

- Win32 / WebView2 host
- Core WebView2 shell UI
- Bridge dispatcher
- Plugin loading
- Solution profile loading
- Shared services and shared product pages

The core repository must not contain Taechang-only business logic.

### Taechang Plugin

`SageNexus-Plugin-Taechang` owns Taechang-specific behavior.

- Taechang DLL
- Taechang WebView2 plugin pages
- Taechang Excel / PDF / HWP tools
- Taechang templates
- Taechang mapping rules

Taechang files are included in the product only through packaging.

### Profile

`profile.json` is a composition file.

It decides which shared pages, plugin pages, and plugin features are enabled for a customer.
It must not be used as an implicit build script or file copy rule.

## Packaging Rule

Use one command from the SageNexus repository:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1 -Profile taechang -BuildType Release
```

The script reads:

- Core executable files from `D:\Projects\SageNexus\Release_x64\`
- Taechang plugin DLL from `D:\Projects\SageNexus-Plugin-Taechang\Release_x64\`
- Taechang plugin assets from `D:\Projects\SageNexus-Plugin-Taechang\`
- Profile from `D:\Projects\SageNexus\SageNexus\profiles\taechang.json`

The script writes:

- `D:\Projects\SageNexus\deploy\taechang\`

## Out Of Scope

- Rewriting core services
- Rewriting Taechang plugin business logic
- Changing the plugin ABI
- Moving profile storage to AppData

Legacy plugin post-build copy events are removed from the Taechang plugin project so plugin builds no longer mutate SageNexus build output folders.
