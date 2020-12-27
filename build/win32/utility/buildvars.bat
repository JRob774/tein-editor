:: Build Configuration Variables Script

@echo off

set Libraries=                                                                    ^
SDL2main.lib SDL2.lib freetype.lib libcurl.lib ws2_32.lib wldap32.lib crypt32.lib ^
opengl32.lib dinput8.lib dxguid.lib user32.lib gdi32.lib winmm.lib imm32.lib      ^
ole32.lib oleaut32.lib shell32.lib version.lib uuid.lib advapi32.lib setupapi.lib ^
comdlg32.lib shlwapi.lib dbghelp.lib setargv.obj

:: Select the build mode, options are: "Debug" and "Release"
set BuildMode=Debug
:: Select the architecture, options are: "x86" and "amd64"
set Architecture=amd64

if %Architecture%==x86   ( set LibArch=x86 )
if %Architecture%==amd64 ( set LibArch=x64 )

set IncludeDirs=-I ..\source\depends\libcurl\include -I ..\source\depends\libfreetype\include -I ..\source\depends\libglad -I ..\source\depends\libglm -I ..\source\depends\libgon -I ..\source\depends\libjson -I ..\source\depends\libsdl2\include -I ..\source\depends\libstb
set LibraryDirs=-libpath:..\source\depends\libcurl\library\%LibArch% -libpath:..\source\depends\libfreetype\library\%LibArch% -libpath:..\source\depends\libsdl2\library\%LibArch%

set Defines=

set CompilerFlags=-Zc:__cplusplus -std:c++17 -nologo -W4 -MT -Oi -EHsc -Z7
set LinkerFlags=-opt:ref -incremental:no

set CompilerWarnings=-wd4100 -wd4505 -wd4189 -wd4201
set LinkerWarnings=-ignore:4099

set ResourceFile=..\resource\resources.rc
set ResourcePath=..\resource\

set InputResource=..\resource\resources.res
set InputSource=..\source\main.cpp

set OutputExecutable=TheEndEditor-%Architecture%

if %BuildMode%==Release (
    set LinkerFlags=%LinkerFlags% -subsystem:windows
)
if %BuildMode%==Debug (
    set OutputExecutable=%OutputExecutable%-Debug
    set Defines=%Defines% -DBUILD_DEBUG
    set InputResource=
    set LinkerFlags=%LinkerFlags% -subsystem:console
)

if %Architecture%==x86 (
    set CompilerFlags=%CompilerFlags% -arch:IA32
    set LinkerFlags=%LinkerFlags%,5.1
)
