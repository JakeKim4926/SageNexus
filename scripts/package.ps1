param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("default", "acme", "beta", "taechang")]
    [string]$Profile,

    [Parameter(Mandatory = $false)]
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Release",

    [Parameter(Mandatory = $false)]
    [string]$TaechangPluginRoot
)

$ErrorActionPreference = "Stop"

$SolutionDir = Split-Path $PSScriptRoot -Parent
$ProjectDir = Join-Path $SolutionDir "SageNexus"
$BuildConfig = "${BuildType}_x64"
$CoreBuildDir = Join-Path $SolutionDir $BuildConfig
$DeployRoot = Join-Path $SolutionDir "deploy"
$DeployDir = Join-Path $DeployRoot $Profile
$ProfilePath = Join-Path $ProjectDir "profiles\$Profile.json"

if ([string]::IsNullOrWhiteSpace($TaechangPluginRoot)) {
    $TaechangPluginRoot = Join-Path $SolutionDir "..\SageNexus-Plugin-Taechang"
}

function Assert-PathExists {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,

        [Parameter(Mandatory = $true)]
        [string]$Label
    )

    if (-not (Test-Path $Path)) {
        throw "$Label not found: $Path"
    }
}

function Copy-RequiredFile {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Source,

        [Parameter(Mandatory = $true)]
        [string]$Destination
    )

    Assert-PathExists -Path $Source -Label "Required file"
    Copy-Item -LiteralPath $Source -Destination $Destination -Force
}

function Copy-OptionalDirectory {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Source,

        [Parameter(Mandatory = $true)]
        [string]$Destination
    )

    if (Test-Path $Source) {
        New-Item -ItemType Directory -Path $Destination -Force | Out-Null
        Copy-Item -Path (Join-Path $Source "*") -Destination $Destination -Recurse -Force
    }
}

Assert-PathExists -Path $CoreBuildDir -Label "Core build directory"
Assert-PathExists -Path $ProfilePath -Label "Profile"

if (Test-Path $DeployDir) {
    Remove-Item -LiteralPath $DeployDir -Recurse -Force
}

New-Item -ItemType Directory -Path $DeployDir -Force | Out-Null

Copy-RequiredFile -Source (Join-Path $CoreBuildDir "SageNexus.exe") -Destination $DeployDir
Copy-RequiredFile -Source (Join-Path $CoreBuildDir "WebView2Loader.dll") -Destination $DeployDir
Copy-RequiredFile -Source $ProfilePath -Destination (Join-Path $DeployDir "profile.json")

$PluginCount = 0

if ($Profile -eq "taechang") {
    $PluginBuildDir = Join-Path $TaechangPluginRoot $BuildConfig
    $PluginDllPath = Join-Path $PluginBuildDir "SageNexus-Plugin-Taechang.dll"
    $PluginsDir = Join-Path $DeployDir "plugins"

    Assert-PathExists -Path $TaechangPluginRoot -Label "Taechang plugin repository"
    Assert-PathExists -Path $PluginBuildDir -Label "Taechang plugin build directory"
    Assert-PathExists -Path $PluginDllPath -Label "Taechang plugin DLL"

    New-Item -ItemType Directory -Path $PluginsDir -Force | Out-Null
    Copy-RequiredFile -Source $PluginDllPath -Destination $PluginsDir

    foreach ($AssetDirName in @("web", "tools", "templates", "rules")) {
        $AssetSource = Join-Path $TaechangPluginRoot $AssetDirName
        $AssetDestination = Join-Path $PluginsDir $AssetDirName
        Copy-OptionalDirectory -Source $AssetSource -Destination $AssetDestination
    }

    $PluginCount = (Get-ChildItem -Path $PluginsDir -Filter "*.dll" -File).Count
}

Write-Host ""
Write-Host "Package ready: $DeployDir"
Write-Host "  Profile : $Profile"
Write-Host "  Build   : $BuildType"
Write-Host "  Plugins : $PluginCount"
