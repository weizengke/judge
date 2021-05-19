set CUR_DIR=%cd%

@ECHO off
SET BIN_NAME=hook

cd %CUR_DIR%

setlocal enabledelayedexpansion

SET BIN_DIR=%CUR_DIR%

@ECHO on
del /s /q /f %BIN_DIR%\hook.dll
g++.exe -w -ansi -fno-asm -static -shared -std=c++98 -o %BIN_DIR%\hook.dll hook.cpp -Wl,--out-implib,libhook.a  -l wsock32

rem compile ok...

@ECHO off

pause