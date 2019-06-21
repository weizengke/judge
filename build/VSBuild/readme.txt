该目录下支持vs2010 编译环境，编译方法为：

编译release版本：
D:\online-judge\judger-kernel\build\VSBuild>nmake -f Makefile.vc

编译debug版本：
D:\online-judge\judger-kernel\build\VSBuild>nmake -f Makefile.vc DBG=1

或者直接运行build.bat,需要先配置好vcvarsall.bat的路径

编译结果在bin目录judger.exe