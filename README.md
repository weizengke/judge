[![Build Status](https://travis-ci.com/weizengke/judge.svg?branch=master)](https://travis-ci.com/weizengke/judge)

# 最好的跨平台Online Judge

## Table of contents
* [安装或者开发指导](#安装或者开发指导)
* [演示地址](#演示地址)
* [平台简介](#平台简介)
* [支持特性](#支持特性)
* [目录说明](#目录说明)
* [部署指导](#部署指导)
  * [运行环境](#运行环境)
  * [部署网站](#部署网站)
  * [部署判题核心](#部署判题核心)
* [高级配置](#高级配置)
  * [熟悉命令行](#熟悉命令行)
  * [Judger基础配置](#judger基础配置)
  * [Virtual Judge配置](#virtual-judge配置)
  * [Telnet配置](#telnet配置)
  * [FTP配置](#FTP配置)
  * [日常维护命令](#日常维护命令)
* [Linux下部署](#Linux下部署)  
* [特别鸣谢](#特别鸣谢)  

## 安装或者开发指导：
XXXX

## 演示地址： 
http://happyoj.com

HappyOJ当前支持的程序语言，如需添加其他语言或者版本，可在此留言~
| 语言 | 编译环境 |
|----|------|
| C  | GNU C 4.5.2|
| C++   | GNU C++ 4.5.2    |
| Java |  Java 1.8    |
| Pascal   | Pascal 2.6.4   |
|  Python  | Python 3.10 |
|  Ruby | Ruby 3.1.1    |
|  Perl | Perl 5.32.1.1|
|  Lua | Lua 0.9.8 |
|  Tcl | Tcl 8.6.7.0  |
|  Pike | Pike 7.8.352  |
|  GO| GO 1.18.1  |
|  JS| nodejs 16.14.2 |
|  Groovy | Groovy 1.8.4 |
|  Rust | Rust 1.60.0 |
|  Kotlin | Kotlin 1.1.2 |
|  R| R 4.1.3|

## 平台简介
没错，我们支持Windows平台，也支持Linux平台（Linux下当前仅支持Virtual Judge）。<br>
Online Judge平台系统（简称OJ）是一个B/S架构的源程序判题系统。用户可以在线提交多种程序（如C、C++、Java）源代码，系统对源代码进行编译和执行，并通过预先设计的测试用例来检验程序源代码的正误。
用户可以在Online Judge系统上练习编程，参加竞赛，与其他用户讨论交流，提高自己的编程水平，并可以用于数据结构、程序设计教学的实验和考试。

## 支持特性:
1. Online Judge Kernel支持所有可命令行编译和运行的程序语言；支持按语言分布式部署；支持动态添加程序语言。
2. Online Judge Kernel支持Virtual Judge：http://acm.hdu.edu.cn，http://leetcode-cn.com 。
3. Online Judge Kernel支持命令行管理，命令行特性支持命令联想、自动补全等功能，极大方便了系统的管理。（这是我们的特色）
4. Online Judge Kernel支持ACM和OI模式的判题，可以通过命令行judge mode acm(oi)灵活切换，默认为ACM模式；
5. Online Judge Kernel支持作为Telnet服务器、客户端，远程连接管理。
6. Online Judge Kernel支持作为FTP服务器，提供FTP服务。 
7. Online Judge Kernel支持API Hook安全防护。 
8. Online Judge Web Platform支持ACM和CodeJam两种竞赛模式。
9. Online Judge Web Platform支持ELO rating system,更好的给参赛选手评估水平
10. Online Judge Kernel支持采集知名OJ的最近比赛信息（[Codeforces](http://codeforces.com/contests)、[ZOJ](http://acm.zju.edu.cn/onlinejudge/showContests.do)、[HDU](http://acm.hdu.edu.cn/contests/contest_list.php)、[AtCoder](http://atcoder.jp)、[HihoCoder](https://hihocoder.com/contests)，持续添加）。每天定时采集，开放json给大家：[http://happyoj.com/otheroj.json](http://happyoj.com/otheroj.json)

## 目录说明：
1. judger-kernel：OJ的判题核心代码，C语言
2. judger-web-platform：OJ的Web端代码，java语言
3. judger_sql：OJ的数据库文件

## 部署指导
### 运行环境
1. 操作系统：Windows XP/7/8/8.1/10， Windows Server 2003/2008/2012

2. JAVA JDK：Java JDK1.8（推荐）<br>
    OJ系统对JDK安装位置没有要求，直接安装完成。以安装D:\Java\jdk1.8.0_201为例。<br>
    然后添加环境变量：<br>
```
    JAVA_HOME=D:\Java\jdk1.8.0_201
    path=%JAVA_HOME%\bin
```
3. Web容器：Apache Tomcat 8(推荐)；安装目录：D:\tomcat8<br>

4. 数据库：MySQL 5.1.55(推荐)<br>
   字符集选择GBK，端口选择默认的3306，设置用户名root，密码rootpwd<br>
   然后创建gdoj命名的数据库，并执行online-judge\judger_sql目录下的sql_import_struct.bat一键导入gdoj_struct.sql<br>

5. 编译环境：GCC、JAVA、PASCAL、Python、Ruby等编程语言编译环境<br>
   可下载https://gitee.com/jungle/online-judge-compiler后，运行env.bat一键配置GCC、JAVA、PASCAL,但要注意修改JAVA_HOME的正确路径: <br>
```
     set JAVA_HOME=D:\Java\jdk1.8.0_201
     setx JAVA_HOME "D:\Java\jdk1.8.0_201" /m
```
### 部署网站
- 删除目录D:\tomcat8\webapps\ROOT里的文件，然后直接拷贝judger-web-platform/gdoj/WebRoot目录里的文件到ROOT。
如：![输入图片说明](https://gitee.com/uploads/images/2017/1206/211620_7e973ace_6154.jpeg "web-download-ok.jpg")
- 根据需要配置D:\tomcat8\webapps\ROOT\WEB-INF\classes\config.properties
```
OJ_WEB=D\:\\tomcat8\\webapps\\ROOT\\
OJ_PATH=D\:\\OJ\\
OJ_DATA_PATH=D\:\\OJ\\data\\
OJ_TMP=D\:\\OJ\\temp
OJ_JUDGE_LOG=D\:\\OJ\\logfile\\judge_log\\        ---> 必须配置正确，否则web端无法查看判题日志
OJ_CONFIG_FILE_PATH=D\:\\OJ\\conf\\config.json    ---> 必须配置正确，否则web无法获取支持的编译语言
OJ_JUDGER_IP=127.0.0.1   ------> 必须配置正确，用于WEB与OJ-Kernel通信的IP
OJ_JUDGER_PORT=5000      ------> 必须配置正确，用于WEB与OJ-Kernel通信的端口号，必须与OJ-Kernel的配置一致
```
- 配置D:\tomcat8\webapps\ROOT\WEB-INF\classes\applicationContext.xml中mysql数据库的密码
```
<property name="username" value="root"></property>
<property name="password" value="rootpwd"></property>
```
- 配置D:\tomcat8\webapps\ROOT\WEB-INF\classes\hibernate.cfg.xml中mysql数据库的密码
```
	<property name="connection.username">root</property>
	<property name="connection.url">jdbc:mysql://localhost:3306/gdoj</property>
	<property name="dialect">org.hibernate.dialect.MySQLDialect</property>
	<property name="myeclipse.connection.profile">localhost</property>
	<property name="connection.password">rootpwd</property>
	<property name="connection.driver_class">com.mysql.jdbc.Driver</property>
```
- 进入 http://域名/admin，登录管理员界面，进行题目添加/比赛创建/用户权限管理等
  详细可参见文档：[https://gitee.com/jungle/online-judge/blob/master/judger-web-platform/doc/软件使用说明书.doc](https://gitee.com/jungle/online-judge/blob/master/judger-web-platform/doc/%E8%BD%AF%E4%BB%B6%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E%E4%B9%A6.doc)

### 部署判题核心
- 创建目录D:\OJ，并拷贝online-judge/judger-kernel/release到OJ目录下，如：![输入图片说明](https://gitee.com/uploads/images/2017/1206/212651_0331a9c6_6154.jpeg "OJ-ls.jpg")

- 预配置OJ启动配置文件D:\OJ\conf\config.json, 配置sock_port和mysql账号密码, 配置支持的编译语言languages
```
    "system": {
        "startup_config": "config.cfg",
        "sysname": "Judge-Kernel",
        "JudgePath": "judger.exe",
        "sock_port": 5000    -----> 系统本地Socket监听的端口号，需要保持与config.properties中的一致
    },

    "mysql": {
        "url": "localhost",
        "username": "root",
        "password": "rootpwd",
        "table": "gdoj",
        "port": 3306
    },

    "languages": [
        { 
            "id": 1, 
            "language_name": "MS 2010 C++",  --> 编译语言别名，可现实与web上语言下拉框
            "Transcoding": 1,
            "TimeLimit": 1,
            "ProcessLimit": 1,            
            "SourcePath": "%PATH%%SUBPATH%%NAME%.%EXT%",
            "ExePath": "%PATH%%SUBPATH%%NAME%.%EXE%",   
            "RunCmd": "%PATH%%SUBPATH%%NAME%.%EXE%",
            "JudgeIP": "127.0.0.1",
            "disable": 1      --> 禁用此语言
        }, 
        { 
            "id": 2, 
            "language_name": "MS 2010 C",
            "Transcoding": 1,
            "disable": 1
        }, 
        {
            "id": 3, 
            "language_name": "GNU C++ 4.5.2",
            "LanguageExt": "cc",
            "LanguageExe": "exe",
            "CompileCmd": "g++ -ansi -fno-asm -Wall -Wl,--stack=67108864 -lm -s -static -std=c++98  -DONLINE_JUDGE -o %PATH%%SUBPATH%%NAME% %PATH%%SUBPATH%%NAME%.%EXT%  2>%PATH%%SUBPATH%%NAME%.txt"
        }, 
        { 
            "id": 4, 
            "language_name": "GNU C 4.5.2",
            "LanguageExt": "c",
            "LanguageExe": "exe",         
            "CompileCmd": "gcc -ansi -fno-asm -Wall -Wl,--stack=67108864 -lm -s -static -std=c99  -DONLINE_JUDGE -o %PATH%%SUBPATH%%NAME% %PATH%%SUBPATH%%NAME%.%EXT%  2>%PATH%%SUBPATH%%NAME%.txt"
        }, 
        { 
            "id": 5, 
            "language_name": "Java 1.8",
            "TimeLimit": 2,
            "LanguageExt": "java",
            "LanguageExe": "class",
            "SourcePath": "%PATH%%SUBPATH%Main.%EXT%",
            "ExePath": "%PATH%%SUBPATH%Main.%EXE%",              
            "CompileCmd": "javac %PATH%%SUBPATH%Main.%EXT% 2>%PATH%%SUBPATH%%NAME%.txt",
            "RunCmd": "java Main"
        }
    ],

    "hdu_languages": [
        { 
            "id": 3,                  --> 此语言对应HDU上的ID
            "language_name": "C++",
            "local_language_id": 3    --> 对应本地编译的语言id。可不填，则表示不支持
        },
        { 
            "id": 4,             
            "language_name": "C",
            "local_language_id": 4
        },
        { 
            "id": 5,
            "language_name": "Java",
            "local_language_id": 5
        }
    ],

    "cf_languages": [
        { 
            "id": 54,                           --> 此语言对应codeforces上的ID
            "language_name": "GNU G++17 7.3.0",
            "local_language_id": 3
        },
        { 
            "id": 43,             
            "language_name": "GNU GCC C11 5.1.0",
            "local_language_id": 4
        },
        { 
            "id": 36,             
            "language_name": "Java 1.8.0_241",
            "local_language_id": 5
        },
        { 
            "id": 7,             
            "language_name": "Python 2.7.18",
            "local_language_id": 9
        },
        { 
            "id": 31,             
            "language_name": "Python 3.8.10",
            "local_language_id": 27
        },
        { 
            "id": 67,             
            "language_name": "Ruby 3.0.0",
            "local_language_id": 10
        }
    ]
```
- 运行judge.exe启动判题核心程序
```
启动后界面如下，并通过display current-configuration查看当前配置
Command APP RegistInfo ok...
Debug APP RegistInfo ok...
AAA APP init ok...
AAA APP RegistInfo ok...
Telnet APP init ok...
Telnet APP RegistInfo ok...
Connect MySQL(localhost, root, xxxxxxx, gdoj, 3306) ok...
Judge APP init ok...
Judge APP RegistInfo ok...
NDP APP init ok...
NDP APP RegistInfo ok...
FTPS APP init ok...
FTPS APP RegistInfo ok...
Sysmng APP running ok...
Command APP running ok...
Debug APP running ok...
AAA APP running ok...
Telnet APP running ok...
Judge APP running ok...
NDP APP running ok...
FTPS APP running ok...
Recover configuration begin.
Eecover configuration end.
Press any key to continue.

Huawei_judger>display  current-configuration include-default
#version 1.1.173
sysname Huawei_judger
#
telnet server enable
telnet authentication-mode aaa
#
ftp server enable
#
aaa
 local-user root password Root@123
 local-user root service-type telnet ftp
#
judge enable
virtual-judge enable
#
contests-collect enable
contests-collect interval 30
contests-collect save-file D:\tomcat8\webapps\ROOT\otheroj.json
#
judge-mgr
 mode acm
 undo auto-detect enable
 auto-detect interval 10
 security enable
 ignore extra-space enable
 testcase-path data
#
virtual-judge-mgr
 hdu-judge enable
 hdu-judge username root password Root@123
 undo hdu-judge remote-judge enable
#
return
#
Huawei_judger>
```

## 高级配置
### 熟悉命令行
  用户通过命令行对系统下发各种命令来实现对系统的配置与日常维护操作。
* 不完整命令字输入<br>
  系统支持不完整命令字输入，即在当前视图下，当输入的字符能够匹配唯一的命令字时，可以不必输入完整的命令字。该功能提供了一种快捷的输入方式，有助于提高操作效率。
```
#如输入dis us和完整输入display users都能执行命令获取当前登录系统的用户
Judge-Kernel]dis us
   #   Type     Delay        Network Address   Socket  Username
 ---------------------------------------------------------------------
 + 0   Console  00:00:00     127.0.0.1         -       -
   1   Telnet   00:25:42     127.0.0.1         588
Judge-Kernel]display users
   #   Type     Delay        Network Address   Socket  Username
 ---------------------------------------------------------------------
 + 0   Console  00:00:00     127.0.0.1         -       -
   1   Telnet   00:25:45     127.0.0.1         588
Judge-Kernel]
```
* Tab键自动补全命令字<br>
  输入不完整的命令字后按下Tab键，系统将自动补全命令字。<br>
  如果TAB自动补全能匹配到多个命令字，循环TAB时将循环显示所有的命令字。<br>
```
Judge-Kernel>display us  
#该状态下按下TAB                          
Judge-Kernel>display users 
```
* 命令行在线帮助<br>
  在线帮助是指在任何时候任何视图下，用户可以通过输入“?”来获取在线帮助。<br>
  用户复杂的命令行过于繁琐而难以记忆时使用，以便完成命令行的正确输入。<br>
```
Judge-Kernel>?
 debugging                Debugging switch
 display                  Display
 quit                     Quit from the current system view
 return                   Return to the user view
 system-view              Enter the system view
 terminal                 Terminal
 undo                     Undo operation
Judge-Kernel>dis?
 display                  Display
Judge-Kernel>dis
Judge-Kernel>display ?
 command-tree             Command tree
 current-configuration    Current Configuration
 debugging                Debugging switch
 history                  Histrory command
 judge                    Judge of OJ
 ndp                      NDP
 save-configuration       Save-configuration
 users                    Users
 <cr>
Judge-Kernel>display us?
 users                    Users
Judge-Kernel>display us
Judge-Kernel>display users ?
 <cr>
Judge-Kernel>                                                                                                       
```
* 进入命令行视图
1. 用户视图<br>
  用户从终端成功登录至系统即进入用户视图，在屏幕上显示：
```
Judge-kernel>
``` 
  在用户视图下，用户可以完成查看运行状态和统计信息等功能。

2. 系统视图<br>
  在用户视图下，输入命令system-view后回车，进入系统视图。<br>
  在系统视图下，用户可以配置系统参数以及通过该视图进入其他的功能配置视图。
```
Judge-Kernel>system-view  
Judge-Kernel]           
Judge-Kernel]?
 aaa                      Authentication Authorization Accounting
 diagnose-view            Enter the daignose view
 display                  Display
 hdu-judge                HDU-Judge
 judge                    Judge of OJ
 mysql                    Mysql
 ndp                      NDP
 quit                     Quit from the current system view
 reboot                   Reboot Judge kernel
 return                   Return to the user view
 save                     Save configuration
 sysname                  Set system name
 telnet                   Telnet Protocol
 undo                     Undo operation
 virtual-judge            Virtual judge
Judge-Kernel]
```

3. 诊断视图<br>
  在系统视图下，输入命令diagnose-view后回车，进入诊断视图。<br>
  在诊断视图下，用户可以执行一些诊断命令，以及一些高级配置，建议在熟悉系统的工程师使用。
```
Judge-Kernel]diagnose-view                                                       
Judge-Kernel-diagnose]  
Judge-Kernel-diagnose]?                                       
 display                  Display                            
 quit                     Quit from the current system view    
 return                   Return to the user view                     
 set                      Set value                         
 version                  Show version of solfware             
Judge-Kernel-diagnose]     
```
4. 退出视图<br>
  通过输入quit命令，可以退出当前视图，并回退到上一层视图。<br>
  也可以通过return命令，直接回退到用户视图
```
#通过quit回退到上一层视图
Judge-Kernel-diagnose]quit
Judge-Kernel]
#直接通过return惠推倒用户视图
Judge-Kernel-diagnose]return
Judge-Kernel>
```

### Judger基础配置
* judge enable<br>
  1）命令行功能：<br>
     **judge enable** 命令用于使能judge功能<br>
     **undo judge enable** 命令用于去使能judge功能<br>
     缺省情况下，judge功能处于使能状态<br>
  2）视图：<br>
     系统视图<br>
  3）使用举例<br>
```
#使能judge功能
Judge-Kernel>system-view 
Judge-Kernel]judge enable
```
```
#去使能judge功能
Judge-Kernel>system-view 
Judge-Kernel]undo judge enable
```
* judge-mgr<br>
  1）命令行功能：<br>
     **judge-mgr** 命令用于进入judge-mgr视图，以便配置本地judger相关配置<br>
  2）视图：<br>
     系统视图<br>
  3）使用举例<br>
```
#进入judge-mgr视图
judger>system-view
judger]judge-mgr
judger-judge-mgr]
```

* mode<br>
  1）命令行功能：<br>
     **mode acm** 命令用于切换judger为acm模式<br>
     **mode oi** 命令用于切换judger为oi模式<br>
     缺省情况下，judger处于acm模式<br>
  2）视图：<br>
     judge-mgr视图<br>
  3）使用举例<br>
```
#切换judger为acm模式
judger>system-view
judger]judge-mgr
judger-judge-mgr]
judger-judge-mgr]mode acm
```
```
#切换judger为oi模式
judger>system-view
judger]judge-mgr
judger-judge-mgr]
judger-judge-mgr]mode oi
```

* security enable<br>
  安全防护（API HOOK）可以保护OJ不被恶意提交的程序破坏，如关机、关闭进程等操作。<br>
  1）命令行功能：<br>
     **security enable** 命令用于使能安全防护（API HOOK）<br>
     **undo security enable** 命令用于去使能安全防护（API HOOK）<br>
     缺省情况下，judger安全防护（API HOOK）是使能的<br>
     注意：当前该功能并不成熟，如果出现误判，在可以保证不出现攻击的情况下，可以先选择关闭，然后联系我们解决。
  2）视图：<br>
     judge-mgr视图<br>
  3）使用举例<br>
```
#使能安全防护（API HOOK）
judger>system-view
judger]judge-mgr
judger-judge-mgr]
judger-judge-mgr]security enable
```
```
#去使能安全防护（API HOOK）
judger>system-view
judger]judge-mgr
judger-judge-mgr]
judger-judge-mgr]undo security enable
judger-judge-mgr]display this
#
judge-mgr
 undo security enable
 testcase-path D:\OJ\data\
#
return
#
judger-judge-mgr]
```
* auto-detect enable<br>
  自动检测用于自动发现未完成判题的提交，避免因设备故障导致的部分提交未完成能够及时发现并判题。<br>
  1）命令行功能：<br>
     **auto-detect enable** 命令用于使能自动检测<br>
     **undo auto-detect enable** 命令用于去使能自动检测<br>
     缺省情况下，judger自动检测是去使能的<br>
  2）视图：<br>
     judge-mgr视图<br>
  3）使用举例<br>
```
#使能自动检测
judger>system-view
judger]judge-mgr
judger-judge-mgr]
judger-judge-mgr]auto-detect enable
```

* testcase-path<br>
  1）命令行功能：<br>
     **testcase-path STRING<1-256>** 命令用于配置judger的测试用例目录<br>
  2）视图：<br>
     judge-mgr视图<br>
  3）使用举例<br>
```
#配置judger的测试用例目录为D:\OJ\data\
judger>system-view
judger]judge-mgr
judger-judge-mgr]
judger-judge-mgr]testcase-path D:\OJ\data\
judger-judge-mgr]display this
#
judge-mgr
 testcase-path D:\OJ\data\
#
return
#
judger-judge-mgr]
```
* ignore extra-space enable<br>
  1）命令行功能：<br>
     **ignore extra-space enable** 命令用于配置judger的判题时忽略多余的空格换行，以便不报格式错误<br>
  2）视图：<br>
     judge-mgr视图<br>
  3）使用举例<br>
```
#配置judger使能忽略多余空格换行
judger>system-view
judger]judge-mgr
judger-judge-mgr]
judger-judge-mgr]ignore extra-space enable
judger-judge-mgr]display this
#
judge-mgr
 ignore extra-space enable
#
return
#
judger-judge-mgr]
```
### Virtual Judge配置
* virtual-judge enable<br>
  当前仅支持hdu的虚拟判题。<br>
  1）命令行功能：<br>
     **virtual-judge enable** 命令用于使能虚拟判题功能<br>
     **undo virtual-judge enable** 命令用于去使能虚拟判题功能<br>
     缺省情况下，虚拟判题功能处于去使能状态<br>
  2）视图：<br>
     系统视图<br>
  3）使用举例<br>
```
#使能虚拟判题功能
Judge-Kernel>system-view 
Judge-Kernel]virtual-judge enable
```
```
#去使能虚拟判题功能
Judge-Kernel>system-view 
Judge-Kernel]undo virtual-judge enable
```

* hdu-judge enable<br>
  1）命令行功能：<br>
     **hdu-judge enable** 命令用于使能HDU虚拟判题功能<br>
     **undo hdu-judge enable** 命令用于去使能HDU虚拟判题功能<br>
     缺省情况下，HDU虚拟判题功能处于去使能状态<br>
  2）视图：<br>
     系统视图<br>
  3）使用举例<br>
```
#使能HDU虚拟判题功能
Judge-Kernel>system-view 
Judge-Kernel]hdu-judge enable
```
```
#去使能HDU虚拟判题功能
Judge-Kernel>system-view 
Judge-Kernel]undo hdu-judge enable
```

* virtual-judge-mgr<br>
  1）命令行功能：<br>
     **virtual-judge-mgr** 命令用于进入virtual-judge-mgr视图，以便配置virtual judger相关配置<br>
  2）视图：<br>
     系统视图<br>
  3）使用举例<br>
```
#进入virtual-judge-mgr视图
judger>system-view
judger]virtual-judge-mgr
judger-vjudge-mgr]
```

* hdu-judge username password<br>
  1）命令行功能：<br>
     **hdu-judge username STRING<1-24> password STRING<1-24>** 命令用于配置HDU虚拟判题下，在HDU平台上的账号<br>
  2）视图：<br>
     virtual-judge-mgr视图<br>
  3）使用举例<br>
```
#配置HDU虚拟判题在HDU平台上的账号和密码为root/root@123
judger>system-view
judger]virtual-judge-mgr
judger-vjudge-mgr]hdu-judge username root password root@123
```

* hdu-judge remote-judge enable<br>
  使能HDU虚拟判题远端裁判功能后，HDU的虚拟判题将在分布式部署的其他OJ系统执行相关判题。
  1）命令行功能：<br>
     **hdu-judge remote-judge enable** 命令用于使能HDU虚拟判题远端裁判功能<br>
     **undo hdu-judge remote-judge enable** 命令用于去使能HDU虚拟判题远端裁判功能<br>
     缺省情况下，HDU虚拟判题远端裁判功能处于去使能状态。<br>
     注意还需要配置远端OJ服务器的IP和端口号（ **hdu-judge ip STRING<1-24> port INTEGER<1-65535>** ）。<br>
  2）视图：<br>
     virtual-judge-mgr视图<br>
  3）使用举例<br>
```
#使能HDU虚拟判题远端裁判功能
judger>system-view
judger]virtual-judge-mgr
judger-vjudge-mgr]hdu-judge remote-judge enable
```
```
#去使能HDU虚拟判题远端裁判功能
judger>system-view
judger]virtual-judge-mgr
judger-vjudge-mgr]undo hdu-judge remote-judge enable
```

* hdu-judge ip port<br>
  1）命令行功能：<br>
     **hdu-judge ip STRING<1-24> port INTEGER<1-65535>** 命令用于配置远端OJ服务器的IP和端口号<br>
  2）视图：<br>
     virtual-judge-mgr视图<br>
  3）使用举例<br>
```
#配置分布式远端hdu-judger的OJ服务器IP和端口号
judger>system-view
judger]virtual-judge-mgr
judger-vjudge-mgr]hdu-judge ip 192.168.1.2 port 5001
```

### Telnet配置
* telnet server enable<br>
  使能Telnet服务器功能后，才能通过telnet协议远程登陆管理系统。<br>
  1）命令行功能：<br>
     **telnet server enable** 命令用于使能Telnet服务器<br>
     **undo telnet server enable** 命令用于去使能Telnet服务器<br>
     缺省情况下，Telnet服务器处于去使能状态<br>
  2）视图：<br>
     系统视图<br>
  3）使用举例<br>
```
#使能Telnet服务器
Judge-Kernel>system-view 
Judge-Kernel]telnet server enable
```
```
#去使能Telnet服务器
Judge-Kernel>system-view 
Judge-Kernel]undo telnet server enable
```
* telnet authentication-mode<br>
  1）命令行功能：<br>
     **telnet authentication-mode none** 命令用于配置Telnet服务器的认证方式为不认证，即telnet登陆后不需要输入账号和密码。<br>
     **telnet authentication-mode password** 命令用于配置Telnet服务器的认证方式为密码认证，即telnet登陆后需要输入账号和密码。<br>
     **telnet authentication-mode aaa** 命令用于配置Telnet服务器的认证方式为AAA认证，即telnet登陆后需要输入AAA账号和密码。<br>
     缺省情况下，Telnet服务器的认证方式为不认证<br>
     Telnet服务器的认证方式多次配置以最后一次为准。<br>
  2）视图：<br>
     系统视图<br>
  3）使用举例<br>
```
#配置Telnet服务器的认证方式为不认证
Judge-Kernel>system-view 
Judge-Kernel]telnet authentication-mode none
```
```
#配置Telnet服务器的认证方式为密码认证，并创建telnet账号和密码
Judge-Kernel>system-view 
Judge-Kernel]telnet authentication-mode password
Info: Please create telnet username and password.
Judge-Kernel]
Judge-Kernel]telnet username admin password admin@123
```
```
#配置Telnet服务器的认证方式为AAA认证，并创建一个aaa用户并且服务类型为telnet
Judge-Kernel>system-view 
Judge-Kernel]telnet authentication-mode aaa
Info: Please create AAA username and password.                                                                                      
Judge-Kernel]
Judge-Kernel]aaa
Judge-Kernel-aaa]local-user root password Root@123 service-type telnet
```
### FTP配置
* ftp server enable<br>
  使能ftp 服务器功能后，才能通过ftp协议传输下载文件。<br>
  需要同时创建一个服务类型为ftp的AAA用户，才能连接<br>
  1）命令行功能：<br>
     **ftp server enable** 命令用于使能ftp服务器<br>
     **undo ftp server enable** 命令用于去使能ftp服务器<br>
     缺省情况下，ftp服务器处于去使能状态<br>
  2）视图：<br>
     系统视图<br>
  3）使用举例<br>
```
#使能ftp服务器
Judge-Kernel>system-view 
Judge-Kernel]ftp server enable
```
```
#去使能ftp服务器
Judge-Kernel>system-view 
Judge-Kernel]undo ftp server enable
```
```
#创建一个aaa用户并且服务类型为ftp
Judge-Kernel>system-view 
Judge-Kernel]telnet authentication-mode aaa
Info: Please create AAA username and password.                                                                                      
Judge-Kernel]
Judge-Kernel]aaa
Judge-Kernel-aaa]local-user ftp password Root@123 service-type ftp
```
### 日常维护命令
* display judge brief<br>
  1）命令行功能：<br>
     **display judge brief** 命令用于查看当前judger的基础配置。<br>
  2）视图：<br>
     NA<br>
  3）使用举例<br>
```
#查看当前judger的基础配置
Judge-Kernel>display judge brief
# Local Judger Info
  Global Judge Is Enable
  Sysname   : Judge-Kernel
  Sock Port : 5000
  Judge Mode: ACM
  Data Path : D:\OJ\data\
  Last Judge: 1970-01-01 08:00:00
 ==========================================================================
# Virtual Judger Info
  Global Virtual Judge Is Enable
  HDU domain: http://acm.hdu.edu.cn
  Judger | Account | Password | Status | Remote |   Judger-IP   | J-Port
  -------------------------------------------------------------------------
  HDU     weizengke  weizengke  Enable   Disable  192.168.1.2     5001
 ==========================================================================
# MySQL Info
  URL       : localhost
  Username  : root
  Password  : rootpwd
  Table-Name: gdoj
  Port      : 3306
 ==========================================================================
Judge-Kernel>
```

* sysname<br>
  1）命令行功能：<br>
     **sysname STRING<1-24> ** 命令用于更改系统名称，最长24个字符<br>
  2）视图：<br>
     系统视图<br>
  3）使用举例<br>
```
#更改系统名称为judger
Judge-Kernel>system-view 
Judge-Kernel]sysname judger                                        
Info: system name change to judger successful.                  
judger]

```
* save<br>
  1）命令行功能：<br>
     **save** 命令用于保存系统配置，以便重启后配置任然生效。<br>
  2）视图：<br>
     系统视图<br>
  3）使用举例<br>
```
Judge-Kernel>system-view 
Judge-Kernel]save      
Info: Save configuration successfully.    
Judge-Kernel]                                                                        
```

* display current-configuration<br>
  1）命令行功能：<br>
     **display current-configuration** 命令用于查看当前配置<br>
  2）视图：<br>
     任意视图<br>
  3）使用举例<br>
```
Judge-Kernel>display  current-configuration
judger]display  current-configuration
#version V100R001C00B090
sysname judger
#
telnet server enable
telnet authentication-mode aaa
#
aaa
 local-user admin password Root@123 service-type telnet
#
judge-mgr
 testcase-path D:\OJ\data\
#
virtual-judge-mgr
 hdu-judge username weizengke password weizengke
 hdu-judge ip 127.0.0.1 port 5000
#
return
#
judger]
```

* display save-configuration<br>
  1）命令行功能：<br>
     **display save-configuration** 命令用于查看当前保存的配置文件中的配置<br>
  2）视图：<br>
     任意视图<br>
  3）使用举例<br>
```
Judge-Kernel>display save-configuration
#version V100R001C00B090
sysname judger
#
telnet server enable
#
aaa
 local-user admin password Root@123 service-type telnet
#
judge-mgr
 mode oi
 testcase-path D:\OJ\data\
#
virtual-judge-mgr
 hdu-judge username weizengke password weizengke
 hdu-judge ip 127.0.0.1 port 5000
#
return
#
judger]
```

* display users<br>
  1）命令行功能：<br>
     **display users** 命令用于查看当前登录到系统的用户<br>
  2）视图：<br>
     任意视图<br>
  3）使用举例<br>
```
Judge-Kernel]display users                                                        
   #   Type     Delay        Network Address   Socket  Username                       
 ---------------------------------------------------------------------                     
   0   Console  00:01:05     127.0.0.1         -       -                        
 + 1   Telnet   00:00:00     127.0.0.1         588                         
Judge-Kernel] 
#带+号的为当前用户。
```
* display history <br>
  1）命令行功能：<br>
     **display history [ INTEGER<1-100> ]** 命令用于查看当前用户的历史执行命令，可以指定需要显示的条数，最大100条<br>
  2）视图：<br>
     任意视图<br>
  3）使用举例<br>
```
Judge-Kernel>display history 10
display current-configuration
judge enable
system-view
Judge-Kernel>
```
## Linux下部署： 
目前Linux下除了本地判题，Kernel其他特性都支持，文档暂没有时间写。主要就是在bin目录下直接./vos.o启动kernel。

## 特别鸣谢
感谢[jetbrains的open source license](https://jb.gg/OpenSource)<br>
![输入图片说明](https://images.gitee.com/uploads/images/2021/0910/220841_a1cd33bc_6154.png "屏幕截图.png")

