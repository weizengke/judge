set CUR_DIR=%cd%

rem 添加程序语言编译环境路径
set PATH_JAVA=C:\Java\jdk1.8.0_201\bin;
set PATH_LUA=C:\online-judge-compiler\judger_compiler\Lua-0.9.8\bin;
set PATH_MINGW=C:\online-judge-compiler\judger_compiler\MinGW\bin;
set PATH_PASCAL=C:\online-judge-compiler\judger_compiler\FPC2.6.4\2.6.4\bin\i386-win32;
set PATH_PYTHON=C:\Users\Administrator\AppData\Local\Programs\Python\Python310;
set Path=%PATH_JAVA%;%PATH_MINGW%;%PATH_PYTHON%;%PATH_LUA%;%PATH_PASCAL%;%SystemRoot%\system32;%SystemRoot%;C:\tools\oj-tools\apache-groovy-sdk-4.0.1\groovy-4.0.1\bin;

SET BIN_NAME=judger.exe

echo 启动%BIN_NAME%
%BIN_NAME%