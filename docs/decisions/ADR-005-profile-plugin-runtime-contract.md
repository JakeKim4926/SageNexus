# ADR-005: Profile / Plugin Runtime Contract

- **Date**: 2026-04-29
- **Status**: Accepted
- **Related Phase**: Phase 8 stabilization

## Decision

Installed plugins and enabled plugins are different concepts.

A plugin DLL may exist in `plugins/`, but it is usable only when the current `profile.json` enables that plugin.

## Runtime Rules

### Loaded

The core loads plugin DLLs from the package `plugins/` folder.
Loaded plugins can report commands and pages to the core.

### Enabled

The active `profile.json` decides whether a loaded plugin is enabled.

If a plugin is disabled:

- its sidebar pages are hidden
- its WebView2 plugin files are not served
- its bridge commands return `SNX_PLUGIN_DISABLED`

## UI Contract

The `appReady` event includes plugin pages with an `enabled` flag.

```json
{
  "pluginPages": [
    {
      "pluginId": "taechang",
      "pageId": "taechang-delivery",
      "pageName": "Delivery Form",
      "entryUrl": "https://app.sagenexus/plugins/taechang/web/delivery.html",
      "enabled": true
    }
  ]
}
```

The WebView2 shell must render plugin pages from this payload and hide entries where `enabled` is `false`.

## Profile Role

`profile.json` is the customer composition file.
It controls available shared menus and enabled plugin modules.

It must not contain file copy rules or build output paths.

Profile files use explicit arrays for composition.

```json
{
  "menus": [
    { "id": "history", "enabled": true },
    { "id": "workflow", "enabled": false }
  ],
  "plugins": [
    { "id": "history", "enabled": true },
    { "id": "taechang", "enabled": true }
  ]
}
```

Legacy flat keys such as `showHistory` and `plugin_taechang` remain readable for old packages, but newly saved profiles use the array schema.

## History Metadata

Plugin ABI v3 lets each plugin provide command history metadata.

```json
{
  "target": "taechang.delivery",
  "action": "generateDeliveryForms",
  "operationType": "taechang.delivery"
}
```

The core records plugin execution history from this metadata.
The core must not contain customer-specific command-to-history mappings.

Changing this contract requires rebuilding both SageNexus and customer plugin DLLs.
