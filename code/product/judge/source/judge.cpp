#include <windows.h>
#include <process.h>
#include <iostream>
#include <conio.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#include <queue>
#include <string>
#include <sstream>

#include "tlhelp32.h"

/* BEGIN: Added by weizengke, 2013/11/15 for debug center*/
#include "..\..\include\pdt_common_inc.h"
/* END:   Added by weizengke, 2013/11/15 */


#include "..\include\psapi.h"
#include "..\include\judge_util.h"
#include "..\include\define.h"

 /* 必须在mysql.h之前，因为里面用到socket  #include <winsock2.h> ,windows.h貌似已经包含 */
#include "..\..\thirdpart\mysql\include\mysql.h"

using namespace std;

#pragma  comment(lib,"F:\cmd-sys\build\lib\psapi.lib")

//#pragma comment(lib,"ws2_32")
#pragma comment(lib, "F:\cmd-sys\build\lib\libmysql.lib")

#define VOS_YES 1
#define VOS_NO  0

#define JUDGE_VIRTUAL VOS_NO   /* 关闭VJUDGE */

#define BUFSIZE 4096

const int MAX_NAME = 30;
const int MAX_CODE = 100000;
const int MAX_TITLE = 200;
const int MAX_CONTENT = 100000;
const int MAX_WAITTIME = 10000 ;


MYSQL *mysql;     //mysql连接
char query[1024]; //查询语句
char INI_filename[]="GDOJ\\data.ini";
char Mysql_url[255];
char Mysql_username[255];
char Mysql_password[255];
char Mysql_table[255];
int  Mysql_port;
char Mysql_Character[255];  //编码

int  nLanguageCount=0;
int isDeleteTemp=0;
int isRestrictedFunction=0;
int  limitJudge=20;  //裁判队列最大等待数量
DWORD OutputLimit=10000; //最大输出
char workPath[MAX_PATH];  //临时工作目录
char judgeLogPath[MAX_PATH];
int JUDGE_LOG_BUF_SIZE = 200;
char dataPath[MAX_PATH];  //数据
char logPath[MAX_PATH]="log\\";  //log
char judgePath[MAX_PATH]; //judge.exe

char judge_log_filename[MAX_PATH] = {0};

int isTranscoding=0;   //针对VS的转码
int limitIndex=1;    //时间限制倍数
int nProcessLimit=1;
char GL_username[MAX_NAME];
int GL_solutionId;
int GL_problemId;
int GL_languageId;
int GL_verdictId;
int GL_contestId;
int GL_time;
int GL_memory;
int GL_reJudge;
int GL_testcase;
time_t GL_submitDate;
char GL_languageName[100]={0};
char GL_languageExt[10]={0};
char GL_languageExe[10]={0};
int GL_time_limit;
int GL_memory_limit;
int GL_spj;

char compileCmd_str[1024]={0};
char runCmd_str[1024]={0};

char sourcePath[1024]={0};
char exePath[1024]={0};

char inFileName[MAX_PATH];
char outFileName[MAX_PATH];
char DebugFile[MAX_PATH];
char ErrorFile[MAX_PATH];

clock_t startt,endt ; //每次run的时间点

STARTUPINFO si;
PROCESS_INFORMATION G_pi = {0};

class CSolution
{
public:
	char username[MAX_NAME];
	int solutionId;
	int problemId;
	int languageId;
	int verdictId;
	int contestId;
	int time;
	int memory;
	int reJudge;
	time_t submitDate;
protected:
private:
};


////////////// VJUDGE
#include "..\include\pcre.h"
//#pragma comment(lib, "lib/pcre.lib")

#define MAX_SIZE_BUF 10000000

#define DEBUG_PRINT(X)   X

#define UCHAR unsigned char
#define ULONG unsigned long
#define CHAR char

#define BOOL_TRUE 0
#define BOOL_FALSE 1

#define OVECCOUNT 30    /* should be a multiple of 3 */

#define MAX_LANG_SIZE 255

enum ENUM_PROVLEM
{
	PROBLEM_TIME = 0,
	PROBLEM_MEMORY,
	PROBLEM_TITLE,
	PROBLEM_DESCRIPTION,
	PROBLEM_INPUT,
	PROBLEM_OUTPUT,
	PROBLEM_SAMPLE_INPUT,
	PROBLEM_SAMPLE_OUTPUT,
	PROBLEM_AUTHOR,

	PROBLEM_TAG_MAX
};

string g_problem_string[PROBLEM_TAG_MAX];
char tmps[MAX_SIZE_BUF];

/* hdu language list */
UCHAR gaucLanguageName[][MAX_LANG_SIZE] = {
	"G++",
	"GCC",
	"C++",
	"C",
	"Pascal",
	"Java"
};
string GL_source;
int GL_vjudge;
int GL_vpid;

#define UCHAR unsigned char
#define ULONG unsigned long
#define CHAR char

#define BOOL_TRUE 0
#define BOOL_FALSE 1

/* HDU VJUDGE */

char hdu_username[1000]="weizengke";
char hdu_password[1000]="269574524";

char tfilename[1000]="tmpfile.txt";

#define DEBUG_OFF 0
#define DEBUG_ON 1

ULONG g_oj_debug_switch = DEBUG_OFF;

#define DEBUG (g_oj_debug_switch == DEBUG_ON)?(1):(0)


void set_debug_switch(ULONG ds)
{
	g_oj_debug_switch = ds;
}

void MSG_OUPUT_DBG(const char *fmt, ...)
{
	va_list ap;
	char buffer[4096];
	time_t  timep = time(NULL);
	int l;
	struct tm *p;

	if (DEBUG_OFF == DEBUG)
	{
		return;
	}

    p = localtime(&timep);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;

	printf("%04d-%02d-%02d %02d:%02d:%02d ",p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

	va_start(ap, fmt);
	l = vsprintf(buffer, fmt, ap);

	printf("%s\n", buffer);
	va_end(ap);

}


int read_buffer(const char *filename, char * buffer, int buf_size)
{
	if (NULL == filename || NULL == buffer)
	{
		return 0;
	}
	FILE * fp=fopen(filename,"r");
	char tmp[4096] = {0};
	int size_tmp = 0;
	char *buf = NULL;

	if (fp == NULL)
	{
		return 0;
	}

	buf = buffer;
	while(fgets(tmp,4096 ,fp))
	{

		buf += sprintf(buf,"%s",tmp);
		if (strlen(buffer) >= buf_size - 5)
		{
			sprintf(&buffer[buf_size - 5],"...");
			break;
		}
	}

	while (strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n')
	{
		buffer[strlen(buffer) - 1] = '\0';
	}

	fclose(fp);
	return 0;
}

int reset_file(const char *filename)
{
	if (NULL == filename)
	{
		return 0;
	}
	FILE *fp = fopen(filename, "w");

	if (fp == NULL)
	{
		return 0;
	}

	fprintf(fp,"");

	fclose(fp);

	return 0;
}

int write_buffer(const char *filename, const char *fmt, ...)
{
	va_list ap;
	int l;
	if (NULL == filename)
	{
		return 0;
	}

	FILE *fp = fopen(filename, "a+");
	char buffer[40960] = {0};
	if (fp == NULL)
	{
		return 0;
	}

	va_start(ap, fmt);
	l = vsprintf(buffer, fmt, ap);
	fprintf(fp, "%s", buffer);
	va_end(ap);

	fclose(fp);

	return 0;
}

ULONG getLanguageNameByID(ULONG id, UCHAR *ucLanguageName)
{
	if (id < 0 || id >= sizeof(gaucLanguageName)/MAX_LANG_SIZE)
	{
		return BOOL_FALSE;
	}

	strcpy((char *)ucLanguageName, (char *)gaucLanguageName[id]);
	return BOOL_TRUE;
}

ULONG getLanguageIDByName(UCHAR *ucLanguageName, ULONG *id)
{
	USHORT usLoop = 0;

	for (usLoop = 0; usLoop <= sizeof(gaucLanguageName)/MAX_LANG_SIZE; ++usLoop)
	{
		if (strcmp((CHAR*)ucLanguageName, (CHAR*)gaucLanguageName[usLoop]) == 0)
		{
			*id = usLoop;
			return BOOL_TRUE;
		}
	}

	return BOOL_FALSE;
}

bool isSpace(char c)
{
	if(c==' '||c=='\n'||c=='\t')
	{
		return true;
	}
	return false;
}

#if(JUDGE_VIRTUAL == VOS_YES)
char dec2hexChar(short int n)
{
	if ( 0 <= n && n <= 9 ) return char( short('0') + n );
	else if ( 10 <= n && n <= 15 )return char( short('A') + n - 10 );
	else return char(0);
}
short int hexChar2dec(char c)
{
	if ( '0'<=c && c<='9' ) return short(c-'0');
	else if ( 'a'<=c && c<='f' ) return ( short(c-'a') + 10 );
	else if ( 'A'<=c && c<='F' ) return ( short(c-'A') + 10 );
	else return -1;
}

string escapeURL(const string &URL)
{
	string result = "";
	for ( unsigned int i=0; i<URL.size(); i++ )
	{
		char c = URL[i];
		if (
			( '0'<=c && c<='9' ) ||
			( 'a'<=c && c<='z' ) ||
			( 'A'<=c && c<='Z' ) ||
			c=='/' || c=='.'
			) result += c;
		else {
			int j = (short int)c;
			if ( j < 0 ) j += 256;
			int i1, i0;
			i1 = j / 16;
			i0 = j - i1*16;
			result += '%';
			result += dec2hexChar(i1);
			result += dec2hexChar(i0);
		}
	}
	return result;
}

string deescapeURL(const string &URL)
{
	string result = "";
	for ( unsigned int i=0; i<URL.size(); i++ )
	{
		char c = URL[i];
		if ( c != '%' ) result += c;
		else {
			char c1 = URL[++i];
			char c0 = URL[++i];
			int num = 0;
			num += hexChar2dec(c1) * 16 + hexChar2dec(c0);
			result += char(num);
		}
	}
	return result;
}



string getAllFromFile(char *filename)
{
    string res="";
    FILE * fp=fopen(filename,"r");
    while (fgets(tmps,1000000,fp)) res+=tmps;
    fclose(fp);
    return res;
}

size_t process_data(void *buffer, size_t size, size_t nmemb, void *user_p)
{
	FILE *fp = (FILE *)user_p;
	size_t return_size = fwrite(buffer, size, nmemb, fp);
	//cout << (char *)buffer << endl;
	return return_size;
}

#if 0
ULONG login()
{
    FILE * fp=fopen(tfilename,"w+");
	CURL *curl;
	CURLcode res;

    curl = curl_easy_init();

	MSG_OUPUT_DBG("Do login...");

	if(curl)
	{
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_data);
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "hdu.cookie");
        curl_easy_setopt(curl, CURLOPT_URL, "http://acm.hdu.edu.cn/userloginex.php?action=login");
        string post=(string)"username="+hdu_username+"&userpass="+hdu_password+"&login=Sign+In";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());

		res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
    }

	fclose(fp);

    if (res) return BOOL_FALSE;

    string ts=getAllFromFile(tfilename);
    if (ts.find("No such user or wrong password.")!=string::npos)
	{
		MSG_OUPUT_DBG("Login failed.");
		return BOOL_FALSE;
	}
    return BOOL_TRUE;
}

