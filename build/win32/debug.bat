@echo off
setlocal

call findvs.bat
call config.bat

call %VSDevPath% -no_logo -arch=amd64
devenv %Solution%

endlocal
