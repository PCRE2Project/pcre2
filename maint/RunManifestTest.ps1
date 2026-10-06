# Script to test a directory listing. We use this to verify that the list of
# files installed by Autoconf, CMake, or Meson matches what we expect.

param (
  [Parameter(Mandatory=$true)]
  [string]$inputDir,

  [Parameter(Mandatory=$true)]
  [string]$manifestName,

  [ValidateSet("autoconf", "cmake", "meson")]
  [string]$producer = "cmake"
)

if ((-not $inputDir) -or (-not $manifestName)) {
  throw "Usage: .\RunManifestTest.ps1 <dir> <manifest name> [<producer>]"
}

$base = [System.IO.Path]::GetFileName($manifestName)

$installedFiles = Get-ChildItem -Recurse -Force -Path $inputDir |
  Sort-Object {[System.BitConverter]::ToString([system.Text.Encoding]::UTF8.GetBytes($_.FullName))} |
  ForEach-Object { $_.Mode.Substring(0,5) + " " + ($_.FullName | Resolve-Path -Relative) }

$null = New-Item -Force $base -Value (($installedFiles | Out-String) -replace "`r`n", "`n")

$expectedFiles = Get-Content -Path $manifestName -Raw
$actualFiles = Get-Content -Path $base -Raw

if ($producer -ne "cmake") {
  $expectedFiles = ($expectedFiles -split "`n" | Where-Object {
    $_ -notmatch '[\\/]lib[\\/]cmake([\\/]pcre2([\\/]pcre2-(config(-version)?|targets(-release)?)\.cmake)?)?\r?$'
  }) -join "`n"
}
if ($producer -ne "autoconf") {
  $expectedFiles = ($expectedFiles -split "`n" | Where-Object {
    $_ -notmatch '[\\/]lib[\\/]libpcre2-(8|16|32|posix)\.la\r?$'
  }) -join "`n"
}

if ($expectedFiles -ne $actualFiles) {
  Write-Host "===Actual==="
  Write-Host $actualFiles
  Write-Host "===End==="

  throw "Installed files differ from expected"
}

Write-Host "Installed files match expected"
Remove-Item -Path $base -Force
