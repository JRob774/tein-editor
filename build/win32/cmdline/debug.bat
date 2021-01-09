@echo off
setlocal

call findvs.bat
call config.bat

pushd ..
call %VSDevPath% -no_logo -arch=amd64
devenv %Solution%
popd

endlocal
