
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"

nmake -f Makefile.vc

cd bin
call judger.exe

pause