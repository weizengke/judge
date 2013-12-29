# -*- coding: utf-8 -*-
# call as python hdu_vjudge.py <param>
# param : the follow fomart supported."
#    1: submit pid lang username password source-path return-file-path
#    2: status pid lang username return-file-path
#    3: ce rid return-file-path
#note: file path don't include space
#weizengke 2013-4-5

#!/usr/bin/python
import urllib
import urllib2
import cookielib
import time
import os
import re
import os.path
import shutil
import sys

#username = 'username'
#password = 'password'
#pid = '1000'
#lang = 1
#source = 'code'

cj=cookielib.CookieJar()

FALSE = 0
TRUE  = 1

# file save path
filename = 'c:/hdu-file.tmp'

#login
def login(username,password):
    url='http://acm.hdu.edu.cn/userloginex.php?action=login'
    values={'username':username,'userpass':password,'login':'Sign+In'}
    data = urllib.urlencode(values)
    headers ={"User-agent":"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1"}
    req = urllib2.Request(url, data,headers)
    opener=urllib2.build_opener(urllib2.HTTPCookieProcessor(cj))
    response =opener.open(req)
    html_content =response.read()

    sErrorString = 'No such user or wrong password.'
    pos = html_content.find(sErrorString)

    if pos > 0:
        print 'login faild...'
        return FALSE
    else:
        print 'login success...'
        return TRUE

#submit
def submit(pid,lang,source_path):

    source = ''
    f=file(source_path)
    # if no mode is specified, 'r'ead mode is assumed by default
    while True:
        line = f.readline()
        if len(line) == 0: # Zero length indicates EOF
            break
        #print line,
        source += line
        # Notice comma to avoid automatic newline added by Python
    f.close()
    # close the file

    #print source

    #incase that less than 50 byte
    source += str('                                                 ');

    url='http://acm.hdu.edu.cn/submit.php?action=submit'
	values={'check':'0','problemid':pid,'language':lang,'usercode':source}
	data = urllib.urlencode(values)
	headers ={"User-agent":"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1"}
	req = urllib2.Request(url, data,headers)
	opener=urllib2.build_opener(urllib2.HTTPCookieProcessor(cj))
	response =opener.open(req)
	html_content =response.read()

    sErrorString = 'One or more following ERROR(s) occurred.'
    pos = html_content.find(sErrorString)

    if pos > 0:
        print 'Submit faild'
        return FALSE
    else:
        print 'Submit success...'
        return TRUE

#status
def status(pid,username,lang):
    print 'start to get status...'
    url='http://acm.hdu.edu.cn/status.php?first=&pid='+ str(pid) +'&user='+ str(username) +'&lang=' + str(lang)+ '&status=0'
    data = urllib.urlopen(url)
    html_content =data.read()

    f = open(filename,'w')
    f.write(html_content)
    f.close()

    print 'save status_content to ',filename

#conpime_error
def getCompileError(runid):

    # 不关注是否获取成功，file.txt 的内容由调用者判断
    url='http://acm.hdu.edu.cn/viewerror.php?rid=' + str(runid)
    data = urllib.urlopen(url)
    html_content =data.read()

    f = open(filename,'w')
    f.write(html_content)
    f.close()

    print 'save compile_error to ',filename

#dispatch
def dispatch(opt):
    print 'dispatch'

if __name__=="__main__":

    # <opt> <problemId> <languageId> <username> <password> <source-path>
    # submit pid lang username password source-path
    # status pid lang username
    # ce rid
    while 1:

        count = len(sys.argv)
        print 'Argv-num :',count

        if count <= 1:
            print 'No argv, so break.'
            break

        print type(sys.argv)
        print str(sys.argv)

        #for a in range(1, len(sys.argv)):
        #    print sys.argv[a]

        if sys.argv[1] == 'submit':
            print 'Do submit...'

            pid = sys.argv[2]
            lang = sys.argv[3]
            username = sys.argv[4]
            password = sys.argv[5]
            source_path = sys.argv[6]
            filename = sys.argv[7]

            print pid,lang,username,password,source_path,filename

            login(username,password)
            submit(pid,lang,source_path)
            break

        if sys.argv[1] == 'status':
            print 'Do status...'
            pid = sys.argv[2]
            lang = sys.argv[3]
            username = sys.argv[4]
            filename = sys.argv[5]

            print pid,lang,username,filename

            status(pid,username,lang)
            break

        if sys.argv[1] == 'ce':
            print 'Do ce...'
            rid = sys.argv[2]
            filename = sys.argv[3]

            print rid,filename

            getCompileError(rid)
            break
        else:
            print "Error: Command line option syntax error, the follow fomart supported."
            print " 1: submit pid lang username password source-path return-file-path"
            print " 2: status pid lang username return-file-path"
            print " 3: ce rid return-file-path"
            break


