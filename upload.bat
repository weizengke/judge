@Echo Off
echo open 139.159.160.103>> ftp.txt
echo ci>> ftp.txt
echo online-judge-ci>> ftp.txt
echo put C:\Users\travis\build\weizengke\judge\build\VSBuild\bin\Release\judger.exe>> ftp.txt
echo bye>> ftp.txt
FTP -s:ftp.txt
del ftp.txt /q