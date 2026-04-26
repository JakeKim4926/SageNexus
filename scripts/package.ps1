param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("default", "acme", "beta", "taechang")]
    [string]$Profile,

    [Parameter(Mandatory = $false)]
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Release"
)

$SolutionDir = Split-Path $PSScriptRoot -Parent
$ProjectDir = Join-Path $SolutionDir "SageNexus"
$BuildConfig = "${BuildType}_x64"
$SourceDir = Join-Path $ProjectDir $BuildConfig
$DeployDir = Join-Path $SolutionDir "deploy\${Profile}"

if (-not (Test-Path $SourceDir)) {
    Write-Error "Build output directory not found: $SourceDir"
    Write-Error "Build '$BuildConfig' first."
    exit 1
}

if (Test-Path $DeployDir) {
    Remove-Item $DeployDir -Recurse -Force
}
New-Item $DeployDir -ItemType Directory | Out-Null

$Items = @(
    "SageNexus.exe",
    "WebView2Loader.dll"
)

foreach ($Item in $Items) {
    $Src = Join-Path $SourceDir $Item
    if (Test-Path $Src) {
        Copy-Item $Src $DeployDir
    } else {
        Write-Warning "Missing file: $Item"
    }
}

foreach ($Folder in @("webui", "resources")) {
    $Src = Join-Path $SourceDir $Folder
    if (Test-Path $Src) {
        Copy-Item $Src $DeployDir -Recurse
    }
}

$PluginsDir = Join-Path $DeployDir "plugins"
New-Item $PluginsDir -ItemType Directory -Force | Out-Null

$SrcPlugins = Join-Path $SourceDir "plugins"
if (Test-Path $SrcPlugins) {
    Get-ChildItem $SrcPlugins -Filter "*.dll" | ForEach-Object {
        Copy-Item $_.FullName $PluginsDir
        Write-Host "  Plugin copied: $($_.Name)"
    }
}

Write-Host ""
Write-Host "Package ready: $DeployDir"
Write-Host "  Profile : $Profile"
Write-Host "  Build   : $BuildType"
Write-Host "  Plugins : $((Get-ChildItem $PluginsDir -Filter '*.dll').Count)"
