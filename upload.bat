@Echo Off
Echo open 139.159.160.103 >ftp.up
Echo w00583872>>ftp.up
Echo wzk!1989>>ftp.up
Echo Cd .\ >>ftp.up
Echo binary>>ftp.up
Echo put "C:\Users\travis\build\weizengke\judge\build\VSBuild\bin\Release\judger.exe">>ftp.up
Echo bye>>ftp.up
FTP -s:ftp.up
del ftp.up /q