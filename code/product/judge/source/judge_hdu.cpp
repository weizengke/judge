/*

适配HDOJ virtual-judge
Author: Jungle Wei
Create Date: 2012-05-12

*/


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


#include "product\judge\include\judge_inc.h"


using namespace std;


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

string g_HDU_problem_string[PROBLEM_TAG_MAX];
char g_HDUtmps[VJUDGE_MAX_SIZE_BUF];

/* hdu language list */
UCHAR gaucHDULanguageName[][VJUDGE_MAX_LANG_SIZE] = {
	"G++",
	"GCC",
	"C++",
	"C",
	"Pascal",
	"Java"
};


/* HDU VJUDGE */
char hdu_domain[256] = "http://acm.hdu.edu.cn";
char hdu_username[1000]={0};
char hdu_password[1000]={0};
char hdu_judgerIP[20]="127.0.0.1";
int hdu_sockport = 0;
int hdu_remote_enable=OS_NO;
int hdu_vjudge_enable=OS_NO;

#if(JUDGE_VIRTUAL == VOS_YES)

ULONG getLanguageNameByID(ULONG id, UCHAR *ucLanguageName)
{
	if (id < 0 || id >= sizeof(gaucHDULanguageName)/VJUDGE_MAX_LANG_SIZE)
	{
		return OS_FALSE;
	}

	strcpy((char *)ucLanguageName, (char *)gaucHDULanguageName[id]);
	return OS_TRUE;
}

ULONG getLanguageIDByName(UCHAR *ucLanguageName, ULONG *id)
{
	USHORT usLoop = 0;

	for (usLoop = 0; usLoop <= sizeof(gaucHDULanguageName)/VJUDGE_MAX_LANG_SIZE; ++usLoop)
	{
		if (strcmp((CHAR*)ucLanguageName, (CHAR*)gaucHDULanguageName[usLoop]) == 0)
		{
			*id = usLoop;
			return OS_TRUE;
		}
	}

	return OS_FALSE;
}

bool isSpace(char c)
{
	if(c==' '||c=='\n'||c=='\t')
	{
		return true;
	}
	return false;
}


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
    while (fgets(g_HDUtmps,1000000,fp)) res+=g_HDUtmps;
    fclose(fp);
    return res;
}

size_t process_data(void *buffer, size_t size, size_t nmemb, void *user_p)
{
	FILE *fp = (FILE *)user_p;
	size_t return_size = fwrite(buffer, size, nmemb, fp);
	return return_size;
}

ULONG getResult(string s, string &res)
{
    int pos=s.find("<font color=");
	if (-1 == pos)
	{
		return OS_FALSE;
	}

    while (s[pos]!='>') pos++;
    pos++;

    int st=pos;
    while (s[pos]!='<') pos++;
    res = s.substr(st,pos-st);

	return OS_TRUE;
}

ULONG getRunid(string s, string &res) {
    int pos=s.find("<td height=22px>");
	if (-1 == pos)
	{
		return OS_FALSE;
	}

    while (s[pos]!='>') pos++;
    pos++;

    int st=pos;
    while (s[pos]!='<') pos++;

    res = s.substr(st,pos-st);

	return OS_TRUE;
}


