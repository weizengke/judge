set CUR_DIR=%cd%

rem 添加g++编译环境路径
set Path=%CUR_DIR%\..\..\..\build\3part\mingw32\bin;%CUR_DIR%\..\..\..\build\3part\7-Zip

rem 解压工具链
cd %CUR_DIR%\..\..\..\build\3part
if not exist mingw32 7z x mingw32.rar -aoa

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