ULONG getSubmitError(char *filename, string &res)
{
	string ts;
	res = "";
    FILE * fp=fopen(filename,"r");
	int begin_ = 0;
	int end_ = 0;
    while (fgets(tmps,1000000,fp))
    {
        ts=tmps;
        if (ts.find("<form id=\"submit\" name=\"submit\"")!=string::npos)
        {
            while (fgets(tmps,1000000,fp))
			{
                ts=tmps;
				begin_ = ts.find("<span>");
                if (begin_!=string::npos)
				{
					//cout<<"Sorry! FOUND SUBMIT_INFO"<<endl;
					end_ = ts.find("</span>");
					if (end_ !=string::npos)
					{
						begin_ += 6;
						res = ts.substr(begin_,end_ - begin_);
						//cout<<res<<endl;
						fclose(fp);
						return BOOL_TRUE;
					}

					while (fgets(tmps,1000000,fp))
					{
						ts=tmps;
						end_ = ts.find("</span>");
						if (end_ !=string::npos)
						{
							begin_ += 6;
							res = ts.substr(begin_,end_ - begin_);
							//cout<<res<<endl;
							fclose(fp);
							return BOOL_TRUE;
						}
						else
						{
							res=res+ts;
						}
					}
					break;
				}
			}
            break;
        }
    }
    fclose(fp);
    return BOOL_FALSE;
}

ULONG submit(string pid, string lang, string source)
{
	CURL *curl;
	CURLcode res;
	FILE * fp=fopen(tfilename,"w+");
	if (NULL == fp)
	{
		MSG_OUPUT_DBG("Open %s failed...", tfilename);
	}

    curl = curl_easy_init();

	headerlist=NULL;
	static const char buf[] = "Expect:";
	headerlist = curl_slist_append(headerlist, buf);

	MSG_OUPUT_DBG("Do submit...");
	MSG_OUPUT_DBG("Problem:%s, Language:%s ....", pid.c_str(), lang.c_str());

	if (source.length() <= 50)
	{
		for (int i =0;i <= 50 - source.length() + 50; i++)
		{
			source += " \r\n";
		}
	}

    if(curl)
	{
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_data);
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "hdu.cookie");
        curl_easy_setopt(curl, CURLOPT_URL, "http://acm.hdu.edu.cn/submit.php?action=submit");

		string post= (string)"check=0&problemid=" + pid + "&language=" + lang + "&usercode=" + escapeURL(source);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

	curl_slist_free_all (headerlist);

	fclose(fp);

	if (res)
	{
		MSG_OUPUT_DBG("curl_easy_perform failed...");
		return BOOL_FALSE;
	}

    string tss=getAllFromFile(tfilename);
    if (tss.find("Connect(0) to MySQL Server failed.")!=string::npos||tss.find("<b>One or more following ERROR(s) occurred.")!=string::npos||tss.find("<h2>The requested URL could not be retrieved</h2>")!=string::npos||tss.find("PHP: Maximum execution time of")!=string::npos)
	{
		MSG_OUPUT_DBG("One or more ERROR(s) occurred.....");
		return BOOL_FALSE;
	}

	MSG_OUPUT_DBG("Submit success...");

    return BOOL_TRUE;
}
#endif
ULONG getResult(string s, string &res)
{
    int pos=s.find("<font color=");
	if (-1 == pos)
	{
		return BOOL_FALSE;
	}

    while (s[pos]!='>') pos++;
    pos++;

    int st=pos;
    while (s[pos]!='<') pos++;
    res = s.substr(st,pos-st);

	return BOOL_TRUE;
}

ULONG getRunid(string s, string &res) {
    int pos=s.find("<td height=22px>");
	if (-1 == pos)
	{
		return BOOL_FALSE;
	}

    while (s[pos]!='>') pos++;
    pos++;

    int st=pos;
    while (s[pos]!='<') pos++;

    res = s.substr(st,pos-st);

	return BOOL_TRUE;
}


string getCEinfo_brief(char *filename)
{
	string res="",ts;
    FILE * fp=fopen(filename,"r");

    while (fgets(tmps,1000000,fp))
    {
        ts=tmps;
        if (ts.find("View Compilation Error")!=string::npos)
        {
            while (fgets(tmps,1000000,fp))
			{
                ts=tmps;
				int pos = ts.find("<pre>");
                if (pos !=string::npos)
				{
					res = ts.substr(pos + 5, ts.length() - pos - 5);

					while (fgets(tmps,1000000,fp))
					{
						ts=tmps;
						if (ts.find("</pre>")!=string::npos)
						{
							MSG_OUPUT_DBG("FOUND CE_INFO");
							break;
						}
						else
						{
							res=res+ts;
						}
					}
					break;
				}
			}
            break;
        }
    }
    fclose(fp);
    return res;
}
#if 0
string getCEinfo(string runid)
{
	FILE *fp = fopen(tfilename, "ab+");
	CURL *curl;
	CURLcode res;

    curl = curl_easy_init();
    if(curl)
    {
		curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "hdu.cookie");
        string url=(string)"http://acm.hdu.edu.cn/viewerror.php?rid="+runid;
        //cout<<url;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    fclose(fp);

    string info = getCEinfo_brief(tfilename);
    return info;
}
#endif

ULONG getUsedTime(string s, string &timeuse)
{
    int pos=s.find("MS</td>");
	if (-1 == pos)
	{
		return BOOL_FALSE;
	}

    int st=pos;
    while (s[pos]!='>') pos--;
    pos++;

    timeuse =  s.substr(pos,st-pos);
	return BOOL_TRUE;
}

ULONG getUsedMem(string s, string &memuse)
{
	int pos=s.find("K</td>");
	if (-1 == pos)
	{
		return BOOL_FALSE;
	}

	int st=pos;
	while (s[pos]!='>') pos--;
	pos++;
	memuse = s.substr(pos,st-pos);
	return BOOL_TRUE;
}

string getLineFromFile(char *filename,int line)
{
    string res="";
    FILE * fp=fopen(filename,"r");
    int cnt=0;
    while (fgets(tmps,10000000,fp))
	{
        cnt++;
        res=tmps;
        if (res.find("<h1>Realtime Status</h1>")!=string::npos)
		{
            fgets(tmps,10000000,fp);
            res=res+tmps;
            fgets(tmps,10000000,fp);
            res=res+tmps;
            break;
        }
    }
    fclose(fp);
    return res;
}
#if 0
ULONG getStatus(string hdu_username, string pid,string lang, string &runid, string &result,string& ce_info,string &tu,string &mu)
{
    ULONG ulRet = BOOL_TRUE;
    tu=mu="0";
    string ts;

	MSG_OUPUT_DBG("Do get status...");

	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();

    if ( curl )
	{
		FILE *fp = fopen(tfilename, "ab+");
		curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "hdu.cookie");
		char url[255] = {0};
		sprintf(url, "http://acm.hdu.edu.cn/status.php?first=&pid=%s&user=%s&lang=&status=0", pid.c_str(), hdu_username.c_str());

		//MSG_OUPUT_DBG(url);

		curl_easy_setopt( curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

		res = curl_easy_perform( curl );
		curl_easy_cleanup(curl);

		fclose(fp);
	}

	ts = getLineFromFile(tfilename,77);

	if(BOOL_FALSE == getUsedTime(ts, tu))
	{
		++ulRet;
		MSG_OUPUT_DBG("getUsedTime failed.");
	}

    if(BOOL_FALSE == getUsedMem(ts, mu))
	{
		++ulRet;
		MSG_OUPUT_DBG("getUsedMem failed.");
	}

	if(BOOL_FALSE == getRunid(ts, runid))
	{
		++ulRet;
		MSG_OUPUT_DBG("getRunid failed.");
	}

	if(BOOL_FALSE == getResult(ts, result))
	{
		++ulRet;
		MSG_OUPUT_DBG("getResult failed.");
	}

	if (BOOL_TRUE != ulRet)
	{
		MSG_OUPUT_DBG("get record failed.");
		return BOOL_FALSE;
	}

	MSG_OUPUT_DBG("problem:%s, language:%s, verdict:%s, submissionID:%s, time:%s ms, memory:%s kb\r\n", pid.c_str(), lang.c_str(), result.c_str(), runid.c_str(), tu.c_str(), mu.c_str());

	MSG_OUPUT_DBG("get status success...");

	if (result.find("Compilation Error")!=string::npos)
	{
		//获取编译错误信息
		string CE_Info = getCEinfo(runid);
		ce_info = CE_Info;
		//MSG_OUPUT_DBG(CE_Info.c_str());
	}

    return BOOL_TRUE;
}
#endif

////////////////////////////////////////
//spider
////////////////////////////////////////

#define PCRE_STATIC // 静态库编译选项

ULONG isNeed2HTML(ENUM_PROVLEM em)
{
	switch (em)
	{
		case PROBLEM_TIME:
		case PROBLEM_MEMORY:
		case PROBLEM_TITLE:
		case PROBLEM_AUTHOR:
			return BOOL_FALSE;
		default:
			return BOOL_TRUE;
	}
	return BOOL_TRUE;
}

