@echo off

:: Can be either "Debug" or "Release"
set BuildMode=Debug
:: Can be either "x86" or "amd64"
set Architecture=amd64

set Libraries=SDL2main.lib SDL2.lib freetype.lib opengl32.lib dinput8.lib dxguid.lib user32.lib gdi32.lib winmm.lib imm32.lib ole32.lib oleaut32.lib shell32.lib version.lib uuid.lib advapi32.lib setupapi.lib comdlg32.lib shlwapi.lib dbghelp.lib setargv.obj

if %Architecture%==x86 ( set LibArch=x86 )
if %Architecture%==amd64 ( set LibArch=x64 )

set IncludeDirs= -I source\depends\libfreetype\include -I source\depends\libglad -I source\depends\libglm -I source\depends\libgon -I source\depends\libsdl2\include -I source\depends\libstb  -I source\depends\libimgui
set LibraryDirs= -libpath:source\depends\libfreetype\library\%LibArch% -libpath:source\depends\libsdl2\library\%LibArch%

set Defines=-D_CRT_SECURE_NO_WARNINGS -DPLATFORM_WIN32

set CompilerFlags=-Zc:__cplusplus -std:c++17 -nologo -W4 -MT -Oi -EHsc -Z7
set LinkerFlags=-opt:ref -incremental:no -force:multiple

set CompilerWarnings=-wd4100 -wd4505 -wd4189 -wd4201
set LinkerWarnings=-ignore:4099

set ResourceFile=resource\Resources.rc
set ResourcePath=resource\

set InputResource=resource\Resources.res
set InputSource=source\Main.cpp

set OutputPath=binary\win32\
set OutputName=TheEndEditor-%Architecture%

if %BuildMode%==Release (
    set Defines=%Defines% -DNDEBUG
    set LinkerFlags=%LinkerFlags% -subsystem:windows
)
if %BuildMode%==Debug (
    set OutputName=%OutputName%-Debug
    set Defines=%Defines% -DBUILD_DEBUG
    set InputResource=
    set LinkerFlags=%LinkerFlags% -subsystem:console
)

if %Architecture%==x86 (
    set CompilerFlags=%CompilerFlags% -arch:IA32
    set LinkerFlags=%LinkerFlags%,5.1
)

set OutputExecutable=%OutputPath%%OutputName%