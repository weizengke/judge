
set Path=%Path%;D:\mingw32\bin

mingw32-make -f makefile32

@ECHO off
SET BIN_NAME=JungleOS.exe

setlocal enabledelayedexpansion

SET extension_o=".o"
SET extension_a=".a"
SET extension_dll=".dll"

SET OBJ_DIR=obj
SET LIB_DIR=lib32
SET BIN_DIR=bin32
SET ROOT_DIR=%~dp0

SET OBJ_LIST=
SET LIB_LIST=

echo 进入目录%ROOT_DIR%%OBJ_DIR%
cd %ROOT_DIR%%OBJ_DIR%

FOR %%a IN (*%extension_o%) DO (
    SET OBJ_LIST=!OBJ_LIST! %OBJ_DIR%\%%a
    echo %OBJ_DIR%\%%a
)

echo 进入目录%ROOT_DIR%LIB_DIR%
cd %ROOT_DIR%%LIB_DIR%

FOR %%a IN (*%extension_a%) DO (
    SET LIB_LIST=!LIB_LIST! %LIB_DIR%\%%a
    echo %LIB_DIR%\%%a
)

FOR %%a IN (*%extension_dll%) DO (
    SET LIB_LIST=!LIB_LIST! %LIB_DIR%\%%a
    echo %LIB_DIR%\%%a
)

echo 进入目录%ROOT_DIR%
cd %ROOT_DIR%

@ECHO on
g++.exe -static -o  %BIN_DIR%/%BIN_NAME% -L"%LIB_DIR%" %OBJ_LIST% -l wsock32 %LIB_LIST%
@ECHO off

echo 进入目录%ROOT_DIR%%BIN_DIR%
cd %ROOT_DIR%%BIN_DIR%
echo 启动%BIN_NAME%
%BIN_NAME%

pause