#if 0
void SQL_updateProblemInfo(string v_ojname, string v_pid)
{
	string val_str="";
	/*
	val_str = g_problem_string[0] + "," + g_problem_string[1]+ "," + "'" + g_problem_string[2] + "'" + "," +
	"'" + g_problem_string[3] + "'" + "," + "'" + g_problem_string[4] + "'" + "," + "'" + g_problem_string[5] + "'" + "," +
	"'" + g_problem_string[6] + "'" + "," + "'" + g_problem_string[7] + "'" + "," + "'" + g_problem_string[8] + "'" + "," +
	"'" + getCurrentTime() + "', 'N', 0,0,0,0,0,1, '" + v_ojname +"', " + v_pid + "";
    */

	MSG_OUPUT_DBG("In SQL_updateProblemInfo, (%s)", v_pid.c_str());

	for(int i=0; i<PROBLEM_TAG_MAX; i++)
	{

		//char *end;
		//char *string_ = (char*)malloc(sizeof(char)*g_problem_string[i].length()+1);

		//strcpy(string_,g_problem_string[i].c_str());
		/*
		end = string_;
		end += strlen(string_);                //point sql tail
		//convert NUL(ASCII 0)、'\n'、'\r'、'\'’、'''、'"'和Control-Z and so on
		*end++ = '\'';
		end += mysql_real_escape_string(mysql, end, query, strlen(string_));
		*end++ = '\"';
	    *end++ = ')';

		cout<<string_<<endl;
		*/

		if (i == PROBLEM_TITLE)
		{
			replace_all_distinct(g_problem_string[i], "\"", " ");
			g_problem_string[i] = "HDU." + v_pid + " - " + g_problem_string[i];
		}

		if (BOOL_TRUE == isNeed2HTML((ENUM_PROVLEM)i))
		{
			replace_all_distinct(g_problem_string[i], "\"", "&quot;");
			replace_all_distinct(g_problem_string[i], "src=/data/images/", "src=http://acm.hdu.edu.cn/data/images/");
			replace_all_distinct(g_problem_string[i], "src=../../data/images/", "src=http://acm.hdu.edu.cn/data/images/");
			replace_all_distinct(g_problem_string[i], "\n", "<br>");
		}
		//val_str += g_problem_string[i];
	}

	val_str = g_problem_string[0] + "," + g_problem_string[1]+ "," + "\"" + g_problem_string[2] + "\"" + "," +
		"\"" + g_problem_string[3] + "\"" + "," + "\"" + g_problem_string[4] + "\"" + "," + "\"" + g_problem_string[5] + "\"" + "," +
		"\"" + g_problem_string[6] + "\"" + "," + "\"" + g_problem_string[7] + "\"" + "," + "\"" + g_problem_string[8] + "\"" + "," +
		"'" + getCurrentTime() + "', 'N', 0,0,0,0,0,0,1, '" + v_ojname +"', " + v_pid + "";

	if (val_str.length() >= MAX_SIZE_BUF)
	{
		MSG_OUPUT_DBG("ERROR, too large size of buffer...");
		return;
	}

	sprintf(query,"insert into problem(time_limit,memory_limit,title,description,input,output,sample_input,sample_output,author,create_date,defunct,spj,accepted,solved,submit,submit_user,contest_id,isvirtual,oj_name,oj_pid) values(%s);",val_str.c_str());

	//MSG_OUPUT_DBG(query);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		//write_log(ERROR,mysql_error(mysql));
		MSG_OUPUT_DBG(mysql_error(mysql));
		return ;
	}

	MSG_OUPUT_DBG("End SQL_updateProblemInfo OK, (%s)", v_pid.c_str());
}
#endif

ULONG checkStringExsit(char *filename, char *pattern)
{
    pcre  *re;
    const char *error;
    int  erroffset;
    int  ovector[OVECCOUNT];
    int  rc;

	string ts;
    FILE * fp=fopen(filename,"r");

    while (fgets(tmps, MAX_SIZE_BUF, fp))
    {
        ts +=tmps;
    }

    fclose(fp);

	//title
	re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
    if (re == NULL) {                 //如果编译失败，返回错误信息
        MSG_OUPUT_DBG("PCRE compilation failed at offset %d: %s\n", erroffset, error);
        return BOOL_FALSE;
    }

    rc = pcre_exec(re,NULL, ts.c_str(), strlen(ts.c_str()), 0, 0, ovector, OVECCOUNT);
	// 返回值：匹配成功返回非负数，没有匹配返回负数
    if (rc < 0) {                     //如果没有匹配，返回错误信息
        if (rc == PCRE_ERROR_NOMATCH) printf("Sorry, no match ...\n");
        else MSG_OUPUT_DBG("Matching error %d\n", rc);
        pcre_free(re);
        return BOOL_FALSE;
    }

	pcre_free(re);

	return BOOL_TRUE;
}

ULONG getInfoByTag(char *src, char *pattern, ENUM_PROVLEM enProblem, char *res)
{
    pcre  *re;
    const char *error;
    int  erroffset;
    int  ovector[OVECCOUNT];
    int  rc, i;

	MSG_OUPUT_DBG("In getInfoByTag...");

	//title
	re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
    if (re == NULL) {                 //如果编译失败，返回错误信息
        MSG_OUPUT_DBG("PCRE compilation failed at offset %d: %s\n", erroffset, error);
        return BOOL_FALSE;
    }

    rc = pcre_exec(re,NULL, src, strlen(src), 0, 0, ovector, OVECCOUNT);
	// 返回值：匹配成功返回非负数，没有匹配返回负数
    if (rc < 0) {                     //如果没有匹配，返回错误信息
		if (rc == PCRE_ERROR_NOMATCH) MSG_OUPUT_DBG("Sorry, no match ...\n");
		else {
			MSG_OUPUT_DBG("Matching error %d\n", rc);
			g_problem_string[enProblem] = "Not Found";
		}
		pcre_free(re);
		return BOOL_FALSE;
	}

	MSG_OUPUT_DBG("In getInfoByTag...");

	i = (rc==0)?(0):(rc-1);

	printf("iiiiiiii=%d , rc=%d\n",i,rc);

//	for (i = 0; i < rc; i++) //分别取出捕获分组 $0整个正则公式 $1第一个()
	{
        char *substring_start =  src + ovector[2*i];
        int substring_length = ovector[2*i+1] - ovector[2*i];
        	MSG_OUPUT_DBG("In getInfoByTag 1 substring_length=%d...",substring_length);
		char *str_tmp = (char*)malloc(sizeof(char)*substring_length+100);
		//	char str_tmp[MAX_SIZE_BUF] ={0};
			MSG_OUPUT_DBG("In getInfoByTag 2...");
		sprintf(str_tmp, "%.*s\n", substring_length, substring_start);

			MSG_OUPUT_DBG("In getInfoByTag 3...");

		//	printf("%s",str_tmp);

		//string string_ = str_tmp;
			MSG_OUPUT_DBG("In getInfoByTag 4...(length = %d)", strlen(str_tmp));

			g_problem_string[enProblem].assign(str_tmp,strlen(str_tmp));

			MSG_OUPUT_DBG("End getInfoByTag success...");

		//MSG_OUPUT_DBG(pattern);
		//MSG_OUPUT_DBG(string_.c_str());
		//	free(substring_start);
			free(str_tmp);
    }


	pcre_free(re);

	return BOOL_TRUE;
}

int getProblemInfo_Brief(string pid)
{
	ULONG ulRet = 0;
	int loop = 0;
	string res="",ts;
    FILE * fp=fopen(tfilename,"r");

    while (fgets(tmps, MAX_SIZE_BUF, fp))
    {
        ts +=tmps;
    }
    fclose(fp);

	char  patternTime [] = "(\\d*) MS";  // 将要被编译的字符串形式的正则表达式

	char  patternMemory [] = "(\\d*) K";  // 将要被编译的字符串形式的正则表达式

	char  patternTitle [] = "<h1 style='color:#1A5CC8'>([\\s\\S]*?)</h1>";  // 将要被编译的字符串形式的正则表达式

	char  patternDescription [] = "Problem Description</div> <div class=panel_content>([\\s\\S]*?)</div><div class=panel_bottom>&nbsp;</div>";  // 将要被编译的字符串形式的正则表达式

	char  patternInput [] = "Input</div> <div class=panel_content>([\\s\\S]*?)</div><div class=panel_bottom>&nbsp;</div>";  // 将要被编译的字符串形式的正则表达式

	char  patternOutput [] = "Output</div> <div class=panel_content>([\\s\\S]*?)</div><div class=panel_bottom>&nbsp;</div>";  // 将要被编译的字符串形式的正则表达式

	char  patternSampleInput [] = "Sample Input</div><div class=panel_content><pre><div style=\"font-family:Courier New,Courier,monospace;\">([\\s\\S]*?)</div></pre></div><div class=panel_bottom>&nbsp;</div>";  // 将要被编译的字符串形式的正则表达式

	char  patternSampleOutput [] = "Sample Output</div><div class=panel_content><pre><div style=\"font-family:Courier New,Courier,monospace;\">([\\s\\S]*?)</div></pre></div><div class=panel_bottom>&nbsp;</div>";  // 将要被编译的字符串形式的正则表达式

	char  patternAuthor [] = "Author</div> <div class=panel_content>([\\s\\S]*?)</div><div class=panel_bottom>&nbsp;</div>";  // 将要被编译的字符串形式的正则表达式

	//char  patternTitle [] = "<h1 style='color:#1A5CC8'>([\\s\\S]*?)</h1>";  // 将要被编译的字符串形式的正则表达式


	for (loop = 0; loop < PROBLEM_TAG_MAX; loop++)
	{
		g_problem_string[loop] = "";
	}

	MSG_OUPUT_DBG("Start Problem %s ...", pid.c_str());

	MSG_OUPUT_DBG("Time");
	ulRet = getInfoByTag((char*)ts.c_str(), patternTime, PROBLEM_TIME ,NULL);
	if(ulRet == 0)
	{
		g_problem_string[0] = "1000";
	}
	MSG_OUPUT_DBG("Memoty");
	ulRet = getInfoByTag((char*)ts.c_str(), patternMemory, PROBLEM_MEMORY, NULL);
	if(ulRet == 0)
	{
		g_problem_string[1] = "65535";
	}

	ulRet = 0;

	MSG_OUPUT_DBG("Title");
	ulRet += getInfoByTag((char*)ts.c_str(), patternTitle, PROBLEM_TITLE,NULL);

	MSG_OUPUT_DBG("Description");
	ulRet += getInfoByTag((char*)ts.c_str(), patternDescription, PROBLEM_DESCRIPTION, NULL);

	MSG_OUPUT_DBG("Input");
	ulRet += getInfoByTag((char*)ts.c_str(), patternInput, PROBLEM_INPUT, NULL);

	MSG_OUPUT_DBG("Output");
	ulRet += getInfoByTag((char*)ts.c_str(), patternOutput, PROBLEM_OUTPUT, NULL);

	MSG_OUPUT_DBG("Sample Input");
	ulRet += getInfoByTag((char*)ts.c_str(), patternSampleInput, PROBLEM_SAMPLE_INPUT, NULL);

	MSG_OUPUT_DBG("Sample Output");
	ulRet += getInfoByTag((char*)ts.c_str(), patternSampleOutput, PROBLEM_SAMPLE_OUTPUT, NULL);

	MSG_OUPUT_DBG("Author");
	ulRet += getInfoByTag((char*)ts.c_str(), patternAuthor, PROBLEM_AUTHOR, NULL);

	if (ulRet != 0)
	{
		if (BOOL_TRUE == checkStringExsit(tfilename, "No such problem"))
		{
			MSG_OUPUT_DBG("No such problem %s", pid.c_str());

			return 0;
		}
	}

	//SQL_updateProblemInfo("HDU",pid);

	MSG_OUPUT_DBG("Get Problem %s OK.", pid.c_str());

    return 0;
}
#endif
#if 0
ULONG getProblemInfo(string pid)
{
	CURL *curl;
    CURLcode res;

	curl = curl_easy_init();

	if (access(tfilename, 0) == 0)
	{
		DeleteFile(tfilename);
	}

    if ( curl ) {
		FILE *fp = fopen(tfilename, "ab+");
		curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "hdu.cookie");
		char url[255] = {0};
		sprintf(url, "http://acm.hdu.edu.cn/showproblem.php?pid=%s", pid.c_str());
		//cout<<url;
		curl_easy_setopt( curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

		res = curl_easy_perform( curl );
		curl_easy_cleanup(curl);
		fclose(fp);
	}

	getProblemInfo_Brief(pid);

	return BOOL_TRUE;
}
#endif

