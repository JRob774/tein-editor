@echo off
setlocal

pushd ..

call msvc\findvs.bat
call msvc\config.bat

call %VSDevPath% -no_logo -arch=%Architecture%

if not exist %OutputPath% mkdir %OutputPath%

if %BuildMode%==Release rc -nologo -i %ResourcePath% %ResourceFile%

call msvc\timer.bat "cl %IncludeDirs% %Defines% %CompilerFlags% %CompilerWarnings% -Fe%OutputExecutable% %InputSource% -link %LinkerFlags% %LinkerWarnings% %LibraryDirs% %Libraries% %InputResource%"

pushd %OutputPath%
if %BuildMode%==Release del %ResourcePath%*.res
del *.ilk *.res *.obj *.exp *.lib
popd
del *.ilk *.res *.obj *.exp *.lib

popd

endlocal
