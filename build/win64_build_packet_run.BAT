
set Path=%Path%;D:\mingw64\bin

mingw32-make -f makefile64

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

g++.exe  -static -o  bin64/JungleOS -L"\lib64" %obj_list% -l wsock32 lib64\libmysql.a lib64\libmysql.dll lib64\psapi.a lib64\psapi.dll lib64\pcre3.a lib64\pcre3.dll lib64\libcurl.a lib64\libcurl.dll


cd bin64
JungleOS.exe

pause