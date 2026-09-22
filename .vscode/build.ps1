param(
    [string]$Configuration = "Debug",
    [string]$Platform = "Win32",
    [string]$Target = "Build"
)

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$msbuild = & $vswhere -latest -prerelease -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1

if (-not $msbuild) {
    Write-Error "MSBuild.exe not found. Is Visual Studio installed?"
    exit 1
}

$solution = Join-Path $PSScriptRoot "..\2DGame\02-Bubble\02-Bubble.sln"

& $msbuild $solution "/t:$Target" "/p:Configuration=$Configuration" "/p:Platform=$Platform" /m /nologo
exit $LASTEXITCODE
