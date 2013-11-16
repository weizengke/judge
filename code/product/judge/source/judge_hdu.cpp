#if(JUDGE_VIRTUAL == VOS_YES)
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


#include "..\include\judge_inc.h"


using namespace std;

#define MAX_SIZE_BUF 10000000

#define DEBUG_PRINT(X)   X

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

int GL_vjudge;
int GL_vpid;



/* HDU VJUDGE */

char hdu_username[1000]="weizengke";
char hdu_password[1000]="********";

char tfilename[1000]="tmpfile.txt";

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

/* HDU Virtual Judge */
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

#if(VJUDGE_CURL==VOS_YES)

struct curl_slist *headerlist=NULL;

int loginEx(char *uname, char *pdw)
{
    FILE * fp=fopen(tfilename,"w+");
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
        curl_easy_setopt(curl, CURLOPT_URL, "http://acm.hdu.edu.cn/userloginex.php?action=login");
        string post=(string)"username="+uname+"&userpass="+pdw+"&login=Sign+In";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());

		res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
    }

	fclose(fp);

	MSG_StopDot();
	judge_outstring("done.\r\n");

    if (res) return BOOL_FALSE;

    string ts=getAllFromFile(tfilename);
    if (ts.find("No such user or wrong password.")!=string::npos)
	{
		judge_outstring("Error: No such user or wrong password.\r\n");
		return BOOL_FALSE;
	}

	judge_outstring("Info: Login hdu-judge success.\r\n");
    return BOOL_TRUE;
}

ULONG login()
{
    FILE * fp=fopen(tfilename,"w+");
	CURL *curl;
	CURLcode res;

    curl = curl_easy_init();

	MSG_OUPUT_DBG("Do login hduoj, it make takes a few seconds, please wait...[%s, %s]",hdu_username,hdu_password);

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
    if (tss.find("Connect(0) to MySQL Server failed.")!=string::npos||tss.find("<b>One or more following JUDGE_ERROR(s) occurred.")!=string::npos||tss.find("<h2>The requested URL could not be retrieved</h2>")!=string::npos||tss.find("PHP: Maximum execution time of")!=string::npos)
	{
		MSG_OUPUT_DBG("One or more JUDGE_ERROR(s) occurred.....");
		return BOOL_FALSE;
	}

	MSG_OUPUT_DBG("Submit success...");

    return BOOL_TRUE;
}
#endif

#if(VJUDGE_CURL==VOS_YES)
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

int getStatusEx(char *hdu_username)
{
	string pid = "1000";
	string lang = "C++";
	string runid;
	string result;
	string ce_info;
	string tu;
	string mu;
    ULONG ulRet = BOOL_TRUE;

    tu=mu="0";
    string ts;

	judge_outstring("Info: It will take a few seconds, please wait...");
	MSG_StartDot();

	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();

    if ( curl )
	{
		FILE *fp = fopen(tfilename, "ab+");
		curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "hdu.cookie");
		char url[255] = {0};
		sprintf(url, "http://acm.hdu.edu.cn/status.php?first=&pid=&user=%s&lang=&status=0", hdu_username);

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

	MSG_StopDot();
	judge_outstring("done.\r\n");
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

    return BOOL_TRUE;
}

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

	judge_outstring("Info: problem[%d] language[%d]  verdict[%s] submissionID[%s] time[%s ms] memory[%s kb].\r\n", pid, lang, result.c_str(), runid.c_str(), tu.c_str(), mu.c_str());

	if (BOOL_TRUE != ulRet)
	{
		MSG_OUPUT_DBG("get record failed.");
		return BOOL_FALSE;
	}

	MSG_OUPUT_DBG("Get status success...");

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

	return BOOL_TRUE;
}


#endif

