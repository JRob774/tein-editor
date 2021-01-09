@echo off
setlocal

call config.bat
pushd ..\..\..\binary\win32
%Executable%
popd

endlocal