//////////////////
// DLL exp
//////////////////


ULONG DLL_HDU_SpiderInit(int pid)
{
	if (access(tfilename, 0) == 0)
	{
		DeleteFile(tfilename);
	}

	return BOOL_TRUE;
}

#if 0
ULONG DLL_GetProblemInfoFromHDU(int pid)
{
	char tmp[10]={0};
	itoa(pid,tmp,10);
	string pid_s = tmp;

	if (BOOL_TRUE != getProblemInfo(pid_s))
	{
		return BOOL_FALSE;
	}

	return BOOL_TRUE;
}


ULONG DLL_HDULogin()
{
	if (BOOL_TRUE != login())
	{
		return BOOL_FALSE;
	}

	return BOOL_TRUE;
}

ULONG DLL_HDUSubmit(int pid, int langid, string source)
{
	char tmp[10]={0};
	itoa(pid,tmp,10);
	string pid_s = tmp;

	char tmplang[10]={0};
	itoa(langid,tmplang,10);
	string lang_string = tmplang;

	if (BOOL_TRUE != submit(pid_s, lang_string, source))
	{
		return BOOL_FALSE;
	}
	return BOOL_TRUE;
}

ULONG DLL_HDUGetStatus(string hdu_username, int pid, int langid, string &runid, string &result,string& ce_info,string &tu,string &mu)
{
	char tmp[10]={0};
	itoa(pid,tmp,10);
	string pid_s = tmp;
	//string runid,result,ce_info,tu, mu;

	char tmplang[10]={0};
	itoa(langid,tmplang,10);
	string lang_string = tmplang;

	if (BOOL_TRUE != getStatus(hdu_username, pid_s, lang_string, runid, result, ce_info, tu, mu))
	{
		MSG_OUPUT_DBG("DLL_HDUGetStatus getStatus error...");
		return BOOL_FALSE;
	}
	else
	{
		MSG_OUPUT_DBG("DLL_HDUGetStatus getStatus success...");
	}

	return BOOL_TRUE;
}

#endif

ULONG DLL_HDUDebugSwitch(ULONG st)
{
	set_debug_switch(st);

	return BOOL_TRUE;
}


/* END HDU VJUDGE */


/////////////////////////LOG
//#define LOG(level) Log(__FILE__, __LINE__, level).GetStream()

extern void pdt_debug_print(const char *format, ...);


void write_log(int level, const char *fmt, ...) {
	va_list ap;
	char buffer[4096];
	time_t  timep = time(NULL);
	int l;
	struct tm *p;
    p = localtime(&timep);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;
	sprintf(buffer,"log/Judge-%04d-%02d-%02d.log",p->tm_year, p->tm_mon, p->tm_mday);

	FILE *fp = fopen(buffer, "a+");
	if (fp == NULL) {
		fprintf(stderr, "open logfile error!\n");
		return;
	}

	fprintf(fp, "%s:%04d-%02d-%02d %02d:%02d:%02d ",LEVEL_NAME[level],p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	if (DEBUG)
	{
		//printf("%s:%04d-%02d-%02d %02d:%02d:%02d ",LEVEL_NAME[level],p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	}

	va_start(ap, fmt);
	l = vsprintf(buffer, fmt, ap);
	fprintf(fp, "%s\n", buffer);
	if (DEBUG)
	{
		/* BEGIN: Added by weizengke, 2013/11/15 for vrp */
		pdt_debug_print("%s", buffer);
		/* END:   Added by weizengke, 2013/11/15 */
		//printf("%s\n", buffer);
	}
	va_end(ap);
	fclose(fp);
}

///////////END LOG


////////////////////////////////////////////////////socket
#define PORT 5000
#define BUFFER 1024
#pragma comment(lib,"WSOCK32.lib")   //必须的
int port=PORT;

typedef struct
{
	int solutionId;
}JUDGE_DATA;

int GL_currentId;//当前裁判id
queue <JUDGE_DATA> Q;//全局队列

SOCKET sListen;

//#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup") // 设置连接器选项

int initSocket()
{
	write_log(INFO,"Start initialization of Socket...");

	WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);
	//加载winsock库
	if(WSAStartup(sockVersion, &wsaData) != 0)
		return 0;
	// 创建套节字
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sListen == INVALID_SOCKET)
	{
		write_log(SYSTEM_ERROR,"create socket error");
		return 0;
	}
	// 在sockaddr_in结构中装入地址信息
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);	// htons函数 将主机的无符号短整形数转换成网络
	//字节顺序
	sin.sin_addr.S_un.S_addr = INADDR_ANY;


	int trybind=50;  //重试bind次数
	int ret=0;
	while((ret=bind(sListen,(LPSOCKADDR)&sin,sizeof(sin)))==SOCKET_ERROR&&trybind>0)
	{
		write_log(SYSTEM_ERROR,"bind failed:%d , it will try later...",WSAGetLastError());
		trybind--;
		Sleep(100);
	}
	if(ret<0) {
		write_log(SYSTEM_ERROR,"Bind failed...");
		return 0;
	}
	write_log(INFO,"Bind success...");

	//进入监听状态
	int trylisten=50; //重试listen次数
	while((ret=listen(sListen,20))==SOCKET_ERROR&&trylisten)
	{
		write_log(SYSTEM_ERROR,"listen failed:%d , it will try later..",WSAGetLastError());
		trylisten--;
		Sleep(100);
		return 0;
	}
	if(ret<0) {
		write_log(SYSTEM_ERROR,"Listen failed...");
		return 0;
	}
	write_log(INFO,"Listen success...");

	return 1;
}
//////////////////////////////////////////////////////////////end socket

int InitMySQL()   //初始化mysql，并设置字符集
{
	mysql=mysql_init((MYSQL*)0);
	if(mysql!=0 && !mysql_real_connect(mysql,Mysql_url, Mysql_username, Mysql_password, Mysql_table,Mysql_port,NULL,CLIENT_MULTI_STATEMENTS )){
		write_log(ERROR,mysql_error(mysql));
		return 0;
	}
	strcpy(query,"SET CHARACTER SET gbk"); //设置编码 gbk

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret){
		write_log(ERROR,mysql_error(mysql));
		return 0;
	}
	return 1;
}


void InitMySqlConfig(){
	port=GetPrivateProfileInt("Tool","Port",PORT,INI_filename);
	isDeleteTemp=GetPrivateProfileInt("Tool","DeleteTemp",0,INI_filename);
	limitJudge=GetPrivateProfileInt("Tool","LimitJudge",20,INI_filename);
	OutputLimit=GetPrivateProfileInt("Tool","OutputLimit",10000,INI_filename);
	JUDGE_LOG_BUF_SIZE=GetPrivateProfileInt("Tool","JUDGE_LOG_BUF_SIZE",500,INI_filename);

	isRestrictedFunction=GetPrivateProfileInt("Tool","isRestrictedFunction",0,INI_filename);
	GetPrivateProfileString("Tool","WorkingPath","",workPath,sizeof(workPath),INI_filename);
	GetPrivateProfileString("Tool","DataPath","",dataPath,sizeof(dataPath),INI_filename);
	GetPrivateProfileString("Tool","JudgePath","",judgePath,sizeof(judgePath),INI_filename);
	GetPrivateProfileString("Tool","JudgeLogPath","",judgeLogPath,sizeof(judgeLogPath),INI_filename);

	GetPrivateProfileString("MySQL","url","",Mysql_url,sizeof(Mysql_url),INI_filename);
	GetPrivateProfileString("MySQL","username","",Mysql_username,sizeof(Mysql_username),INI_filename);
	GetPrivateProfileString("MySQL","password","",Mysql_password,sizeof(Mysql_password),INI_filename);
	GetPrivateProfileString("MySQL","table","",Mysql_table,sizeof(Mysql_table),INI_filename);
	Mysql_port=GetPrivateProfileInt("MySQL","port",0,INI_filename);

	GetPrivateProfileString("HDU","username","",hdu_username,sizeof(hdu_username),INI_filename);
	GetPrivateProfileString("HDU","password","",hdu_password,sizeof(hdu_password),INI_filename);

	write_log(INFO,"socketPort:%d, DataPath:%s, TempPath:%s",port,dataPath,workPath);
	write_log(INFO,"MySQL:%s %s %s %s %d",Mysql_url,Mysql_username,Mysql_password,Mysql_table,Mysql_port);

}


void InitPath()
{
	if( (_access(workPath, 0 )) == -1 )   {
		CreateDirectory(workPath,NULL);
	}

	//GetPrivateProfileString(lpAppName,lpKeyName,lpDefault,lpReturnedString,nSize,lpFileName);
	//GetPrivateProfileInt(lpAppName,lpKeyName,nDefault,lpFileName);

	char keyname[100]={0};
	sprintf(keyname,"Language%d",GL_languageId);

	GetPrivateProfileString("Language",keyname,"",GL_languageName,100,INI_filename);

	//sprintf(keyname,"%s",GL_languageName);

	GetPrivateProfileString("LanguageExt",GL_languageName,"",GL_languageExt,10,INI_filename);

	GetPrivateProfileString("LanguageExe",GL_languageName,"",GL_languageExe,10,INI_filename);

	GetPrivateProfileString("CompileCmd",GL_languageName,"",compileCmd_str,1024,INI_filename);

	GetPrivateProfileString("RunCmd",GL_languageName,"",runCmd_str,1024,INI_filename);

	GetPrivateProfileString("SourcePath",GL_languageName,"",sourcePath,1024,INI_filename);

	GetPrivateProfileString("ExePath",GL_languageName,"",exePath,1024,INI_filename);

	isTranscoding=GetPrivateProfileInt("Transcoding",GL_languageName,0,INI_filename);

	limitIndex=GetPrivateProfileInt("TimeLimit",GL_languageName,1,INI_filename);

	nProcessLimit=GetPrivateProfileInt("ProcessLimit",GL_languageName,1,INI_filename);


	char buf[1024];
	sprintf(buf, "%d", GL_solutionId);
	string name = buf;
	string compile_string=compileCmd_str;
	replace_all_distinct(compile_string,"%PATH%",workPath);
	replace_all_distinct(compile_string,"%NAME%",name);
	replace_all_distinct(compile_string,"%EXT%",GL_languageExt);
	replace_all_distinct(compile_string,"%EXE%",GL_languageExe);
	strcpy(compileCmd_str,compile_string.c_str());      //编译命令行
	//	cout<<CompileCmd_str<<endl;

	string runcmd_string=runCmd_str;
	replace_all_distinct(runcmd_string,"%PATH%",workPath);
	replace_all_distinct(runcmd_string,"%NAME%",name);
	replace_all_distinct(runcmd_string,"%EXT%",GL_languageExt);
	replace_all_distinct(runcmd_string,"%EXE%",GL_languageExe);
	strcpy(runCmd_str,runcmd_string.c_str());			//运行命令行
	//	cout<<RunCmd_str<<endl;

	string sourcepath_string=sourcePath;
	replace_all_distinct(sourcepath_string,"%PATH%",workPath);
	replace_all_distinct(sourcepath_string,"%NAME%",name);
	replace_all_distinct(sourcepath_string,"%EXT%",GL_languageExt);
	strcpy(sourcePath,sourcepath_string.c_str());		//源程序路径
	//  cout<<SourcePath<<endl;

	string exepath_string=exePath;
	replace_all_distinct(exepath_string,"%PATH%",workPath);
	replace_all_distinct(exepath_string,"%NAME%",name);
	replace_all_distinct(exepath_string,"%EXE%",GL_languageExe);
	strcpy(exePath,exepath_string.c_str());				//可执行文件路径
	//	cout<<ExePath<<endl;

	sprintf(DebugFile,"%s%s.txt",workPath,name.c_str()); //debug文件路径
	sprintf(ErrorFile,"%s%s_re.txt",workPath,name.c_str()); //re文件路径

	if( (_access(judgeLogPath, 0 )) == -1 )   {
		CreateDirectory(judgeLogPath,NULL);
	}

	sprintf(judge_log_filename,"%sjudge-log-%d.log",judgeLogPath,GL_solutionId);

	//	cout<<DebugFile<<endl;
}

