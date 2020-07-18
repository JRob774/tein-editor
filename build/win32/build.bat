:: Build Process Script

@echo off
setlocal

pushd ..\..

call build\win32\utility\findvsdev.bat
call build\win32\utility\buildvars.bat

                          python tools\manifest.py
if %BuildMode%==Release ( python tools\packgpak.py )

call %VSDevPath% -no_logo -arch=%Architecture%

if not exist binary mkdir binary
pushd binary

if %BuildMode%==Release rc -nologo -i %ResourcePath% %ResourceFile%

call ..\build\win32\utility\buildtime.bat "cl %IncludeDirs% %Defines% %CompilerFlags% %CompilerWarnings% -Fe%OutputExecutable% %InputSource% -link %LinkerFlags% %LinkerWarnings% %LibraryDirs% %Libraries% %InputResource%"

del *.ilk *.res *.obj *.exp *.lib

popd
popd

endlocal
