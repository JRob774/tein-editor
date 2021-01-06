@echo off
setlocal

pushd ..
call msvc\findvs.bat
call msvc\config.bat
call %VSDevPath% -no_logo -arch=%Architecture%
devenv %OutputExecutable%.exe
popd

endlocal
