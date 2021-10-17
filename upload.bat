@Echo Off
echo open %1>> ftp.txt
echo %2>> ftp.txt
echo %3>> ftp.txt
echo put build\VSBuild\bin\Release\judger.exe>> ftp.txt
echo bye>> ftp.txt
FTP -s:ftp.txt
del ftp.txt /q