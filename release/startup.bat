set CUR_DIR=%cd%

rem 添加程序语言编译环境路径
set PATH_JAVA=C:\Java\jdk1.6.0_10\bin;
set PATH_MINGW=C:\online-judge-compiler\judger_compiler\MinGW\bin;C:\online-judge-compiler\judger_compiler\FPC2.6.4\2.6.4\bin\i386-win32

set Path=%PATH_JAVA%;%PATH_MINGW%;C:\Python\Python38;

SET BIN_NAME=judger.exe

echo 启动%BIN_NAME%
%BIN_NAME%