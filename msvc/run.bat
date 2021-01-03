@echo off
setlocal

pushd ..
call msvc\config.bat
pushd %OutputPath%
%OutputName%.exe
popd
popd

endlocal
