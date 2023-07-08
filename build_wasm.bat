@echo off

IF NOT EXIST build mkdir build

set name=gmtk23

set compiler_flags=-Os -Wall -Wno-implicit-const-int-float-conversion -Wno-missing-braces -Wno-switch -DPLATFORM_WEB
set compiler_flags=--preload-file assets %compiler_flags%
set compiler_flags=-I. -Iinclude %compiler_flags%
set linker_flags=-L. -Llib/libraylib.a

if ["%~1"]==["release"] (call :Release) else (call :Debug)

goto :EOF


:Debug
    set compiler_flags=--shell-file code/shell_debug.html %compiler_flags%
    goto :Compile


:Release
    set compiler_flags=--shell-file code/shell.html %compiler_flags%
    goto :Compile


:Compile
    call emcc -o %name%.html code/game.cpp lib/libraylib.a -s USE_GLFW=3 -s ASYNCIFY %compiler_flags%

    pushd build
    IF NOT EXIST wasm mkdir wasm
    popd

    move "%name%.html" build/wasm/index.html"
    move "%name%.js" build/wasm/%name%.js"
    move "%name%.wasm" build/wasm/%name%.wasm"
    move "%name%.data" build/wasm/%name%.data"
