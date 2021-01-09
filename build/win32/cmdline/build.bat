@echo off
setlocal

call findvs.bat
call config.bat

pushd ..
call %VSDevPath% -no_logo -arch=amd64
msbuild -noLogo -maxCpuCount -consoleLoggerParameters:Summary;ShowCommandLine -property:Configuration=%Configuration% -property:Platform=%Platform% -target:%Target% %Solution%
popd ..

endlocal
