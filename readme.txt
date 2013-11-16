Project site:   http://weizengke.com/archives/143

编译环境：Windows 8.1  64位，MinGW64 。

链接所需的.a文件需要有相应DLL生成，如：
pexports pcre3.dll > pcre3.def
dlltool –input-def pcre3.def –dllname pcre3.dll –output-lib pcre3.a -k


usefull site:

pexports:
http://202.97.199.19/1/ishare.down.sina.com.cn/21841154.zip?ssig=Cm3GzOiS6u&Expires=1384531200&KID=sina,ishare&fn=Pexports-0.43.zip

pcre:
http://www.airesoft.co.uk/pcre
http://www.airesoft.co.uk/files/pcre/pcre-8.33.zip

curl:
http://curl.haxx.se/gknw.net/7.33.0/dist-w64/curl-7.33.0-devel-mingw64.7z

MinGW:
http://jaist.dl.sourceforge.net/project/mingwbuilds/mingw-builds-install/mingw-builds-install.exe


