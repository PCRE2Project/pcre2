# Script to test a directory listing. We use this to verify that the list of
# files installed by "make install" or "cmake --install" matches what we expect.

param (
  [Parameter(Mandatory=$true)]
  [string]$inputDir,

  [Parameter(Mandatory=$true)]
  [string]$manifestName,

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
if ($producer -eq "meson") {
  $expectedFiles = $expectedFiles.Replace("pcre2-config-version.cmake", "PCRE2ConfigVersion.cmake")
  $expectedFiles = $expectedFiles.Replace("pcre2-config.cmake", "PCRE2Config.cmake")
  $expectedFiles = (($expectedFiles.TrimEnd() -split "`n") |
    Sort-Object {[System.BitConverter]::ToString([system.Text.Encoding]::UTF8.GetBytes($_.Substring($_.IndexOf(" ") + 1)))} |
    Out-String) -replace "`r`n", "`n"
}
$actualFiles = Get-Content -Path $base -Raw

if ($expectedFiles -ne $actualFiles) {
  Write-Host "===Actual==="
  Write-Host $actualFiles
  Write-Host "===End==="

  throw "Installed files differ from expected"
}

Write-Host "Installed files match expected"
Remove-Item -Path $base -Force