string getCEinfo_brief(char *filename)
{
	string res="",ts;
    FILE * fp=fopen(filename,"r");

    while (fgets(g_HDUtmps,1000000,fp))
    {
        ts=g_HDUtmps;
        if (ts.find("View Compilation Error")!=string::npos)
        {
            while (fgets(g_HDUtmps,1000000,fp))
			{
                ts=g_HDUtmps;
				int pos = ts.find("<pre>");
                if (pos !=string::npos)
				{
					res = ts.substr(pos + 5, ts.length() - pos - 5);

					while (fgets(g_HDUtmps,1000000,fp))
					{
						ts=g_HDUtmps;
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



ULONG getUsedTime(string s, string &timeuse)
{
    int pos=s.find("MS</td>");
	if (-1 == pos)
	{
		return OS_FALSE;
	}

    int st=pos;
    while (s[pos]!='>') pos--;
    pos++;

    timeuse =  s.substr(pos,st-pos);
	return OS_TRUE;
}

ULONG getUsedMem(string s, string &memuse)
{
	int pos=s.find("K</td>");
	if (-1 == pos)
	{
		return OS_FALSE;
	}

	int st=pos;
	while (s[pos]!='>') pos--;
	pos++;
	memuse = s.substr(pos,st-pos);
	return OS_TRUE;
}

string getLineFromFile(char *filename,int line)
{
    string res="";
    FILE * fp=fopen(filename,"r");
    int cnt=0;
    while (fgets(g_HDUtmps,10000000,fp))
	{
        cnt++;
        res=g_HDUtmps;
        if (res.find("<h1>Realtime Status</h1>")!=string::npos)
		{
            fgets(g_HDUtmps,10000000,fp);
            res=res+g_HDUtmps;
            fgets(g_HDUtmps,10000000,fp);
            res=res+g_HDUtmps;
            break;
        }
    }
    fclose(fp);
    return res;
}

/* HDU Virtual Judge */
/*
HDU 支持的语言
char gaucHDULanguageName[][255] = {
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

#if(VJUDGE_CURL==VOS_YES)

struct curl_slist *headerlist=NULL;

int HDU_loginEx(char *uname, char *pdw)
{
    FILE * fp=fopen(g_Vjudgetfilename,"w+");
	CURL *curl;
	CURLcode res;

    curl = curl_easy_init();

	judge_outstring("Info: It will take a few seconds, please wait [%s]...",uname);
	MSG_StartDot();
	if(curl)
	{
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_data);
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "hdu.cookie");
        char url[255] = {0};
		sprintf(url, "%s/userloginex.php?action=login",hdu_domain);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        string post=(string)"username="+uname+"&userpass="+pdw+"&login=Sign+In";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());

		res = curl_easy_perform(curl);
        
        Judge_Debug(DEBUG_TYPE_FUNC, "Post request to HDU. (url=%s, parama:%s, res=%d)", 
                        url, post.c_str(), res);

		curl_easy_cleanup(curl);
    }

	fclose(fp);

	MSG_StopDot();
	judge_outstring("done.\r\n");

    if (res) return OS_FALSE;

    string ts=getAllFromFile(g_Vjudgetfilename);
    if (ts.find("No such user or wrong password.")!=string::npos)
	{
		judge_outstring("Error: No such user or wrong password.\r\n");
		return OS_FALSE;
	}

	judge_outstring("Info: Login hdu-judge success.\r\n");
    return OS_TRUE;
}

ULONG HDU_login()
{
    FILE * fp=fopen(g_Vjudgetfilename,"w+");
	CURL *curl;
	CURLcode res;

    curl = curl_easy_init();

	MSG_OUPUT_DBG("Do login hduoj, it make takes a few seconds, please wait...[%s, %s]",hdu_username,hdu_password);

	if(curl)
	{
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_data);
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "hdu.cookie");
        char url[255] = {0};
		sprintf(url, "%s/userloginex.php?action=login",hdu_domain);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        string post=(string)"username="+hdu_username+"&userpass="+hdu_password+"&login=Sign+In";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());

		res = curl_easy_perform(curl);

        Judge_Debug(DEBUG_TYPE_FUNC, "Post request to HDU. (url=%s, parama:%s, res=%d)", 
                        url, post.c_str(), res);
        
		curl_easy_cleanup(curl);
    }


	fclose(fp);

    if (res) return OS_FALSE;

    string ts=getAllFromFile(g_Vjudgetfilename);
    if (ts.find("No such user or wrong password.")!=string::npos)
	{
		MSG_OUPUT_DBG("Login failed.");
		return OS_FALSE;
	}
    return OS_TRUE;
}

ULONG getSubmitError(char *filename, string &res)
{
	string ts;
	res = "";
    FILE * fp=fopen(filename,"r");
	int begin_ = 0;
	int end_ = 0;
    while (fgets(g_HDUtmps,1000000,fp))
    {
        ts=g_HDUtmps;
        if (ts.find("<form id=\"submit\" name=\"submit\"")!=string::npos)
        {
            while (fgets(g_HDUtmps,1000000,fp))
			{
                ts=g_HDUtmps;
				begin_ = ts.find("<span>");
                if (begin_!=string::npos)
				{
					end_ = ts.find("</span>");
					if (end_ !=string::npos)
					{
						begin_ += 6;
						res = ts.substr(begin_,end_ - begin_);
						fclose(fp);
						return OS_TRUE;
					}

					while (fgets(g_HDUtmps,1000000,fp))
					{
						ts=g_HDUtmps;
						end_ = ts.find("</span>");
						if (end_ !=string::npos)
						{
							begin_ += 6;
							res = ts.substr(begin_,end_ - begin_);
							fclose(fp);
							return OS_TRUE;
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
    return OS_FALSE;
}

ULONG HDU_submit(string pid, string lang, string source)
{
	CURL *curl;
	CURLcode res;
	FILE * fp=fopen(g_Vjudgetfilename,"w+");
	if (NULL == fp)
	{
		MSG_OUPUT_DBG("Open %s failed...", g_Vjudgetfilename);
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
        char url[255] = {0};
		sprintf(url, "%s/submit.php?action=submit",hdu_domain);
        curl_easy_setopt(curl, CURLOPT_URL, url);
		string post= (string)"check=0&problemid=" + pid + "&language=" + lang + "&usercode=" + escapeURL(source);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

        res = curl_easy_perform(curl);

        Judge_Debug(DEBUG_TYPE_FUNC, "Post request to HDU. (url=%s, res=%d)", 
                        url, res);
        
        curl_easy_cleanup(curl);
    }

	curl_slist_free_all (headerlist);

	fclose(fp);

	if (res)
	{
		MSG_OUPUT_DBG("curl_easy_perform failed...");
		return OS_FALSE;
	}

    string tss=getAllFromFile(g_Vjudgetfilename);
    if (tss.find("Connect(0) to MySQL Server failed.")!=string::npos||tss.find("<b>One or more following JUDGE_ERROR(s) occurred.")!=string::npos||tss.find("<h2>The requested URL could not be retrieved</h2>")!=string::npos||tss.find("PHP: Maximum execution time of")!=string::npos)
	{
		MSG_OUPUT_DBG("One or more JUDGE_ERROR(s) occurred.....");
		return OS_FALSE;
	}

	MSG_OUPUT_DBG("Submit success...");

    return OS_TRUE;
}
#endif

#if(VJUDGE_CURL==VOS_YES)
string getCEinfo(string runid)
{
	FILE *fp = fopen(g_Vjudgetfilename, "ab+");
	CURL *curl;
	CURLcode res;

    curl = curl_easy_init();
    if(curl)
    {
		curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "hdu.cookie");
        char url[255] = {0};
        sprintf(url, "%s/viewerror.php?rid=%s", 
		hdu_domain,runid.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, url);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        res = curl_easy_perform(curl);

        Judge_Debug(DEBUG_TYPE_FUNC, "Post request to HDU. (url=%s, res=%d)", 
                        url, res);
        
        curl_easy_cleanup(curl);
    }

    fclose(fp);

    string info = getCEinfo_brief(g_Vjudgetfilename);
    return info;
}

int getStatusEx(char *hdu_username)
{
	string pid = "1000";
	string lang = "C++";
	string runid;
	string result;
	string ce_info;
	string tu;
	string mu;
    ULONG ulRet = OS_TRUE;

    tu=mu="0";
    string ts;

	judge_outstring("Info: It will take a few seconds, please wait...");
	MSG_StartDot();

	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();

    if ( curl )
	{
		FILE *fp = fopen(g_Vjudgetfilename, "w+");
		curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "hdu.cookie");
		char url[255] = {0};
		sprintf(url, "%s/status.php?first=&pid=&user=%s&lang=&status=0", 
		hdu_domain,hdu_username);
		curl_easy_setopt( curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

		res = curl_easy_perform( curl );
        Judge_Debug(DEBUG_TYPE_FUNC, "Post request to HDU. (url=%s, res=%d)", 
                        url, res);        
		curl_easy_cleanup(curl);

		fclose(fp);
	}

	ts = getLineFromFile(g_Vjudgetfilename,77);

	if(OS_FALSE == getUsedTime(ts, tu))
	{
		++ulRet;
		MSG_OUPUT_DBG("getUsedTime failed.");
	}

    if(OS_FALSE == getUsedMem(ts, mu))
	{
		++ulRet;
		MSG_OUPUT_DBG("getUsedMem failed.");
	}

	if(OS_FALSE == getRunid(ts, runid))
	{
		++ulRet;
		MSG_OUPUT_DBG("getRunid failed.");
	}

	if(OS_FALSE == getResult(ts, result))
	{
		++ulRet;
		MSG_OUPUT_DBG("getResult failed.");
	}

	MSG_StopDot();
	judge_outstring("done.\r\n");

	if (OS_TRUE != ulRet)
	{
		judge_outstring("Error: Get status failed [%s].\r\n", hdu_username);
		MSG_OUPUT_DBG("get record failed.");
		return OS_FALSE;
	}

	judge_outstring("Problem[%s] language[%s] verdict[%s] submissionID[%s] time[%sms] memory[%skb].\r\n", pid.c_str(), lang.c_str(), result.c_str(), runid.c_str(), tu.c_str(), mu.c_str());

	MSG_OUPUT_DBG("get status success...");

	if (result.find("Compilation Error")!=string::npos)
	{
		//获取编译错误信息
		string CE_Info = getCEinfo(runid);
		ce_info = CE_Info;

		judge_outstring("Compilation Info: \r\n%s\r\n", ce_info.c_str());
		//MSG_OUPUT_DBG(CE_Info.c_str());
	}

    return OS_TRUE;
}

ULONG getStatus(string hdu_username, string pid,string lang, string &runid, string &result,string& ce_info,string &tu,string &mu)
{
    ULONG ulRet = OS_TRUE;
    tu=mu="0";
    string ts;

	MSG_OUPUT_DBG("Do get status...");

	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();

    if ( curl )
	{
		FILE *fp = fopen(g_Vjudgetfilename, "w+");
		curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "hdu.cookie");
		char url[255] = {0};
		sprintf(url, "%s/status.php?first=&pid=%s&user=%s&lang=&status=0",
            hdu_domain,pid.c_str(), hdu_username.c_str());

		//MSG_OUPUT_DBG(url);

		curl_easy_setopt( curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

		res = curl_easy_perform( curl );

        Judge_Debug(DEBUG_TYPE_FUNC, "Post request to HDU. (url=%s, res=%d)", 
                        url, res);
        
		curl_easy_cleanup(curl);

		fclose(fp);
	}

	ts = getLineFromFile(g_Vjudgetfilename,77);

	if(OS_FALSE == getUsedTime(ts, tu))
	{
		++ulRet;
		MSG_OUPUT_DBG("getUsedTime failed.");
	}

    if(OS_FALSE == getUsedMem(ts, mu))
	{
		++ulRet;
		MSG_OUPUT_DBG("getUsedMem failed.");
	}

	if(OS_FALSE == getRunid(ts, runid))
	{
		++ulRet;
		MSG_OUPUT_DBG("getRunid failed.");
	}

	if(OS_FALSE == getResult(ts, result))
	{
		++ulRet;
		MSG_OUPUT_DBG("getResult failed.");
	}

	if (OS_TRUE != ulRet)
	{
		MSG_OUPUT_DBG("get record failed.");
		return OS_FALSE;
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

    return OS_TRUE;
}

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
			return OS_FALSE;
		default:
			return OS_TRUE;
	}
	return OS_TRUE;
}


void SQL_updateProblemInfo(string v_ojname, string v_pid)
{
	string val_str="";
	/*
	val_str = g_HDU_problem_string[0] + "," + g_HDU_problem_string[1]+ "," + "'" + g_HDU_problem_string[2] + "'" + "," +
	"'" + g_HDU_problem_string[3] + "'" + "," + "'" + g_HDU_problem_string[4] + "'" + "," + "'" + g_HDU_problem_string[5] + "'" + "," +
	"'" + g_HDU_problem_string[6] + "'" + "," + "'" + g_HDU_problem_string[7] + "'" + "," + "'" + g_HDU_problem_string[8] + "'" + "," +
	"'" + getCurrentTime() + "', 'N', 0,0,0,0,0,1, '" + v_ojname +"', " + v_pid + "";
    */

	MSG_OUPUT_DBG("In SQL_updateProblemInfo, (%s)", v_pid.c_str());

	for(int i=0; i<PROBLEM_TAG_MAX; i++)
	{

		//char *end;
		//char *string_ = (char*)malloc(sizeof(char)*g_HDU_problem_string[i].length()+1);

		//strcpy(string_,g_HDU_problem_string[i].c_str());
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
			replace_all_distinct(g_HDU_problem_string[i], "\"", " ");
			g_HDU_problem_string[i] = "HDU." + v_pid + " - " + g_HDU_problem_string[i];
		}

		if (OS_TRUE == isNeed2HTML((ENUM_PROVLEM)i))
		{
			replace_all_distinct(g_HDU_problem_string[i], "\"", "&quot;");
			replace_all_distinct(g_HDU_problem_string[i], "src=/data/images/", "src=http://acm.hdu.edu.cn/data/images/");
			replace_all_distinct(g_HDU_problem_string[i], "src=../../data/images/", "src=http://acm.hdu.edu.cn/data/images/");
			replace_all_distinct(g_HDU_problem_string[i], "\n", "<br>");
		}
		//val_str += g_HDU_problem_string[i];
	}

	val_str = g_HDU_problem_string[0] + "," + g_HDU_problem_string[1]+ "," + "\"" + g_HDU_problem_string[2] + "\"" + "," +
		"\"" + g_HDU_problem_string[3] + "\"" + "," + "\"" + g_HDU_problem_string[4] + "\"" + "," + "\"" + g_HDU_problem_string[5] + "\"" + "," +
		"\"" + g_HDU_problem_string[6] + "\"" + "," + "\"" + g_HDU_problem_string[7] + "\"" + "," + "\"" + g_HDU_problem_string[8] + "\"" + "," +
		"'" + getCurrentTime() + "', 'N', 0,0,0,0,0,0,1, '" + v_ojname +"', " + v_pid + "";

	if (val_str.length() >= VJUDGE_MAX_SIZE_BUF)
	{
		MSG_OUPUT_DBG("JUDGE_ERROR, too large size of buffer...");
		return;
	}

	sprintf(query,"insert into problem(time_limit,memory_limit,title,description,input,output,sample_input,sample_output,author,create_date,defunct,spj,accepted,solved,submit,submit_user,contest_id,isvirtual,oj_name,oj_pid) values(%s);",val_str.c_str());

	//MSG_OUPUT_DBG(query);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		//write_log(JUDGE_ERROR,mysql_error(mysql));
		MSG_OUPUT_DBG(mysql_error(mysql));
		return ;
	}

	MSG_OUPUT_DBG("End SQL_updateProblemInfo OK, (%s)", v_pid.c_str());
}

ULONG checkStringExsit(char *filename, char *pattern)
{
    pcre  *re;
    const char *error;
    int  erroffset;
    int  ovector[VJUDGE_OVECCOUNT];
    int  rc;

	string ts;
    FILE * fp=fopen(filename,"r");

    while (fgets(g_HDUtmps, VJUDGE_MAX_SIZE_BUF, fp))
    {
        ts +=g_HDUtmps;
    }

    fclose(fp);

	//title
	re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
    if (re == NULL) {                 //如果编译失败，返回错误信息
        MSG_OUPUT_DBG("PCRE compilation failed at offset %d: %s\n", erroffset, error);
        return OS_FALSE;
    }

    rc = pcre_exec(re,NULL, ts.c_str(), strlen(ts.c_str()), 0, 0, ovector, VJUDGE_OVECCOUNT);
	// 返回值：匹配成功返回非负数，没有匹配返回负数
    if (rc < 0) {                     //如果没有匹配，返回错误信息
        if (rc == PCRE_ERROR_NOMATCH) MSG_OUPUT_DBG("Sorry, no match ...\n");
        else MSG_OUPUT_DBG("Matching error %d\n", rc);
        pcre_free(re);

        return OS_FALSE;
    }

	pcre_free(re);

	return OS_TRUE;
}

ULONG getInfoByTag(char *src, char *pattern, ENUM_PROVLEM enProblem, char *res)
{
    pcre  *re;
    const char *error;
    int  erroffset;
    int  ovector[VJUDGE_OVECCOUNT];
    int  rc, i;

	MSG_OUPUT_DBG("In getInfoByTag...");

	//title
	re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
    if (re == NULL) {                 //如果编译失败，返回错误信息
        MSG_OUPUT_DBG("PCRE compilation failed at offset %d: %s\n", erroffset, error);
        return OS_FALSE;
    }

    rc = pcre_exec(re,NULL, src, strlen(src), 0, 0, ovector, VJUDGE_OVECCOUNT);
	// 返回值：匹配成功返回非负数，没有匹配返回负数
    if (rc < 0) {                     //如果没有匹配，返回错误信息
		if (rc == PCRE_ERROR_NOMATCH) MSG_OUPUT_DBG("Sorry, no match ...\n");
		else {
			MSG_OUPUT_DBG("Matching error %d\n", rc);
			g_HDU_problem_string[enProblem] = "Not Found";
		}
		pcre_free(re);
		return OS_FALSE;
	}

	MSG_OUPUT_DBG("In getInfoByTag...");

	i = (rc==0)?(0):(rc-1);

//	printf("iiiiiiii=%d , rc=%d\n",i,rc);

//	for (i = 0; i < rc; i++) //分别取出捕获分组 $0整个正则公式 $1第一个()
	{
        char *substring_start =  src + ovector[2*i];
        int substring_length = ovector[2*i+1] - ovector[2*i];
        	MSG_OUPUT_DBG("In getInfoByTag 1 substring_length=%d...",substring_length);
		char *str_tmp = (char*)malloc(sizeof(char)*substring_length+100);
		//	char str_tmp[VJUDGE_MAX_SIZE_BUF] ={0};
			MSG_OUPUT_DBG("In getInfoByTag 2...");
		sprintf(str_tmp, "%.*s\n", substring_length, substring_start);

			MSG_OUPUT_DBG("In getInfoByTag 3...");

		//	printf("%s",str_tmp);

		//string string_ = str_tmp;
			MSG_OUPUT_DBG("In getInfoByTag 4...(length = %d)", strlen(str_tmp));

			g_HDU_problem_string[enProblem].assign(str_tmp,strlen(str_tmp));

			MSG_OUPUT_DBG("End getInfoByTag success...");

		//MSG_OUPUT_DBG(pattern);
		//MSG_OUPUT_DBG(string_.c_str());
		//	free(substring_start);
			free(str_tmp);
    }


	pcre_free(re);

	return OS_TRUE;
}

int getProblemInfo_Brief(string pid)
{
	ULONG ulRet = 0;
	int loop = 0;
	string res="",ts;
    FILE * fp=fopen(g_Vjudgetfilename,"r");

    while (fgets(g_HDUtmps, VJUDGE_MAX_SIZE_BUF, fp))
    {
        ts +=g_HDUtmps;
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

	for (loop = 0; loop < PROBLEM_TAG_MAX; loop++)
	{
		g_HDU_problem_string[loop] = "";
	}

	MSG_OUPUT_DBG("Start Problem %s ...", pid.c_str());

	MSG_OUPUT_DBG("Time");
	ulRet = getInfoByTag((char*)ts.c_str(), patternTime, PROBLEM_TIME ,NULL);
	if(ulRet == 0)
	{
		g_HDU_problem_string[0] = "1000";
	}

	MSG_OUPUT_DBG("Memory");
	ulRet = getInfoByTag((char*)ts.c_str(), patternMemory, PROBLEM_MEMORY, NULL);
	if(ulRet == 0)
	{
		g_HDU_problem_string[1] = "65535";
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
		if (OS_TRUE == checkStringExsit(g_Vjudgetfilename, "No such problem")
			|| OS_TRUE == checkStringExsit(g_Vjudgetfilename, "<DIV>Invalid Parameter.</DIV>"))
		{
			Judge_Debug(DEBUG_TYPE_ERROR, "No such problem %s on hdu-judge", pid.c_str());

			return OS_FALSE;
		}
	}



	SQL_updateProblemInfo("HDU",pid);

	MSG_OUPUT_DBG("Get Problem %s OK.", pid.c_str());

    return OS_TRUE;
}

ULONG getProblemInfo(string pid)
{
	CURL *curl;
    CURLcode res;
	int ret = OS_TRUE;

	curl = curl_easy_init();

	if (access(g_Vjudgetfilename, 0) == 0)
	{
		DeleteFile(g_Vjudgetfilename);
	}

    if ( curl ) {
		FILE *fp = fopen(g_Vjudgetfilename, "ab+");
		curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "hdu.cookie");
		char url[255] = {0};
		sprintf(url, "%s/showproblem.php?pid=%s", hdu_domain,pid.c_str());
		//cout<<url;
		curl_easy_setopt( curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

		res = curl_easy_perform( curl );
		curl_easy_cleanup(curl);
		fclose(fp);
	}

	ret = getProblemInfo_Brief(pid);

	return ret;
}

int DLL_GetProblemInfoFromHDU(int pid)
{
	char tmp[10]={0};
	itoa(pid,tmp,10);
	string pid_s = tmp;

	if (OS_TRUE != getProblemInfo(pid_s))
	{
		return OS_FALSE;
	}

	return OS_TRUE;
}


ULONG DLL_HDULogin()
{
	if (OS_TRUE != HDU_login())
	{
		return OS_FALSE;
	}

	return OS_TRUE;
}

ULONG DLL_HDUSubmit(int pid, int langid, string source)
{
	char tmp[10]={0};
	itoa(pid,tmp,10);
	string pid_s = tmp;

	char tmplang[10]={0};
	itoa(langid,tmplang,10);
	string lang_string = tmplang;

	if (OS_TRUE != HDU_submit(pid_s, lang_string, source))
	{
		return OS_FALSE;
	}
	return OS_TRUE;
}

ULONG DLL_HDUGetStatus(string hdu_username, int pid, int langid, string &runid, string &result,string& ce_info,string &tu,string &mu)
{
	char tmp[10]={0};
	itoa(pid,tmp,10);
	string pid_s = tmp;

	char tmplang[10]={0};
	itoa(langid,tmplang,10);
	string lang_string = tmplang;

	if (OS_TRUE != getStatus(hdu_username, pid_s, lang_string, runid, result, ce_info, tu, mu))
	{
		MSG_OUPUT_DBG("DLL_HDUGetStatus getStatus error...");
		return OS_FALSE;
	}
	else
	{
		MSG_OUPUT_DBG("DLL_HDUGetStatus getStatus success...");
	}

	return OS_TRUE;
}

#endif

ULONG getHDUStatus(string hdu_username, int pid,int lang, string &runid, string &result,string& ce_info,string &tu,string &mu)
{
	ULONG ulRet = OS_TRUE;
	tu=mu="0";
	string ts;

	MSG_OUPUT_DBG("Do get status...");

	ts = getLineFromFile(g_Vjudgetfilename,77);

	if(OS_FALSE == getUsedTime(ts, tu))
	{
		++ulRet;
		MSG_OUPUT_DBG("getUsedTime failed.");
	}

	if(OS_FALSE == getUsedMem(ts, mu))
	{
		++ulRet;
		MSG_OUPUT_DBG("getUsedMem failed.");
	}

	if(OS_FALSE == getRunid(ts, runid))
	{
		++ulRet;
		MSG_OUPUT_DBG("getRunid failed.");
	}

	if(OS_FALSE == getResult(ts, result))
	{
		++ulRet;
		MSG_OUPUT_DBG("getResult failed.");
	}

	judge_outstring("Info: problem[%d] language[%d]  verdict[%s] submissionID[%s] time[%s ms] memory[%s kb].\r\n", pid, lang, result.c_str(), runid.c_str(), tu.c_str(), mu.c_str());

	if (OS_TRUE != ulRet)
	{
		MSG_OUPUT_DBG("get record failed.");
		return OS_FALSE;
	}

	MSG_OUPUT_DBG("Get status success...");

	return OS_TRUE;
}

int Judge_Via_CurlLib(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	int ret = OS_TRUE;
	char current_path[MAX_PATH] = {0};
	char tmp_source_path[MAX_PATH] = {0};
	char tmp_return_path[MAX_PATH] = {0};
	GetCurrentDirectory(sizeof(current_path),current_path);

	sprintf(tmp_source_path, "%s//%s",current_path,pstJudgeSubmission->sourcePath);
	sprintf(tmp_return_path, "%s//OJ_TMP//hdujudge-%d.tmp",current_path,pstJudgeSubmission->stSolution.solutionId);
	do
	{

		ret = DLL_HDULogin();
		if (OS_TRUE != ret)
		{
			Judge_Debug(DEBUG_TYPE_ERROR, "Error: Login hdu-judge failed.");
            write_log(JUDGE_ERROR, "HDU login failed. (solutionId=%u)",
				pstJudgeSubmission->stSolution.solutionId);
			return OS_ERR;
		}

        write_log(JUDGE_INFO, "HDU login ok. (solutionId=%u)",
				pstJudgeSubmission->stSolution.solutionId);
	
		/* get source , just get 0xFFFFFF size */
		string source_ = "";
		int lang_id = getHDULangID(pstJudgeSubmission->stSolution.languageId);

		int cnt_ = 0xFFFF;
		FILE * fp=fopen(pstJudgeSubmission->sourcePath,"r");
    	while (fgets(g_HDUtmps,0xFFFFFF,fp))
    	{
			source_+=g_HDUtmps;
			if (cnt_-- <=0) break;
		}

	    fclose(fp);

        write_log(JUDGE_INFO, "HDU get source ok. (solutionId=%u)",
				pstJudgeSubmission->stSolution.solutionId);

		ret = DLL_HDUSubmit(pstJudgeSubmission->stProblem.virtualPID, lang_id, source_);
		if (OS_TRUE != ret)
		{
			Judge_Debug(DEBUG_TYPE_ERROR, "Error: Submit solution to hdu-judge failed.");
            write_log(JUDGE_INFO, "HDU submit failed. (solutionId=%u, virtualPID=%u)",
				pstJudgeSubmission->stSolution.solutionId,
				pstJudgeSubmission->stProblem.virtualPID);
			return OS_ERR;
		}

        write_log(JUDGE_INFO, "HDU submit ok. (solutionId=%u, virtualPID=%u)",
				pstJudgeSubmission->stSolution.solutionId,
				pstJudgeSubmission->stProblem.virtualPID);
        
		/* get status */
		string runid, result,ce_info,tu,mu;
		int tryTime = 6;
		//hdu 的status多1
		lang_id += 1;
		ret = OS_FALSE;
		while (ret != OS_TRUE)
		{
			result = "";
			MSG_OUPUT_DBG("Get Status...");

			Sleep(10000);

			ret =DLL_HDUGetStatus(hdu_username, pstJudgeSubmission->stProblem.virtualPID, lang_id, runid, result,ce_info,tu,mu);
			if (ret != OS_TRUE
				||result.find("Queuing")!=string::npos
				|| result.find("Compiling")!=string::npos
				|| result.find("Running")!=string::npos)
			{
				Judge_Debug(DEBUG_TYPE_FUNC, "Get Status, Queuing or Compiling or Running , try again...");
                write_log(JUDGE_INFO, "HDU get status failed. (solutionId=%u, virtualPID=%u)",
                        pstJudgeSubmission->stSolution.solutionId,
                        pstJudgeSubmission->stProblem.virtualPID);

                ret = OS_FALSE;
			}

            write_log(JUDGE_INFO, "HDU get status ok. (solutionId=%u, virtualPID=%u)",
                    pstJudgeSubmission->stSolution.solutionId,
                    pstJudgeSubmission->stProblem.virtualPID);
                

			if (result.find("Compilation Error")!=string::npos)
			{
				//获取编译错误信息
				string  CE_Info = getCEinfo(runid);
				ce_info = CE_Info;
			}

			tryTime --;
			/* 循环等待60s */
			if (0 == tryTime)
			{
				break;
			}
		}

		if (OS_FALSE == ret)
		{
			MSG_OUPUT_DBG("Get Status Error...");
			pstJudgeSubmission->stSolution.verdictId = V_SK;
		}
		else
		{
			MSG_OUPUT_DBG("Get Status success...");
			if (result.find("Accepted")!=string::npos)
			{
				pstJudgeSubmission->stSolution.verdictId = V_AC;
			}
			else if (result.find("Presentation Error")!=string::npos)
			{
				pstJudgeSubmission->stSolution.verdictId = V_PE;
			}
			else if (result.find("Runtime Error")!=string::npos)
			{
				pstJudgeSubmission->stSolution.verdictId = V_RE;
			}
			else if (result.find("Time Limit Exceeded")!=string::npos)
			{
				pstJudgeSubmission->stSolution.verdictId = V_TLE;
			}
			else if (result.find("Memory Limit Exceeded")!=string::npos)
			{
				pstJudgeSubmission->stSolution.verdictId = V_TLE;
			}
			else if (result.find("Output Limit Exceeded")!=string::npos)
			{
				pstJudgeSubmission->stSolution.verdictId = V_OLE;
			}
			else if (result.find("Wrong Answer")!=string::npos)
			{
				pstJudgeSubmission->stSolution.verdictId = V_WA;
			}
			else if (result.find("Compilation Error")!=string::npos)
			{
				pstJudgeSubmission->stSolution.verdictId = V_CE;
				FILE *fp;
				char buffer[4096]={0};
				if ((fp = fopen (pstJudgeSubmission->DebugFile, "w")) == NULL){
					write_log(JUDGE_ERROR,"DebugFile open error");
					break;
				}
				fputs(ce_info.c_str(),fp);
				fclose(fp);
				SQL_updateCompileInfo(pstJudgeSubmission);
			}
			else
			{
				pstJudgeSubmission->stSolution.verdictId = V_SK;
			}
		}

		pstJudgeSubmission->stSolution.time_used= atoi(tu.c_str());
		pstJudgeSubmission->stSolution.memory_used= atoi(mu.c_str());

	}while(0);

	DeleteFile(g_Vjudgetfilename);

	return OS_OK;
}

#if 0
int Judge_Via_python()
{
	char current_path[MAX_PATH] = {0};
	char tmp_source_path[MAX_PATH] = {0};
	char tmp_return_path[MAX_PATH] = {0};
	GetCurrentDirectory(sizeof(current_path),current_path);

	sprintf(tmp_source_path, "%s//%s",current_path,sourcePath);
	sprintf(tmp_return_path, "%s//OJ_TMP//hdujudge-%d.tmp",current_path,GL_solutionId);

	strcpy(g_Vjudgetfilename,tmp_return_path);

	do
	{
		int lang_id = getHDULangID(GL_languageId);

		char cmd_string[MAX_PATH];
		sprintf(cmd_string,"python -O hdu-vjudge.py submit %d %d %s %s %s %s",
				GL_vpid, lang_id, hdu_username, hdu_password, tmp_source_path,tmp_return_path);
		system(cmd_string) ;

		int tryTime = 10;
		int ret = OS_FALSE;
		string runid, result,ce_info,tu,mu;

		//hdu 的status多1
		lang_id += 1;

		while (ret != OS_TRUE)
		{
			result = "";
			MSG_OUPUT_DBG("Get Status...");

			Sleep(6000);

			sprintf(cmd_string,"python -O hdu-vjudge.py status %d %d %s %s",
				GL_vpid, lang_id , hdu_username,tmp_return_path);

			system(cmd_string) ;

			ret =getHDUStatus(hdu_username, GL_vpid, lang_id, runid, result,ce_info,tu,mu);

			if (result.find("Queuing")!=string::npos
				|| result.find("Compiling")!=string::npos
				|| result.find("Running")!=string::npos)
			{
				ret = OS_FALSE;
			}

			if (result.find("Compilation Error")!=string::npos)
			{
				//获取编译错误信息
				sprintf(cmd_string,"python -O hdu-vjudge.py ce %s %s",runid.c_str(), tmp_return_path);
				system(cmd_string) ;

				string CE_Info = getCEinfo_brief(tmp_return_path);
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

		if (OS_FALSE == ret)
		{
			MSG_OUPUT_DBG("Get Status Error...");
			GL_verdictId = V_SE;
		}
		else
		{
			MSG_OUPUT_DBG("Get Status success...");
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
					write_log(JUDGE_ERROR,"DebugFile open error");
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

	return OS_TRUE;

}
#endif

int HDU_VJudge(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{

	Judge_Debug(DEBUG_TYPE_FUNC, "virtua-judge Local HDU.");

	return Judge_Via_CurlLib(pstJudgeSubmission);

	//return Judge_Via_python();
}



#endif

