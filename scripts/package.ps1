param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("default", "acme", "beta", "taechang")]
    [string]$Profile,

    [Parameter(Mandatory = $false)]
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Release"
)

$SolutionDir  = Split-Path $PSScriptRoot -Parent
$BuildConfig  = if ($Profile -eq "default") { "${BuildType}_x64" } else { "${BuildType}-${Profile}_x64" }
$SourceDir    = Join-Path $SolutionDir $BuildConfig
$DeployDir    = Join-Path $SolutionDir "deploy\${Profile}"

if (-not (Test-Path $SourceDir)) {
    Write-Error "빌드 출력 폴더가 없습니다: $SourceDir"
    Write-Error "'$BuildConfig' 구성으로 먼저 빌드하세요."
    exit 1
}

if (Test-Path $DeployDir) {
    Remove-Item $DeployDir -Recurse -Force
}
New-Item $DeployDir -ItemType Directory | Out-Null

$Items = @(
    "SageNexus.exe",
    "WebView2Loader.dll",
    "profile.json"
)

foreach ($Item in $Items) {
    $Src = Join-Path $SourceDir $Item
    if (Test-Path $Src) {
        Copy-Item $Src $DeployDir
    } else {
        Write-Warning "누락된 파일: $Item"
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
        Write-Host "  플러그인 복사: $($_.Name)"
    }
}

Write-Host ""
Write-Host "패키징 완료: $DeployDir"
Write-Host "  프로필  : $Profile"
Write-Host "  빌드    : $BuildType"
Write-Host "  플러그인: $(( Get-ChildItem $PluginsDir -Filter '*.dll' ).Count)개"
