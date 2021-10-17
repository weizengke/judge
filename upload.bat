@Echo Off
Echo open 139.159.160.103 >ftp.up
Echo %1>>ftp.up
Echo %2>>ftp.up
Echo put "C:\Users\travis\build\weizengke\judge\build\VSBuild\bin\Release\judger.exe">>ftp.up
Echo bye>>ftp.up
FTP -s:ftp.up
del ftp.up /q
