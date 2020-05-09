@echo off

REM ============================================================================
REM = Visual Studio Command Line Tools Locator Script                          =
REM = Authored by Joshua Robertson                                             =
REM ============================================================================

REM ============================================================================

set VSWhere="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=1* delims=: " %%i in (`%VSWhere% -latest -requires Microsoft.Component.MSBuild`) do (
    if /i "%%i"=="installationPath" set VSDevPath=%%j
)
set VSDevPath="%VSDevPath%\Common7\Tools\vsdevcmd.bat"

REM ============================================================================