int SQL_getSolutionSource(){
	sprintf(query,"select source from solution_source where solution_id=%d",GL_solutionId);
	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret){
		write_log(ERROR,mysql_error(mysql));
		return 0;
	}
	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL){
		write_log(ERROR,"SQL_getSolutionSource");
		return 0;
	}
	FILE *fp_source = fopen(sourcePath, "w");
	char code[MAX_CODE]={0};
	MYSQL_ROW row;
	if(row=mysql_fetch_row(recordSet))
	{
		sprintf(code, "%s", row[0]);
	}else {
		write_log(ERROR,"SQL_getSolutionSource Error");
	}

	if(isTranscoding==1){
		int ii=0;
		//解决VS下字符问题
		while (code[ii]!='\0') {
			if (code[ii]=='\r') code[ii] = '\n';ii++;
		}
	}

	fprintf(fp_source, "%s", code);

	/* add for vjudge*/
	string code_ = code;
	GL_source = code_;

	mysql_free_result(recordSet);
	fclose(fp_source);
	return 1;
}
int SQL_getSolutionInfo(){
	//problemId,languageId,submitDate,username

	sprintf(query,"select problem_id,contest_id,language,username,submit_date from solution where solution_id=%d",GL_solutionId);
	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret){
		write_log(ERROR,mysql_error(mysql));
		//printf("Error SQL_getSolutionData...\n");
		return 0;
	}
	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL){
		write_log(ERROR,"Error SQL_getSolutionData");
		return 0;
	}

	//获取数据
	MYSQL_ROW row; //一个行数据的类型安全(type-safe)的表示
	if(row=mysql_fetch_row(recordSet))  //获取下一条记录
	{
		GL_problemId=atoi(row[0]);
		GL_contestId=atoi(row[1]);
		GL_languageId=atoi(row[2]);
		strcpy(GL_username,row[3]);
		StringToTimeEX(row[4],GL_submitDate);

	}else {
		write_log(ERROR,"Error SQL_getSolutionData");
	}
	mysql_free_result(recordSet);//释放结果集
	return 1;
}

int SQL_getProblemInfo(){//time_limit,memory_limit,spj

	sprintf(query,"select time_limit,memory_limit,spj,isvirtual,oj_pid from problem where problem_id=%d",GL_problemId);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret){
		write_log(ERROR,mysql_error(mysql));
		return 0;
	}
	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL){
		return 0;
	}

	//获取数据
	MYSQL_ROW row; //一个行数据的类型安全(type-safe)的表示
	if(row=mysql_fetch_row(recordSet))  //获取下一条记录
	{
		GL_time_limit=atoi(row[0]);
		GL_memory_limit=atoi(row[1]);
		GL_spj=atoi(row[2]);
		GL_vjudge=atoi(row[3]);
		GL_vpid=atoi(row[4]);
	}

	mysql_free_result(recordSet);//释放结果集
	return 1;
}

int SQL_getProblemInfo_contest(int contestId,int problemId,char *num){//num

	sprintf(query,"select num from contest_problem where contest_id=%d and problem_id=%d",contestId,problemId);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret){
		write_log(ERROR,mysql_error(mysql));
		return 0;
	}
	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL){
		write_log(INFO,"SQL_getProblemInfo_contest ,No record");
		return 0;
	}

	//获取数据
	MYSQL_ROW row; //一个行数据的类型安全(type-safe)的表示
	if(row=mysql_fetch_row(recordSet))  //获取下一条记录
	{
		strcpy(num,row[0]);
	}

	mysql_free_result(recordSet);//释放结果集
	return 1;
}

int SQL_getContestInfo(int contestId,time_t &start_time,time_t &end_time){
	//start_time,end_time

	sprintf(query,"select start_time,end_time from contest where contest_id=%d",contestId);
	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret){
		write_log(ERROR,mysql_error(mysql));
		return 0;
	}
	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL){
		return 0;
	}

	//获取数据
	MYSQL_ROW row; //一个行数据的类型安全(type-safe)的表示
	if(row=mysql_fetch_row(recordSet))  //获取下一条记录
	{
		StringToTimeEX(row[0],start_time);
		StringToTimeEX(row[1],end_time);
	}

	mysql_free_result(recordSet);//释放结果集
	return 1;
}
int SQL_getContestAttend(int contestId,char *username,char num,long &ac_time,int &wrongsubmits){
	//

	sprintf(query,"select %c_time,%c_wrongsubmits from attend where contest_id=%d and username='%s';",num,num,contestId,username);

	//cout<<query<<endl;
	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret){
		write_log(ERROR,mysql_error(mysql));
		return 0;
	}
	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL){
		return 0;
	}

	MYSQL_ROW row;
	if(row=mysql_fetch_row(recordSet))  {
		ac_time=atol(row[0]);
		wrongsubmits=atoi(row[1]);
	}

	mysql_free_result(recordSet);
	return 1;
}
int SQL_countContestProblems(int contestId){
	sprintf(query,"select count(problem_id) from contest_problem where contest_id=%d",contestId);
	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret){
		write_log(ERROR,mysql_error(mysql));
		return 0;
	}
	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL){
		return 0;
	}
	int nCount=0;
	MYSQL_ROW row;
	if(row=mysql_fetch_row(recordSet))
	{
		nCount=atoi(row[0]);
	}

	mysql_free_result(recordSet);
	return nCount;
}

int SQL_getFirstACTime_contest(int contestId,int problemId,char *username,time_t &ac_time,time_t start_time,time_t end_time){
	//
	string s_t,e_t;
	API_TimeToString(s_t,start_time);
	API_TimeToString(e_t,end_time);

	sprintf(query,"select submit_date from solution where contest_id=%d and problem_id=%d and username='%s'and verdict=%d and submit_date between '%s' and '%s' order by solution_id ASC limit 1;",contestId,problemId,username,V_AC,s_t.c_str(),e_t.c_str());
	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret){
		write_log(ERROR,mysql_error(mysql));
		return 0;
	}
	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL){

		return 0;
	}

	MYSQL_ROW row;
	if(row=mysql_fetch_row(recordSet))  {

		StringToTimeEX(row[0],ac_time);
	}else {
		return 0;
	}
	mysql_free_result(recordSet);
	return 1;
}
long SQL_countProblemVerdict(int contestId,int problemId,int verdictId,char *username){

	sprintf(query,"select count(solution_id) from solution where contest_id=%d and problem_id=%d and verdict=%d and username='%s'",contestId,problemId,verdictId,username);
	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret){
		write_log(ERROR,mysql_error(mysql));
		return 0;
	}
	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL){
		return 0;
	}

	long nCount=0;

	//获取数据
	MYSQL_ROW row; //一个行数据的类型安全(type-safe)的表示
	if(row=mysql_fetch_row(recordSet))  //获取下一条记录
	{
		nCount=atoi(row[0]);
	}
	mysql_free_result(recordSet);//释放结果集
	return nCount;
}


void SQL_updateSolution(int solutionId,int verdictId,int testCase,int time,int memory)
{
	sprintf(query,"update solution set verdict=%d,testcase=%d,time=%d,memory=%d where solution_id=%d;",verdictId,testCase,time,memory,solutionId);
	//	printf("%s\n",query);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
	}
}

void SQL_updateProblem(int problemId)
{
	sprintf(query,"update problem set accepted=(SELECT count(*) FROM solution WHERE problem_id=%d and verdict=%d) where problem_id=%d;",problemId,V_AC,problemId);
//cout<<query;
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
	}
	sprintf(query,"update problem set submit=(SELECT count(*) FROM solution WHERE problem_id=%d)  where problem_id=%d;",problemId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
	}
	sprintf(query,"update problem set solved=(SELECT count(DISTINCT username) FROM solution WHERE problem_id=%d and verdict=%d) where problem_id=%d;",problemId,V_AC,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
	}
	sprintf(query,"update problem set submit_user=(SELECT count(DISTINCT username) FROM solution WHERE problem_id=%d) where problem_id=%d;",problemId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
	}
}
void SQL_updateProblem_contest(int contestId,int problemId)
{
	sprintf(query,"update contest_problem set accepted=(SELECT count(*) FROM solution WHERE contest_id=%d and problem_id=%d and verdict=%d) where contest_id=%d and problem_id=%d;",contestId,V_AC,contestId,problemId);
//	cout<<query;
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
	}
	sprintf(query,"update contest_problem set submit=(SELECT count(*) FROM solution WHERE contest_id=%d and problem_id=%d)  where contest_id=%d and problem_id=%d;",contestId,problemId,contestId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
	}
	sprintf(query,"update contest_problem set solved=(SELECT count(DISTINCT username) FROM solution WHERE contest_id=%d and problem_id=%d and verdict=%d) where contest_id=%d and problem_id=%d;",contestId,problemId,V_AC,contestId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
	}
	sprintf(query,"update contest_problem set submit_user=(SELECT count(DISTINCT username) FROM solution WHERE contest_id=%d and problem_id=%d) where contest_id=%d and problem_id=%d;",contestId,problemId,contestId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
	}
}

int SQL_countACContestProblems(int contestId,char *username,time_t start_time,time_t end_time){

	string s_t,e_t;
	API_TimeToString(s_t,start_time);
	API_TimeToString(e_t,end_time);

	sprintf(query,"select count(distinct(problem_id)) from solution where  verdict=%d and contest_id=%d and username='%s' and submit_date between '%s' and '%s'",V_AC,contestId,username,s_t.c_str(),e_t.c_str());

	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
		return 0;
	}
	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL){
		return 0;
	}
	int nCount=0;
	MYSQL_ROW row;
	if(row=mysql_fetch_row(recordSet))
	{
		nCount=atoi(row[0]);
	}

	mysql_free_result(recordSet);
	return nCount;
}


