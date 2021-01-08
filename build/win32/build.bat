@echo off
setlocal

call findvs.bat
call config.bat

call %VSDevPath% -no_logo -arch=amd64
msbuild -noLogo -maxCpuCount -consoleLoggerParameters:PerformanceSummary;Summary;ErrorsOnly;ShowCommandLine -property:Configuration=%Configuration% -property:Platform=%Platform% -target:%Target% %Solution%

endlocal
