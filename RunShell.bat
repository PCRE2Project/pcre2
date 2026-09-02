@echo off
setlocal EnableDelayedExpansion

:rem This script is used to run a shell script using either WSL or Git for Windows.
:rem Rather than rewriting our shell scripts for Windows, this allows us to
:rem locate and use an actual bash shell on Windows for testing.

if "%~1"=="" (
  if not defined RUNSHELL_SCRIPT (
    echo RunShell.bat: no script was specified. 1>&2
    exit /b 1
  )
  cd /d "%~dp0"
) else (
  set "RUNSHELL_SCRIPT=%~1"
)

if exist "%SystemRoot%\System32\wsl.exe" (
  for /f "delims=" %%P in ('%SystemRoot%\System32\wsl.exe -- wslpath -u "!RUNSHELL_SCRIPT!"') do set "RUNSHELL_SCRIPT=%%P"
  "%SystemRoot%\System32\wsl.exe" -- sh "!RUNSHELL_SCRIPT!"
  exit /b !ERRORLEVEL!
)

if exist "%ProgramFiles%\Git\bin\bash.exe" (
  set MSYS_NO_PATHCONV=1
  "%ProgramFiles%\Git\bin\bash.exe" "!RUNSHELL_SCRIPT!"
  exit /b !ERRORLEVEL!
)

echo RunShell.bat: WSL or Git for Windows is required. 1>&2
exit /b 1