set CUR_DIR=%cd%

rem 添加程序语言编译环境路径
set PATH_JAVA=%JAVA_HOME%\bin;
set PATH_MINGW=D:\code\online-judge\judger-kernel\build\3part\mingw32\bin

set Path=%PATH_JAVA%;
set Path=%Path%%PATH_MINGW%;

SET BIN_NAME=judger.exe

echo 启动%BIN_NAME%
%BIN_NAME%