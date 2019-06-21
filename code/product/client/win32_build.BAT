set CUR_DIR=%cd%

rem 添加g++编译环境路径
set Path=%CUR_DIR%\..\..\..\build\3part\mingw32\bin;%CUR_DIR%\..\..\..\build\3part\7-Zip

rem 解压工具链
cd %CUR_DIR%\..\..\..\build\3part
if not exist mingw32 7z x mingw32.rar -aoa

@ECHO off
SET BIN_NAME=client.exe

cd %CUR_DIR%

setlocal enabledelayedexpansion

SET BIN_DIR=%CUR_DIR%

@ECHO on
g++.exe  -ansi -fno-asm -Wall -lm -s -std=c++98 -o %BIN_DIR%/%BIN_NAME% client.cpp -l wsock32

rem compile ok...

cd %BIN_DIR%
%BIN_NAME% 1

@ECHO off

pause