int SQL_getContestScore(int contestId,char *username,time_t start_time,time_t end_time){

	if(SQL_countACContestProblems(contestId,username,start_time,end_time)==0){
		return 0;
	}
	string s_t,e_t;
	API_TimeToString(s_t,start_time);
	API_TimeToString(e_t,end_time);

	sprintf(query,"SELECT sum(point) from contest_problem where contest_id=%d and problem_id in (select distinct(problem_id) from solution where  verdict=%d and contest_id=%d and username='%s' and submit_date between '%s' and '%s')",contestId,V_AC,contestId,username,s_t.c_str(),e_t.c_str());

	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
		return 0;
	}
	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL){
		write_log(ERROR,"SQL_getContestScore recordSet ERROR");
	}
	int point=0;
	MYSQL_ROW row;

	if(row=mysql_fetch_row(recordSet))
	{
		point=atoi(row[0]);
	}
	mysql_free_result(recordSet);
	return point;
}

void SQL_updateAttend_contest(int contestId,int verdictId,int problemId,char *num,char *username,time_t start_time,time_t end_time){

	//已经AC的不需要修改attend
	//update ac_time
	long AC_time=0;
	time_t first_ac_t;
	if(SQL_getFirstACTime_contest(contestId,problemId,username,first_ac_t,start_time,end_time)){
		AC_time=getdiftime(first_ac_t,start_time);
	}else{
		AC_time=0;
		first_ac_t = end_time;
	}
	sprintf(query,"update attend set %s_time=%ld where contest_id=%d and username='%s';",num,AC_time,contestId,username);
	//cout<<query<<endl;
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
	}

	long ac_nCount=SQL_countProblemVerdict(GL_contestId,problemId,V_AC,username);
	int score_ = SQL_getContestScore(contestId,username,start_time,end_time);
	string s_t,e_t,fAC_t;
	API_TimeToString(s_t,start_time);
	API_TimeToString(e_t,end_time);
	API_TimeToString(fAC_t,first_ac_t);

	//update score solved ,wrongsubmits
	sprintf(query,"update attend set solved=(SELECT count(DISTINCT problem_id) FROM solution WHERE contest_id=%d and username='%s' and verdict=%d and submit_date between '%s' and '%s'),%s_wrongsubmits=(SELECT count(solution_id) FROM solution WHERE contest_id=%d and problem_id=%d and username='%s' and verdict>%d and submit_date between '%s' and '%s'),score=%d  where contest_id=%d and username='%s';",contestId,username,V_AC,s_t.c_str(),e_t.c_str(),   num,contestId,problemId,username,V_AC,s_t.c_str(),fAC_t.c_str(),  score_,  contestId,username);

//	puts(query);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
			write_log(ERROR,mysql_error(mysql));
	}

	//penalty
	int nCountProblems=SQL_countContestProblems(contestId);
	char index='A';
	long penalty=0;
	for(int i=0;i<nCountProblems;i++){
		long a_time_=0;
		int wrongsubmits_=0;
		SQL_getContestAttend(contestId,username,index+i,a_time_,wrongsubmits_);
		//cout<<a_time_<<" "<<wrongsubmits_<<endl;
		if(a_time_>0){
			penalty=penalty+a_time_+wrongsubmits_*60*20;
		}
	}
	sprintf(query,"update attend set penalty=%ld where contest_id=%d and username='%s';",penalty,contestId,username);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
	}


}

void SQL_updateUser(char *username)
{
	sprintf(query,"update users set submit=(SELECT count(*) FROM solution WHERE username='%s') where username='%s';",username,username);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
	}
	sprintf(query,"update users set solved=(SELECT count(DISTINCT problem_id) FROM solution WHERE username='%s' and verdict=%d) where username='%s';",username,V_AC,username);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query))){
		write_log(ERROR,mysql_error(mysql));
	}
}


void SQL_updateCompileInfo(int solutionId)
{
	FILE *fp;
	char buffer[4096]={0};
	if ((fp = fopen (DebugFile, "r")) == NULL){
		write_log(ERROR,"DebugFile open error");
		return ;
	}
	//先删除
	sprintf(query,"delete from compile_info  where solution_id=%d;",solutionId);
	mysql_real_query(mysql,query,(unsigned int)strlen(query));

	if((fgets(buffer, 4095, fp))!= NULL){  //先插入
		sprintf(query,"insert into compile_info values(%d,\"%s\");",solutionId,buffer);
		int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
		if(ret){
			write_log(ERROR,mysql_error(mysql));
			fclose(fp);
			return ;
		}
	}
	while ((fgets (buffer, 4095, fp))!= NULL){ //后连接
		buffer[strlen(buffer)];
		sprintf(query,"update compile_info set error=CONCAT(error,\"%s\") where solution_id=%d;",buffer,solutionId);
		int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
		if(ret){
			write_log(ERROR,mysql_error(mysql));
			fclose(fp);
			return ;
		}
 	}
	fclose(fp);
}

DWORD WINAPI CompileThread(LPVOID lp) //ac
{

	SQL_updateSolution(GL_solutionId,V_C,0,0,0); //V_C Compiling
	system(compileCmd_str);

	return 0;
}
//编译,是否应该防止编译器假死造成的卡死
int compile(){

	if(strcmp(compileCmd_str,"NULL")==0) return 1;

	HANDLE hThread_com;
	hThread_com=CreateThread(NULL,NULL,CompileThread,NULL,0,NULL);
	if(hThread_com==NULL) {
		write_log(ERROR,"Create CompileThread Error");
		CloseHandle(hThread_com);
	}

	DWORD status_ = WaitForSingleObject(hThread_com,30000);   //30S 编译时间,返回值大于零说明超时
	if(status_>0){
		write_log(WARNING,"Compile over time_limit");
	}
	//是否正常生成用户的可执行程序
	if( (_access(exePath, 0 )) != -1 )   {
		return 1;
	}
	else return 0;
}

//是否存在异常
BOOL RUN_existException(DWORD dw){
	switch(dw){
	case EXCEPTION_ACCESS_VIOLATION:
		return TRUE;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		return TRUE;
	case EXCEPTION_BREAKPOINT:
		return TRUE;
	case EXCEPTION_SINGLE_STEP:
		return TRUE;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		return TRUE;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		return TRUE;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		return TRUE;
	case EXCEPTION_FLT_INEXACT_RESULT:
		return TRUE;
	case EXCEPTION_FLT_INVALID_OPERATION:
		return TRUE;
	case EXCEPTION_FLT_OVERFLOW:
		return TRUE;
	case EXCEPTION_FLT_STACK_CHECK:
		return TRUE;
	case EXCEPTION_FLT_UNDERFLOW:
		return TRUE;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		return TRUE;
	case EXCEPTION_INT_OVERFLOW:
		return TRUE;
	case EXCEPTION_PRIV_INSTRUCTION:
		return TRUE;
	case EXCEPTION_IN_PAGE_ERROR:
		return TRUE;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		return TRUE;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		return TRUE;
	case EXCEPTION_STACK_OVERFLOW:
		return TRUE;
	case EXCEPTION_INVALID_DISPOSITION:
		return TRUE;
	case EXCEPTION_GUARD_PAGE:
		return TRUE;
	case EXCEPTION_INVALID_HANDLE:
		return TRUE;
	default:
		return FALSE;
	}
}

HANDLE G_job=NULL;
HANDLE InputFile ;  //父进程输入文件句柄
HANDLE OutputFile;  //子进程标准输出句柄
DWORD g_dwCode;	//定义进程状态

HANDLE CreateSandBox(){
	HANDLE hjob =CreateJobObject(NULL,NULL);
	if(hjob!=NULL)
	{
		JOBOBJECT_BASIC_LIMIT_INFORMATION jobli;
		 memset(&jobli,0,sizeof(jobli));
		jobli.LimitFlags=JOB_OBJECT_LIMIT_PRIORITY_CLASS|JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
		jobli.PriorityClass=IDLE_PRIORITY_CLASS;
		jobli.ActiveProcessLimit=nProcessLimit; //Limit of processes
	//	jobli.MinimumWorkingSetSize= 1;
	//	jobli.MaximumWorkingSetSize= 1024*GL_memory_limit;|JOB_OBJECT_LIMIT_WORKINGSET|JOB_OBJECT_LIMIT_PROCESS_TIME
	//	jobli.PerProcessUserTimeLimit.QuadPart=10000*(GL_time_limit+2000);
		if(SetInformationJobObject(hjob,JobObjectBasicLimitInformation,&jobli,sizeof(jobli)))
		{
			JOBOBJECT_BASIC_UI_RESTRICTIONS jobuir;
			jobuir.UIRestrictionsClass=JOB_OBJECT_UILIMIT_NONE;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_EXITWINDOWS;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_READCLIPBOARD ;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_WRITECLIPBOARD ;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_HANDLES;

			if(SetInformationJobObject(hjob,JobObjectBasicUIRestrictions,&jobuir,sizeof(jobuir)))
			{
				return hjob;
			}
			else
			{
				write_log(SYSTEM_ERROR,"SetInformationJobObject  JOBOBJECT_BASIC_UI_RESTRICTIONS   [Error:%d]\n",GetLastError());
			}
		}
		else
		{
			write_log(SYSTEM_ERROR,"SetInformationJobObject  JOBOBJECT_BASIC_LIMIT_INFORMATION   [Error:%d]\n",GetLastError());
		}
	}
	else
	{
		write_log(SYSTEM_ERROR,"CreateJobObject     [Error:%d]\n",GetLastError());
	}
	return NULL;
}

bool ProcessToSandbox(HANDLE job,PROCESS_INFORMATION p){
	if(AssignProcessToJobObject(job,p.hProcess))
	{
		//顺便调整本进程优先级为高
		/*HANDLE   hPS   =   OpenProcess(PROCESS_ALL_ACCESS,   false,  p.dwProcessId);
		if(!SetPriorityClass(hPS,   HIGH_PRIORITY_CLASS)){
			write_log(SYSTEM_ERROR,"SetPriorityClass        [Error:%d]\n",GetLastError());
		}
		CloseHandle(hPS);*/
		return true;
	}
	else
	{
		write_log(SYSTEM_ERROR,"AssignProcessToJobObject Error:%s",GetLastError());
	}
	return false;
}

