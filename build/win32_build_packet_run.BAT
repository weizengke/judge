set CUR_DIR=%cd%

rem 添加g++编译环境路径
set Path=%Path%;%CUR_DIR%\3part\mingw32\bin;%CUR_DIR%\3part\7-Zip

rem 解压工具链
cd %CUR_DIR%\3part
if not exist mingw32 7z x mingw32.rar -aoa

cd %CUR_DIR%
rem -j32并行编译，加快编译速度
mingw32-make -j32 -f makefile32

@ECHO off
SET BIN_NAME=judge.exe

setlocal enabledelayedexpansion

SET extension_o=".o"
SET extension_a=".a"
SET extension_lib=".lib"
SET extension_dll=".dll"

SET OBJ_DIR=obj
SET LIB_DIR=lib32
SET BIN_DIR=bin32
SET ROOT_DIR=%~dp0

SET OBJ_LIST=
SET LIB_LIST=
SET LIB_LIST2=

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

FOR %%a IN (*%extension_lib%) DO (
    SET LIB_LIST2=!LIB_LIST2! %LIB_DIR%\%%a
    echo %LIB_DIR%\%%a
)

FOR %%a IN (*%extension_dll%) DO (
    SET LIB_LIST=!LIB_LIST! %LIB_DIR%\%%a
    echo %LIB_DIR%\%%a
)

echo 进入目录%ROOT_DIR%
cd %ROOT_DIR%

@ECHO on
g++.exe -static -s -o  %BIN_DIR%/%BIN_NAME% -L%LIB_DIR% %OBJ_LIST% -lwsock32 -lcurl -lwldap32 -lws2_32 %LIB_LIST% %LIB_LIST2%
@ECHO off

echo 进入目录%ROOT_DIR%%BIN_DIR%
cd %ROOT_DIR%%BIN_DIR%
echo 启动%BIN_NAME%
%BIN_NAME%

pause