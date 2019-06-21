mysqlclient.lib：mysql-5.6.35，x86
libcurl.lib: curl-7.61.1, nmake /f Makefile.vc mode=static VC=10 ENABLE_IDN=no RTLIBCFG=static  MACHINE=x86

 拷贝openssl头文件目录到D:\code\curl-curl-7_61_1\include\openssl
 nmake /f Makefile.vc mode=static VC=10  WITH_SSL=static ENABLE_SSPI=no ENABLE_IPV6=no ENABLE_IDN=no RTLIBCFG=static  MACHINE=x86



openssl:
http://www.cnblogs.com/zzugyl/p/5037152.html
1)安装perl程序
2)
perl Configure VC-WIN32 no-asm --prefix=D:\code\openssl-1.0.2e\builds
ms\do_ms.bat
nmake -f ms\nt.mak
nmake -f ms\nt.mak install