DWORD WINAPI RUN_ProgramThread(LPVOID lp) //ac
{
	/// cmd/c solution.exe <data.in >data.out 2>error.txt
	//ChildIn_Write是子进程的输入句柄，ChildIn_Read是父进程用于写入子进程输入的句柄
	HANDLE ChildIn_Read, ChildIn_Write;
	//ChildOut_Write是子进程的输出句柄，ChildOut_Read是父进程用于读取子进程输出的句柄
	HANDLE ChildOut_Read, ChildOut_Write;

	SECURITY_ATTRIBUTES saAttr = {0};
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	CreatePipe(&ChildIn_Read, &ChildIn_Write, &saAttr, 0);
	SetHandleInformation(ChildIn_Write, HANDLE_FLAG_INHERIT, 0);
	CreatePipe(&ChildOut_Read, &ChildOut_Write, &saAttr, 0);
	SetHandleInformation(ChildOut_Read, HANDLE_FLAG_INHERIT, 0);

	SetErrorMode(SEM_NOGPFAULTERRORBOX );

	STARTUPINFO StartupInfo = {0};
	StartupInfo.cb = sizeof(STARTUPINFO);
	//	StartupInfo.hStdError = h_ErrorFile;
	StartupInfo.hStdOutput = ChildOut_Write;
	StartupInfo.hStdInput = ChildIn_Read;
	StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

	if(CreateProcess(NULL,runCmd_str,NULL,NULL,TRUE,CREATE_SUSPENDED|CREATE_NEW_CONSOLE,NULL,NULL,&StartupInfo,&G_pi))
	{

		G_job = CreateSandBox();
		if(G_job!=NULL)
		{
			if(ProcessToSandbox(G_job,G_pi))
			{
				ResumeThread(G_pi.hThread);
				CloseHandle(G_pi.hThread);

				InputFile= CreateFile(inFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
				BOOL flag = FALSE;
				while (true)
				{
					char buffer[BUFSIZE] = {0};
					DWORD BytesRead, BytesWritten;
					flag = ReadFile(InputFile, buffer, BUFSIZE, &BytesRead, NULL);
					if (!flag || (BytesRead == 0)) break;
					flag = WriteFile(ChildIn_Write, buffer, BytesRead, &BytesWritten, NULL);

					if (!flag){ break;}
				}

				CloseHandle(InputFile);InputFile=NULL;
				CloseHandle(ChildIn_Write);ChildIn_Write=NULL;
				CloseHandle(ChildOut_Write);ChildOut_Write=NULL;

				//读取子进程的标准输出，并将其传递给文件输出
				OutputFile= CreateFile(outFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				startt = clock();

				DWORD limit_output =0;
				while (true)
				{
					char buffer[BUFSIZE] = {0};
					DWORD BytesRead, BytesWritten;
					flag = ReadFile(ChildOut_Read, buffer, BUFSIZE, &BytesRead, NULL);
					if (!flag || (BytesRead == 0)) break;
					flag = WriteFile(OutputFile, buffer, BytesRead, &BytesWritten, NULL);
					if (!flag) break;

					limit_output+=BytesWritten;
					if(limit_output>OutputLimit){
						write_log(INFO,"OLE");
						GL_verdictId = V_OLE;
						//CloseHandle(pi.hProcess);
						break;
					}
				}
				endt = clock();

				CloseHandle(ChildIn_Read);ChildIn_Read=NULL;
				CloseHandle(ChildOut_Read);ChildOut_Read=NULL;
				CloseHandle(OutputFile);OutputFile=NULL;
				//printf("OK\n");
				return 1;
			}else{
				write_log(SYSTEM_ERROR,"ProcessToSandBox Error:%s",GetLastError());
			}
		}
		else{
			write_log(SYSTEM_ERROR,"CreateSandBox Error:%s",GetLastError());
		}
	}
	else
	{
		write_log(SYSTEM_ERROR,"CreateProcess       [Error:%d]\n",GetLastError());
	}
	GL_verdictId = V_SE;
	return 0;
}

int special_judge(const char *inFile,const char *uOutFile){ //return 0 ====Error,return 1 ====Accepted
	int judge ;
	char spj_path[MAX_PATH];
	sprintf(spj_path,"%s%d\\spj_%d.exe %s %s",dataPath,GL_problemId,GL_problemId,inFile,uOutFile);
	judge = system(spj_path) ;

	if (judge == -1){
		//printf("system error!") ;
		return 0;
	}
	if(judge == 1){  //spj返回1,表示程序正确
		return 1;
	}
	return 0;
}

int RUN_solution(int solutionId)
{
	long caseTime=0;
	int i,case_;
	char srcPath[MAX_PATH];
	char ansPath[MAX_PATH];
	char buf[40960];

	g_dwCode=0;

	for(i=0;;++i){
		if(i==0){
			case_=1;
		}else{
			case_=i;
		}
		sprintf(inFileName,"%s%d\\data%d.in",dataPath,GL_problemId,case_);
		sprintf(outFileName,"%s%d%_%d.out",workPath,solutionId,case_);
		sprintf(srcPath, "%s",outFileName);
		sprintf(ansPath, "%s%d\\data%d.out",dataPath,GL_problemId,case_);

		if( (_access(inFileName, 0 )) == -1 )   {
			write_log(INFO,"Test over..");
			break ;
		}
		GL_testcase = case_;

		SQL_updateSolution(solutionId,V_RUN,case_,GL_time-GL_time%10,GL_memory);

		HANDLE hThread_run;
		hThread_run=CreateThread(NULL,NULL,RUN_ProgramThread,NULL,0,NULL);
		if(hThread_run==NULL) {
			write_log(ERROR,"Create thread error");
			CloseHandle(hThread_run);
		}

		DWORD status_ = WaitForSingleObject(hThread_run,GL_time_limit+2000);   //放宽时限2S,返回值大于零说明超时.
		if(status_>0){
			puts("hThread_run TIME LIMIT");
			GL_time = GL_time_limit;
			if(GL_verdictId==V_AC)
			{
				GL_verdictId = V_TLE;
			}
		}

		if(InputFile!=NULL) CloseHandle(InputFile);
		if(OutputFile!=NULL) CloseHandle(OutputFile);

		//get memory info
		PROCESS_MEMORY_COUNTERS   pmc;
		unsigned long tmp_memory=0;

		/*
		 //del for mingw
		if(GetProcessMemoryInfo(G_pi.hProcess,&pmc,sizeof(pmc))) {
			tmp_memory=pmc.PeakWorkingSetSize/1024;
			if(tmp_memory>GL_memory) GL_memory=tmp_memory;
		}
       */

		//get process state
		GetExitCodeProcess(G_pi.hProcess, &g_dwCode);
		if(RUN_existException(g_dwCode))
		{
			GL_verdictId=V_RE;
			goto l;
		}
		else if(g_dwCode==STILL_ACTIVE)
		{	//超时
			puts("TIME LIMIT");
			TerminateProcess(G_pi.hProcess, 0);
			if(GL_verdictId==V_AC)
			{
				//TLE 超时
				GL_verdictId=V_TLE;
				GL_time = GL_time_limit;
				goto l;
			}

		}
		caseTime = endt - startt;
		if(caseTime<0){
			caseTime = GL_time_limit;
		}

		TerminateJobObject(G_job,0);//exit
		CloseHandle(G_pi.hProcess);
		CloseHandle(G_job);G_job=NULL;


		GL_time = (caseTime>GL_time)?caseTime:GL_time;

		if(GL_time>=GL_time_limit){
			GL_verdictId=V_TLE;
			GL_time = GL_time_limit;
			goto l;
		}
		if(GL_memory>=GL_memory_limit){
			GL_verdictId=V_MLE;
			GL_memory = GL_memory_limit;
			goto l;
		}
		//OLE
		if(GL_verdictId!=V_AC){
			goto l;
		}
		//judge file，spj or not

		if(GL_spj==1){
			int verdict_ = special_judge(inFileName,outFileName);
			if(verdict_) GL_verdictId=V_AC;
			else GL_verdictId=V_WA;
		}else{
			GL_verdictId = compare(srcPath,ansPath);
		}

l:		write_log(INFO,"ID:%d Test%d ,%s ,%dms %dkb ,Return code:%u",GL_solutionId,i,VERDICT_NAME[GL_verdictId],caseTime,tmp_memory,g_dwCode);

		/* write judge-log */

		if (i == 0)
		{
			reset_file(judge_log_filename);
			if (GL_verdictId!=V_AC)
			{
				i  = 1;
				write_buffer(judge_log_filename,"Test: #%d, time: %d ms, memory: %d kb, exit code: %d,verdict: %s",
					i,GL_time-GL_time%10,tmp_memory,g_dwCode,VERDICT_NAME[GL_verdictId]);

				memset(buf,0,sizeof(buf));
				read_buffer(inFileName, buf, JUDGE_LOG_BUF_SIZE);
				write_buffer(judge_log_filename,"\nInput\n",i);
				write_buffer(judge_log_filename,buf);

				memset(buf,0,sizeof(buf));
				read_buffer(outFileName, buf, JUDGE_LOG_BUF_SIZE);
				write_buffer(judge_log_filename,"\nOutput\n",i);
				write_buffer(judge_log_filename,buf);

				memset(buf,0,sizeof(buf));
				read_buffer(ansPath, buf, JUDGE_LOG_BUF_SIZE);
				write_buffer(judge_log_filename,"\nAnswer\n");
				write_buffer(judge_log_filename,buf);

				write_buffer(judge_log_filename,"\n------------------------------------------------------------------\n");
				break;
			}
		}
		else
		{
			write_buffer(judge_log_filename,"Test: #%d, time: %d ms, memory: %d kb, exit code: %d,verdict: %s",
				i,caseTime - caseTime%10,tmp_memory,g_dwCode,VERDICT_NAME[GL_verdictId]);

			memset(buf,0,sizeof(buf));
			read_buffer(inFileName, buf, JUDGE_LOG_BUF_SIZE);
			write_buffer(judge_log_filename,"\nInput\n");
			write_buffer(judge_log_filename,buf);

			memset(buf,0,sizeof(buf));
			read_buffer(outFileName, buf, JUDGE_LOG_BUF_SIZE);
			write_buffer(judge_log_filename,"\nOutput\n");
			write_buffer(judge_log_filename,buf);

			memset(buf,0,sizeof(buf));
			read_buffer(ansPath, buf, JUDGE_LOG_BUF_SIZE);
			write_buffer(judge_log_filename,"\nAnswer\n");
			write_buffer(judge_log_filename,buf);

			write_buffer(judge_log_filename,"\n------------------------------------------------------------------\n");
		}

		if(GL_verdictId!=V_AC){
			break;
		}
		if(i==0){
			GL_time=0;
			GL_memory=0;
		}
	}
	return 0;

}

void resetVal(){
	GL_verdictId=V_AC;
	GL_contestId=0;
	GL_time=0;
	GL_memory=0;
	GL_time_limit=1000;
	GL_memory_limit=65535;
	GL_reJudge=0;
	GL_testcase=0;
}

/*
HDU 支持的语言
char gaucLanguageName[][255] = {
	"G++",
	"GCC",
	"C++",
	"C",
	"Pascal",
	"Java"
};
*/
int getHDULangID(int GDOJlangID)
{
	int alang[25] = {0,2,3,0,1,5,0,0,4,0,0,0};

	return alang[GDOJlangID];
}

int Judge_Local()
{
	char buf[4096] = {0};
	write_log(INFO,"Start Compile...");

	if(0 == compile())
	{
		write_log(INFO,"Compile Error...");
		GL_verdictId=V_CE;
		SQL_updateCompileInfo(GL_solutionId);

		reset_file(judge_log_filename);

		write_buffer(judge_log_filename,"Test: #1, time: 0 ms, memory: 0 kb, exit code: 0,verdict: %s\n",VERDICT_NAME[GL_verdictId]);
	}
	else
	{
		write_log(INFO,"Start Run...");
		RUN_solution(GL_solutionId);
	}

	return BOOL_TRUE;
}

/* HDU Virtual Judge */
#if(JUDGE_VIRTUAL == VOS_YES)
ULONG getHDUStatus(string hdu_username, int pid,int lang, string &runid, string &result,string& ce_info,string &tu,string &mu)
{
	ULONG ulRet = BOOL_TRUE;
	tu=mu="0";
	string ts;

	MSG_OUPUT_DBG("Do get status...");

	ts = getLineFromFile(tfilename,77);

	if(BOOL_FALSE == getUsedTime(ts, tu))
	{
		++ulRet;
		MSG_OUPUT_DBG("getUsedTime failed.");
	}

	if(BOOL_FALSE == getUsedMem(ts, mu))
	{
		++ulRet;
		MSG_OUPUT_DBG("getUsedMem failed.");
	}

	if(BOOL_FALSE == getRunid(ts, runid))
	{
		++ulRet;
		MSG_OUPUT_DBG("getRunid failed.");
	}

	if(BOOL_FALSE == getResult(ts, result))
	{
		++ulRet;
		MSG_OUPUT_DBG("getResult failed.");
	}

	MSG_OUPUT_DBG("problem:%d, language:%d, verdict:%s, submissionID:%s, time:%s ms, memory:%s kb\r\n", pid, lang, result.c_str(), runid.c_str(), tu.c_str(), mu.c_str());

	if (BOOL_TRUE != ulRet)
	{
		MSG_OUPUT_DBG("get record failed.");
		return BOOL_FALSE;
	}

	MSG_OUPUT_DBG("get status success...");

	return BOOL_TRUE;
}

int Judge_Remote()
{
	char current_path[MAX_PATH] = {0};
	char tmp_source_path[MAX_PATH] = {0};
	char tmp_return_path[MAX_PATH] = {0};
	GetCurrentDirectory(sizeof(current_path),current_path);

	sprintf(tmp_source_path, "%s//%s",current_path,sourcePath);
	sprintf(tmp_return_path, "%s//OJ_TMP//hdubjudge-%d.tmp",current_path,GL_solutionId);

	strcpy(tfilename,tmp_return_path);

	do
	{
		int lang_id = getHDULangID(GL_languageId);

		char cmd_string[MAX_PATH];
		sprintf(cmd_string,"python -O hdu-vjudge.py submit %d %d %s %s %s %s",
				GL_vpid, lang_id, hdu_username, hdu_password, tmp_source_path,tmp_return_path);
		system(cmd_string) ;

		int tryTime = 6;
		int ret = BOOL_FALSE;
		string runid, result,ce_info,tu,mu;

		//hdu 的status多1
		lang_id += 1;

		while (ret != BOOL_TRUE)
		{
			result = "";
			puts("Get Status...");

			Sleep(10000);

			sprintf(cmd_string,"python -O hdu-vjudge.py status %d %d %s %s",
				GL_vpid, lang_id , hdu_username,tmp_return_path);

			system(cmd_string) ;

			ret =getHDUStatus(hdu_username, GL_vpid, lang_id, runid, result,ce_info,tu,mu);

			if (result.find("Queuing")!=string::npos
				|| result.find("Compiling")!=string::npos
				|| result.find("Running")!=string::npos)
			{
				puts("Get Status, Queuing or Compiling or Running , try again...");
				ret = BOOL_FALSE;
			}

			if (result.find("Compilation Error")!=string::npos)
			{
				//获取编译错误信息
				sprintf(cmd_string,"python -O hdu-vjudge.py ce %s %s",runid.c_str(), tmp_return_path);
				system(cmd_string) ;

				string CE_Info = getCEinfo_brief(tfilename);
				ce_info = CE_Info;
				//MSG_OUPUT_DBG(CE_Info.c_str());
			}

			tryTime --;
			/* 循环等待60s */
			if (0 == tryTime)
			{
				break;
			}
		}

		if (BOOL_FALSE == ret)
		{
			puts("Get Status Error...");
			GL_verdictId = V_SE;
		}
		else
		{
			puts("Get Status success...");
			if (result.find("Accepted")!=string::npos)
			{
				GL_verdictId = V_AC;
			}
			else if (result.find("Presentation Error")!=string::npos)
			{
				GL_verdictId = V_PE;
			}
			else if (result.find("Runtime Error")!=string::npos)
			{
				GL_verdictId = V_RE;
			}
			else if (result.find("Time Limit Exceeded")!=string::npos)
			{
				GL_verdictId = V_TLE;
			}
			else if (result.find("Memory Limit Exceeded")!=string::npos)
			{
				GL_verdictId = V_TLE;
			}
			else if (result.find("Output Limit Exceeded")!=string::npos)
			{
				GL_verdictId = V_OLE;
			}
			else if (result.find("Wrong Answer")!=string::npos)
			{
				GL_verdictId = V_WA;
			}
			else if (result.find("Compilation Error")!=string::npos)
			{
				GL_verdictId = V_CE;
				FILE *fp;
				char buffer[4096]={0};
				if ((fp = fopen (DebugFile, "w")) == NULL){
					write_log(ERROR,"DebugFile open error");
					break;
				}
				fputs(ce_info.c_str(),fp);
				fclose(fp);
				SQL_updateCompileInfo(GL_solutionId);
			}
			else
			{
				GL_verdictId = V_SE;
			}
		}

		GL_time = atoi(tu.c_str());
		GL_memory = atoi(mu.c_str());

	} while (0);

	DeleteFile(tmp_return_path);

	return BOOL_TRUE;
}
#endif

int work(int solutionId){  //开始工作

	GL_solutionId = solutionId;

	resetVal();//重置
	SQL_getSolutionInfo();
	InitPath();  //包含sourcePath,所以在SQL_getSolutionSource之前
	SQL_getSolutionSource();  //取出source，并保存到sourcePath
	SQL_getProblemInfo();	//problem info



	if (1 == GL_vjudge)
	{
		#if(JUDGE_VIRTUAL == VOS_YES)
		(void)Judge_Remote();
		#endif

		g_dwCode = 0;
		GL_testcase = 0;
	}
	else
	{
		GL_time_limit*=limitIndex;
		GL_memory_limit*=limitIndex;
		(void)Judge_Local();

	}

	string time_string_;
	API_TimeToString(time_string_,GL_submitDate);
	//update MySQL............
	write_log(INFO,"ID:%d ->Rusult:%s Case:%d %dms %dkb ,Return code:%u at %s by %s",GL_solutionId,VERDICT_NAME[GL_verdictId],GL_testcase,GL_time-GL_time%10,GL_memory,g_dwCode,time_string_.c_str(),GL_username);
	SQL_updateSolution(GL_solutionId,GL_verdictId,GL_testcase,GL_time-GL_time%10,GL_memory);
	SQL_updateProblem(GL_problemId);
	SQL_updateUser(GL_username);

	//contest or not
	if(GL_contestId > 0)
	{
		//contest judge
		time_t contest_s_time,contest_e_time;
		char num[10]={0};

		/* 获取contest problem题目标号 */
		SQL_getProblemInfo_contest(GL_contestId,GL_problemId,num);
		SQL_getContestInfo(GL_contestId,contest_s_time,contest_e_time);

		if(contest_e_time>GL_submitDate)
		{
			/* 比赛running ，修改Attend */
			SQL_updateAttend_contest(GL_contestId,GL_verdictId,GL_problemId,num,GL_username,contest_s_time,contest_e_time);
		}

		SQL_updateProblem_contest(GL_contestId,GL_problemId);
	}

	DeleteFile(sourcePath);
	DeleteFile(DebugFile);
	DeleteFile(exePath);

	return 1;
}

DWORD WINAPI WorkThread(LPVOID lpParam)
{
	JUDGE_DATA jd;
	time_t first_t,second_t;
	string str_time;
    time(&first_t);
	while(1)
	{
		//cout<<"size"<<Q.size()<<endl;
		if(Q.size()>limitJudge) {int a=0;cout<<2/a<<endl; return 0;} //保护系统,故作此限制,最大接受limitJudge个同时等待,异常退出重启
		if(!Q.empty())
		{
				jd=Q.front();
			  	GL_currentId=jd.solutionId;
				//启动评判
				work(GL_currentId);
				Q.pop();
				//cout<<"judge over"<<endl;
		}
		Sleep(1);
	}
	write_log(ERROR,"WorkThread Crash");
	return 0;
}
DWORD WINAPI ListenThread(LPVOID lpParam)
{
	// 循环接受客户的连接请求
	sockaddr_in remoteAddr;
	SOCKET sClient;
	//初始化客户地址长度
	int nAddrLen = sizeof(remoteAddr);
	JUDGE_DATA j;
	while(TRUE)
	{
		sClient = accept(sListen, (SOCKADDR*)&remoteAddr, &nAddrLen);
		if(sClient == INVALID_SOCKET){
			write_log(ERROR,"Accept() Error");
			continue;
		}
		int ret=recv(sClient,(char*)&j,sizeof(j),0);
		if(ret>0){
			write_log(INFO,"Push SolutionId:%d into Judge Queue....",j.solutionId);
			Q.push(j);
		}
		Sleep(1);
	}
	write_log(ERROR,"ListenThread Crash");
	closesocket(sClient);
	return 0;
}

long WINAPI ExceptionFilter(EXCEPTION_POINTERS * lParam)
{
	write_log(ERROR,"System Error! \r\n System may restart after 2 seconds...");
	Sleep(2000);
	ShellExecuteA(NULL,"open",judgePath,NULL,NULL,SW_SHOWNORMAL);
	return EXCEPTION_EXECUTE_HANDLER;
}

int OJ_main()
{
	#if 0
	if (argc > 1)
	{
		strcpy(INI_filename,argv[1]);
	}
	#endif

	//关闭调试开关
	DLL_HDUDebugSwitch(1);

	write_log(INFO,"Running Judge Core...");
	InitMySqlConfig();
	if(InitMySQL()==0){//初始化mysql
		write_log(ERROR,"Init MySQL ERROR...");
		return 0;
	}
	write_log(INFO,"Init MySQL Success...");
	if(initSocket()==0){         // 初始化socket
		write_log(ERROR,"Init Socket ERROR...");
		return 0;
	}

	//创建工作线程
	HANDLE hThreadW;
	hThreadW=CreateThread(NULL,NULL,WorkThread,0,0,0);
	//进入循环，等待客户连接请求
	HANDLE hThreadR=CreateThread(NULL,NULL,ListenThread,0,0,0);

	write_log(INFO,"Judge Task init ok...");

	while(TRUE)
	{
		Sleep(1);
	}

	closesocket(sListen);
	WSACleanup();
}

void OJ_TaskEntry()
{
	SetUnhandledExceptionFilter(ExceptionFilter);
 	SetErrorMode(SEM_NOGPFAULTERRORBOX );

	if( (_access(logPath, 0 )) == -1 )   {
		CreateDirectory(logPath,NULL);
	}

	(void)OJ_main();

}

#if 0
int main(int argc, char **argv)
{
	OJ_TaskEntry();
	return 0;
}
#endif
