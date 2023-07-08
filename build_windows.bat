@echo off

call vcvarsall.bat x64

IF NOT EXIST build mkdir build
pushd build

rem Common flags
set compiler_flags=-WL -FC -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -WX -W4
set compiler_flags=-wd4996 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -GS- %compiler_flags%
set compiler_flags=-I ../include %compiler_flags%

rem Common linker flags
set linker_flags=/DEFAULTLIB:user32 /NODEFAULTLIB:msvcrtd
set linker_flags=../lib/raylib.lib opengl32.lib kernel32.lib shell32.lib gdi32.lib winmm.lib /NODEFAULTLIB:libcmt %linker_flags%


if ["%~1"]==["release"] (call :Release) else (call :Debug)

goto :EOF

:Debug
    set compiler_flags=-Od -Zi -DBUILD_DEBUG=1 %compiler_flags%
    goto :Compile

:Release
    set compiler_flags=-O2 -DBUILD_DEBUG=0 %compiler_flags%
    set linker_flags=/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup %linker_flags%
    goto :Compile

:Compile
    cl %compiler_flags% ../code/game.cpp -link %linker_flags%
    copy /y game.exe ..\run_tree\game.exe

popd
