@echo

set Path=%Path%;D:\mingw32\bin

mingw32-make -f makefile32

@ECHO off
setlocal enabledelayedexpansion
SET extension=".o"
SET OBJ_DIR=obj
SET root_path=%~dp0
SET obj_list=

echo 进入目录%~dp0%OBJ_DIR%
cd %~dp0%OBJ_DIR%

FOR %%a IN (*%extension%) DO (
    SET obj_list=!obj_list! %OBJ_DIR%\%%a
    echo %OBJ_DIR%\%%a
)


echo 进入目录%root_path%
cd %root_path%

g++.exe -static -o  bin32/JungleOS -L"\lib32" %obj_list% -l wsock32 lib32\libmysql.a lib32\libmysql.dll lib32\psapi.a lib32\psapi.dll lib32\pcre.a lib32\pcre.dll lib32\curllib.a lib32\curllib.dll

cd bin32
JungleOS.exe

pause