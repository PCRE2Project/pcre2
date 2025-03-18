# Script to test that all the symbols of a DLL are as expected.

param (
    [Parameter(Mandatory=$true)]
    [string]$inputDir,

    [Parameter(Mandatory=$true)]
    [string]$manifestDir
)

if ((-not $inputDir) -or (-not $manifestDir)) {
  throw "Usage: .\RunSymbolTest.ps1 <dll_dir> <manifest_dir>"
}

$dllNames = @("pcre2-8", "pcre2-16", "pcre2-32", "pcre2-posix")

foreach ($dllName in $dllNames) {
    $expectedFile = Join-Path $manifestDir ("manifest-lib$dllName.so")
    $dllFile = Join-Path $inputDir ("$dllName.dll")
    $base = [System.IO.Path]::GetFileName($expectedFile)

    # Get path to dumpbin using vswhere
    $vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    $dumpbin = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find VC\Tools\MSVC\*\bin\Hostx64\x64\dumpbin.exe | Select-Object -First 1

    $actualSymbols = & $dumpbin /exports $dllFile |
        ForEach-Object {
            if ($_ -match '^\s*\d+\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+(\S+)') {
                "T $($matches[1])"
            }
        } |
        Where-Object {
            $_ -match '^T '
        } |
        Sort-Object

    $actualOutput = ($actualSymbols -join "`n") + "`n"
    $null = New-Item -Force $base -Value $actualOutput

    $expectedSymbols = (Get-Content -Path $expectedFile -Raw).TrimEnd("`n") |
        ForEach-Object { $_ -replace '@@.*', '' }
    $expectedOutput = ($expectedSymbols -join "`n") + "`n"

    if ($expectedOutput -ne $actualOutput) {
        Write-Host "Shared object contents for $dllFile differ from expected"
        Write-Host "===Actual==="
        Write-Host $actualOutput
        Write-Host "===End==="
        throw "Symbol test failed"
    } else {
        Write-Host "Shared object contents for $dllFile match expected"
    }

    Remove-Item -Path $base -Force
}
