#include "product/judge/include/judge_inc.h"
#include <functional>
#include <algorithm>

using namespace std;

#define JUDGE_RECENT_MAX_SIZE_BUF 10000000

char g_judge_recent_tmpfile[MAX_PATH]="judge_recent_tmp.txt";
char g_judge_recent_buf[JUDGE_RECENT_MAX_SIZE_BUF];
char g_judge_recent_json_path[MAX_PATH] = "otheroj.json";
int g_judge_recent_enable = OS_NO;


#define MATCH_BUF_SIZE 2024
const int VECSIZE = 30;

struct MatchResult
{
    string name;
    vector<string> value;
};

class Pcre
{
public:
    Pcre();
    ~Pcre();

    //Add a regrex, pass in name and regrex
    int AddRule(const string &name, const string &patten);

    //clear all the regrex
    void ClearRules();

    //match all the regrex, also return all the string match to every regrex
    vector<MatchResult> MatchAllRule(const char content[]);

private:
    const char *error;
    int erroffset;
    int ovector[VECSIZE];
    vector<pcre*> re_arr;
    vector<string> patten_name;
};

Pcre::Pcre()
{
    re_arr.clear();
    patten_name.clear();
}

Pcre::~Pcre()
{
    for(int i=0; i<re_arr.size(); i++)
        {
                pcre_free(re_arr[i]);
        }
}

//Add a regrex patten and compile it.
int Pcre::AddRule(const string &name, const string &patten)
{
    pcre *re = pcre_compile(patten.c_str(), PCRE_MULTILINE|PCRE_NO_AUTO_CAPTURE, &error, &erroffset, NULL);
        if(re == NULL)
        {
                printf("pcre compile failed, offset %d: %s\n", erroffset, error);
        return -1;
        }
    else
    {
        re_arr.push_back(re);
        patten_name.push_back(name);
    }
}

//clear all the rule
void Pcre::ClearRules()
{
    for(int i=0; i<re_arr.size(); i++)
    {
        pcre_free(re_arr[i]);
    }
    re_arr.clear();
    patten_name.clear();
}

//match all regrex, if any match, return the matched patten name and it's values
vector<MatchResult> Pcre::MatchAllRule(const char content[])
{
    vector<MatchResult> result_arr;
    int length = strlen(content);
    char *buf = NULL;

	buf = (char*)malloc(MATCH_BUF_SIZE);
	if (NULL == buf)
	{
		return result_arr;
	}

    for(int i=0; i<re_arr.size(); i++)
    {
        MatchResult result;
        result.name = patten_name[i];
        int cur = 0;
        int rc;
        while(cur<length && (rc = pcre_exec(re_arr[i], NULL, content, length, cur, PCRE_NOTEMPTY, ovector, VECSIZE)) >= 0)
        {
            for(int j=0; j<rc; j++)
            {
	            memset(buf, 0, MATCH_BUF_SIZE);
				if (ovector[2*j+1]-ovector[2*j] > MATCH_BUF_SIZE)
				{
					continue;
				}
	            strncpy(buf, content+ovector[2*j], ovector[2*j+1]-ovector[2*j]);
                result.value.push_back(buf);
            }

            cur = ovector[1];
        }

        if(result.value.size() > 0)
            result_arr.push_back(result);

    }

	free(buf);

    return result_arr;
}

size_t judge_write_cb(void *buffer, size_t size, size_t nmemb, void *user_p)
{
	FILE *fp = (FILE *)user_p;
	size_t return_size = fwrite(buffer, size, nmemb, fp);
	return return_size;
}

string judge_get_context(char *filename)
{
    string res="";
    FILE * fp=fopen(filename,"r");

    while (fgets(g_judge_recent_buf,JUDGE_RECENT_MAX_SIZE_BUF,fp))
	{
		res+=g_judge_recent_buf;
    }

    fclose(fp);

    return res;
}

inline string& ltrim(string &str)
{
	string::iterator p = find_if(str.begin(), str.end(), not1(ptr_fun<int, int>(isspace)));
	str.erase(str.begin(), p);
	return str;
}

inline string& rtrim(string &str)
{
	string::reverse_iterator p = find_if(str.rbegin(), str.rend(), not1(ptr_fun<int , int>(isspace)));
	str.erase(p.base(), str.end());
	return str;
}

inline string& trim(string &str)
{
	ltrim(rtrim(str));
	return str;
}


typedef struct hdu_contest_info
{
	string oj_name;
    string title;
	string url;
    time_t start_t;
	bool   public_b;
}contest_t;

char *judge_to_utf8(char *pbuf)
{
#ifdef _WIN32_
	int sBufSize = strlen(pbuf);
	int dBufSize = MultiByteToWideChar(CP_ACP, 0, pbuf, sBufSize, NULL, 0) + 32;

	wchar_t * dBuf = new wchar_t[dBufSize];
	if (NULL == dBuf)
	{
		return NULL;
	}
	wmemset(dBuf, 0, dBufSize);

	int n =MultiByteToWideChar(CP_ACP, 0, pbuf, sBufSize, dBuf, dBufSize);
	if ( n > 0 )
	{
		dBufSize = WideCharToMultiByte(CP_UTF8, 0, dBuf, wcslen(dBuf), NULL, 0, NULL, 0) + 32;

		char *dBuf_ = (char*)malloc(dBufSize);
		if (NULL == dBuf_)
		{
			delete(dBuf);
			return NULL;
		}
		memset(dBuf_, 0, dBufSize);

		n = WideCharToMultiByte(CP_UTF8, 0, dBuf, wcslen(dBuf), dBuf_, dBufSize, NULL, 0);

		if ( n > 0 )
		{
			dBuf_[n] = '\0';

			delete(dBuf);

			return dBuf_;
		}

	}

	delete(dBuf);

	return NULL;
#else
	int sBufSize = strlen(pbuf);
	char *dBuf_ = (char*)malloc(sBufSize + 1);
	if (NULL == dBuf_)
	{
		return NULL;
	}
	memset(dBuf_, 0, sBufSize + 1);

	strcpy(dBuf_, pbuf);

	return dBuf_;
#endif
}

int judge_parse_contest_info_zoj(char *html, contest_t * contest)
{
	string string_ = html;

	if (NULL == html
		|| NULL == contest)
	{
		return 1;
	}

	//printf("\r\n ====%s====", html);
	{
		string s = string_;
		int pos =s.find("<td class=\"contestStatus\">Starts at");
		if (-1 == pos)
		{
			return 1;
		}		
	}
	
	{
		/* url */
		string s = string_;

		int pos =s.find("<td class=\"contestName\"><a href=\"");
		if (-1 == pos)
		{
			printf("\r\nError: Cound not found zoj url 1.");
			return 1;
		}
		pos += strlen("<td class=\"contestName\"><a href=\"");

		int pos2 =s.find("\"><font color=\"blue\">");
		if (-1 == pos2)
		{
			printf("\r\nError: Cound not found zoj url 2.");
			return 1;
		}

		string ts = s.substr(pos, pos2 - pos);

		contest->url = "http://acm.zju.edu.cn" + ts;

		Judge_Debug(DEBUG_TYPE_FUNC, "\r\n url=%s.", contest->url.c_str());

		string_ = s.substr(pos2);
	}
	
	{
		/* title */
		string s = string_;

		int pos =s.find("<font color=\"blue\">");
		if (-1 == pos)
		{
			printf("\r\nError: Cound not found zoj title 1.");
			return 1;
		}
		pos += strlen("<font color=\"blue\">");

		int pos2 =s.find("</font></a></td>");
		if (-1 == pos2)
		{
			printf("\r\nError: Cound not found zoj title 2.");
			return 1;
		}

		string s_ = s.substr(pos, pos2 - pos);
		contest->title = trim(s_);

		//printf("\r\n %s \r\n", contest->title.c_str());

		Judge_Debug(DEBUG_TYPE_FUNC, "\r\n title=%s.",contest->title.c_str());
		
		string_ = s.substr(pos2);
	}

	/* start */
	{
		string s = string_;

		//printf("\r\n ===%s==== \r\n", s.c_str());
		
		int pos =s.find("http://www.timeanddate.com/worldclock/fixedtime.html?");
		if (-1 == pos)
		{
			printf("\r\nError: Cound not found zoj contest start time.");
			return 1;
		}
		pos += strlen("http://www.timeanddate.com/worldclock/fixedtime.html?");

		int st = pos;
	    while (s[st]!='"') st++;

		string ts = s.substr(pos, st - pos);
		//printf("\r\n time1:%s", ts.c_str());

		int day, month, year, hour, min_, sec, p1;
		//month=1&day=19&year=2019&hour=12&min=00&sec=00&p1=33
		sscanf(ts.c_str(), "month=%d&day=%d&year=%d&hour=%d&min=%d&sec=%d&p1=%d",
				&month, &day, &year, &hour, &min_, &sec, &p1);

		char ttt[128] = {0};
		sprintf(ttt, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, min_, sec);
		
		Judge_Debug(DEBUG_TYPE_FUNC, "\r\n start=%s.", ttt);
		
		ts = ttt;

		time_t t;
		util_string_to_time(ts, t);

		contest->start_t = t;
		
	}

	contest->oj_name = "ZOJ";
	contest->public_b = true;

	return 0;
}


int judge_get_contest_zoj(vector<contest_t> &contest_v)
{
	FILE *fp = fopen(g_judge_recent_tmpfile, "wb+");
	CURL *curl;
	CURLcode res;

	Judge_Debug(DEBUG_TYPE_FUNC, "judge_get_contest_zoj start...");
	
    curl = curl_easy_init();
    if(curl)
    {
		curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );

        char url[255] = {0};
        sprintf(url, "http://acm.zju.edu.cn/onlinejudge/showContests.do");
        curl_easy_setopt(curl, CURLOPT_URL, url);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &judge_write_cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        res = curl_easy_perform(curl);

        Judge_Debug(DEBUG_TYPE_FUNC, "judge_get_contest_zoj, post request to zoj. (url=%s, res=%d)",
                        url, res);

        curl_easy_cleanup(curl);
    }

    fclose(fp);

	string ts = judge_get_context(g_judge_recent_tmpfile);

	Pcre test;
	test.AddRule("contest", "<tr class=\"rowOdd\">([\\s\\S]*?)<tr class=");
	test.AddRule("contest2", "<tr class=\"rowEven\">([\\s\\S]*?)<tr class=");

	vector<MatchResult> res1 = test.MatchAllRule(ts.c_str());

	int sum = 0;
	for(int i=0; i<res1.size(); i++)
	{
		Judge_Debug(DEBUG_TYPE_FUNC, "ZOJ find size=%u", res1[i].value.size());
	
		for(int j = 0; j<res1[i].value.size(); j++)
		{
			contest_t contest;
			contest.oj_name = "";
			contest.title = "";
			contest.url = "";
			contest.start_t = 0;
			contest.public_b = true;

			//printf("\r\n====\r\n %s \r\n====\r\n", (char*)res1[i].value[j].c_str());

			int ret = judge_parse_contest_info_zoj((char*)res1[i].value[j].c_str(), &contest);
			if (0 != ret)
			{
				continue;
			}

			string time_s = "";
			(VOID)util_time_to_string(time_s, contest.start_t);

			Judge_Debug(DEBUG_TYPE_FUNC, "ZOJ:\r\nOJ:%s\r\nTitle:%s\r\nurl=%s\r\ntime:%s\r\n%s\r\n",
					contest.oj_name.c_str(),
					contest.title.c_str(),
					contest.url.c_str(),
					time_s.c_str(),
					(true == contest.public_b)?"public":"private");

			contest_v.push_back(contest);
		}
	}

    return 0;
}

int judge_parse_contest_info_hdu(char *html, contest_t * contest)
{
	if (NULL == html
		|| NULL == contest)
	{
		return 1;
	}

    //char *phtml = judge_to_utf8(html);
	string string_ = html;

	/* ֮����pending��contest */
    int pos = string_.find("<td><font color=green>Pending</font></td>");
	if (-1 == pos)
	{
		//printf("\r\n not found pending..");
		return 1;
	}

	pos = string_.find("Private");
	if (-1 == pos)
	{
		contest->public_b = true;
	}
	else
	{
		contest->public_b = false;
	}

	Pcre test;
	test.AddRule("url", "<td height=22>([\\s\\S]*?)</td>");
	test.AddRule("title", "&nbsp;([\\s\\S]*?)</a></td>");
	test.AddRule("start", "<td>((?=.*\d)(?=.*[\\s].*)(?=.*-).{19})</td>");

	vector<MatchResult> res1 = test.MatchAllRule(html);

	//printf("\r\n res1.size=%d", res1.size());

	for(int i=0; i<res1.size(); i++)
	{
		//printf("\r\n %s, %d", res1[i].name.c_str(), res1[i].value.size());
		if (res1[i].value.size() == 0)
		{
			continue;
		}

		//printf("\r\n %s", res1[i].value[0].c_str());
		if (0 == strcmp(res1[i].name.c_str(), "url"))
		{
			/* url */
			string s = res1[i].value[0];

			int pos = s.find("</td>");
			if (-1 == pos)
			{
				printf("\r\nError: Cound not found hdu contest url.");
				return 1;
			}

			int st = pos;
			while (s[pos]!='>'){
				if (0 == pos){
					break;
				}
				pos--;
			}
			pos++;

			string cid = s.substr(pos, st-pos);

			contest->url = "http://acm.hdu.edu.cn/contests/contest_show.php?cid=" + cid;

			Judge_Debug(DEBUG_TYPE_FUNC, "\r\n url=%s.", contest->url.c_str());
		}

		if (0 == strcmp(res1[i].name.c_str(), "title"))
		{
			/* title */
			string s = res1[i].value[0];

			int pos =s.find("&nbsp;");
			if (-1 == pos)
			{
				printf("\r\nError: Cound not found hdu contest title.");
				return 1;
			}
			pos += strlen("&nbsp;");

			int pos2 =s.find("</a></td>");
			if (-1 == pos2)
			{
				printf("\r\nError: Cound not found hdu contest title.");
				return 1;
			}

			contest->title = judge_to_utf8((char*)s.substr(pos, pos2 - pos).c_str());

			Judge_Debug(DEBUG_TYPE_FUNC, "\r\n title=%s.",contest->title.c_str());
		}

		if (0 == strcmp(res1[i].name.c_str(), "start"))
		{
			string s = res1[i].value[0];

			int pos =s.find("<td>");
			if (-1 == pos)
			{
				printf("\r\nError: Cound not found hdu contest start time.");
				return 1;
			}
			pos += strlen("<td>");

			int pos2 =s.find("</td>");
			if (-1 == pos)
			{
				printf("\r\nError: Cound not found hdu contest start time.");
				return 1;
			}

			string ts = s.substr(pos, pos2 - pos);

			time_t t;
			util_string_to_time(ts, t);

			Judge_Debug(DEBUG_TYPE_FUNC, "\r\n start=%s.", ts.c_str());
			
			contest->start_t = t;
		}
	}

	contest->oj_name = "HDU";

	return 0;
}

int judge_get_contest_hdu(vector<contest_t> &contest_v)
{
	FILE *fp = fopen(g_judge_recent_tmpfile, "wb+");
	CURL *curl;
	CURLcode res;

	Judge_Debug(DEBUG_TYPE_FUNC, "judge_get_contest_hdu start...");
	
    curl = curl_easy_init();
    if(curl)
    {
		curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );

        char url[255] = {0};
        sprintf(url, "%s/contests/contest_list.php", hdu_domain);
        curl_easy_setopt(curl, CURLOPT_URL, url);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &judge_write_cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        res = curl_easy_perform(curl);

        Judge_Debug(DEBUG_TYPE_FUNC, "judge_get_hdu_contest, post request to HDU. (url=%s, res=%d)",
                        url, res);

        curl_easy_cleanup(curl);
    }

    fclose(fp);

	string ts = judge_get_context(g_judge_recent_tmpfile);

	Pcre test;
	test.AddRule("contest", "<tr align=center>([\\s\\S]*?)<tr bgcolor=#D7EBFF align=center>");
	test.AddRule("contest2", "<tr bgcolor=#D7EBFF align=center>([\\s\\S]*?)<tr align=center>");

	vector<MatchResult> res1 = test.MatchAllRule(ts.c_str());

	for(int i=0; i<res1.size(); i++)
	{
		Judge_Debug(DEBUG_TYPE_FUNC, "HDU find size=%u", res1[i].value.size());
	
		for(int j = 0; j<res1[i].value.size(); j++)
		{
			contest_t contest;
			contest.oj_name = "";
			contest.title = "";
			contest.url = "";
			contest.start_t = 0;
			contest.public_b = true;

			int ret = judge_parse_contest_info_hdu((char*)res1[i].value[j].c_str(), &contest);
			if (0 != ret)
			{
				continue;
			}

			string time_s = "";
			(VOID)util_time_to_string(time_s, contest.start_t);

			Judge_Debug(DEBUG_TYPE_FUNC, "HDU:\r\nOJ:%s\r\nTitle:%s\r\nurl=%s\r\ntime:%s\r\n%s\r\n",
					contest.oj_name.c_str(),
					contest.title.c_str(),
					contest.url.c_str(),
					time_s.c_str(),
					(true == contest.public_b)?"public":"private");

			contest_v.push_back(contest);
		}
	}

    return 0;
}

int judge_parse_contest_info_cf(char *html, contest_t * contest)
{
	string string_ = html;

	if (NULL == html
		|| NULL == contest)
	{
		return 1;
	}

	//printf("\r\n ====%s====", html);

	{
		/* title */
		string s = string_;

		int pos =s.find("<td>");
		if (-1 == pos)
		{
			printf("\r\nError: Cound not found cf contest title.");
			return 1;
		}
		pos += strlen("<td>");

		int pos2 =s.find("</td>");
		if (-1 == pos2)
		{
			printf("\r\nError: Cound not found cf contest title.");
			return 1;
		}

		string s_ = s.substr(pos, pos2 - pos);
		contest->title = trim(s_);

		Judge_Debug(DEBUG_TYPE_FUNC, "\r\n title=%s.",contest->title.c_str());
		
		string string_ = html + pos2 + strlen("</td>");
	}

	{
		string s = string_;

		int pos =s.find("https://www.timeanddate.com/worldclock/fixedtime.html?");
		if (-1 == pos)
		{
			printf("\r\nError: Cound notfound hdu contest start time.");
			return 1;
		}
		pos += strlen("https://www.timeanddate.com/worldclock/fixedtime.html?");

		int st = pos;
	    while (s[st]!='"') st++;

		string ts = s.substr(pos, st - pos);
		//printf("\r\n time1:%s", ts.c_str());

		int day, month, year, hour, min_, sec, p1;
		//day=24&month=10&year=2018&hour=19&min=35&sec=0&p1=166
		sscanf(ts.c_str(), "day=%d&month=%d&year=%d&hour=%d&min=%d&sec=%d&p1=%d",
				&day, &month, &year, &hour, &min_, &sec, &p1);

		char ttt[128] = {0};
		sprintf(ttt, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, min_, sec);

		//printf("\r\n time2:%s", ttt);

		ts = ttt;

		Judge_Debug(DEBUG_TYPE_FUNC, "\r\n start=%s.", ts.c_str());
		
		time_t t;
		util_string_to_time(ts, t);

		/* rusia UTC+3, ��ҪתΪUTC8*/
		t += 60*60*5;
		contest->start_t = t;
	}

	contest->oj_name = "Codeforces";
	contest->public_b = true;
	contest->url = "http://codeforces.com/contests";

	return 0;
}

int judge_get_contest_cf(vector<contest_t> &contest_v)
{
	FILE *fp = fopen(g_judge_recent_tmpfile, "wb+");
	CURL *curl;
	CURLcode res;

	Judge_Debug(DEBUG_TYPE_FUNC, "judge_get_contest_cf start...");
	
    curl = curl_easy_init();
    if(curl)
    {
		curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );

        char url[255] = {0};
        sprintf(url, "http://codeforces.com/contests");
        curl_easy_setopt(curl, CURLOPT_URL, url);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &judge_write_cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        res = curl_easy_perform(curl);

        Judge_Debug(DEBUG_TYPE_FUNC, "judge_get_contest_cf, post request to cf. (url=%s, res=%d)",
                        url, res);

        curl_easy_cleanup(curl);
    }

    fclose(fp);

	string ts_ = judge_get_context(g_judge_recent_tmpfile);

	/* �Ȼ�ȡδ��ʼ�ı��� */
	int pos =ts_.find("Current or upcoming contests");
	if (-1 == pos)
	{
		//printf("\r\nError: hihocoder Cound not found Current or upcoming contests.");
		return 1;
	}
	pos += strlen("Current or upcoming contests");

	int pos2 =ts_.find("Contest history");
	if (-1 == pos)
	{
		printf("\r\nError: Cound not found Contest history.");
		return 1;
	}

	string ts = ts_.substr(pos, pos2 - pos);

	pos =ts.find("data-contestId");
	if (-1 == pos)
	{
		printf("\r\nError: Cound not not found data-contestId.");
		return 0;
	}

	/* fallback to find <tr */
    while (ts[pos]!='<'){
		if (0 == pos){
			break;
		}
		pos--;
    }

	pos2 =ts.find("</table>");
	if (-1 == pos2)
	{
		printf("\r\nError: Cound not not found </table>.");
		return 0;
	}

	//printf("\r\n pos=%u, pos2=%u", pos, pos2);
	ts = ts.substr(pos, pos2 - pos);

	Judge_Debug(DEBUG_TYPE_FUNC, "string=============\r\n%s\r\n=============", ts.c_str());

	Pcre test;
	test.AddRule("contest", "<tr([\\s\\S]*?)</tr>");

	vector<MatchResult> res1 = test.MatchAllRule(ts.c_str());

	for(int i=0; i<res1.size(); i++)
	{
		Judge_Debug(DEBUG_TYPE_FUNC, "Coderforces find size=%u", res1[i].value.size());

		for(int j = 0; j<res1[i].value.size(); j++)
		{
			contest_t contest;
			contest.oj_name = "";
			contest.title = "";
			contest.url = "";
			contest.start_t = 0;
			contest.public_b = true;

			int ret = judge_parse_contest_info_cf((char*)res1[i].value[j].c_str(), &contest);
			if (0 != ret)
			{
				continue;
			}

			string time_s = "";
			(VOID)util_time_to_string(time_s, contest.start_t);
			
			Judge_Debug(DEBUG_TYPE_FUNC, "Coderforces:\r\nOJ:%s\r\nTitle:(%s)\r\nurl=%s\r\ntime:%s\r\n%s\r\n",
					contest.oj_name.c_str(),
					contest.title.c_str(),
					contest.url.c_str(),
					time_s.c_str(),
					(true == contest.public_b)?"public":"private");

			contest_v.push_back(contest);
		}
	}

    return 0;
}

/*
<tbody>
	<tr>
		<td><small><a href='http://www.timeanddate.com/worldclock/fixedtime.html?iso=20190209T2100&p1=248' target='blank'><time class='fixtime fixtime-short'>2019-02-09 21:00:00+0900</time></a></small></td>
		<td><small><a href='/contests/yahoo-procon2019-qual'>Yahoo Programming Contest 2019</a></small></td>
	</tr>
</tbody>
*/
int judge_parse_contest_info_atcoder(char *html, contest_t * contest)
{
	string string_ = html;

	if (NULL == html
		|| NULL == contest)
	{
		return 1;
	}

	{
		/* start */
		string s = string_;

		int pos =s.find("<time class=\'fixtime fixtime-short\'>");
		if (-1 == pos)
		{
			printf("\r\nError: Cound not found atcoder start time 1.");
			return 1;
		}
		pos += strlen("<time class=\'fixtime fixtime-short\'>");

		int pos2 =s.find("</time></a>");
		if (-1 == pos2)
		{
			printf("\r\nError: Cound not found atcoder start time 2.");
			return 1;
		}

		string ts = s.substr(pos, pos2 - pos);

		int day, month, year, hour, min_, sec, pp = 0;

		//2018/11/04 21:00
		sscanf(ts.c_str(), "%d-%d-%d %d:%d:%u+%d",
				&year, &month, &day, &hour, &min_, &sec, &pp);

		char ttt[128] = {0};
		sprintf(ttt, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, min_, sec);

		//printf("\r\n time:%s \r\n", ttt);

		ts = ttt;

		Judge_Debug(DEBUG_TYPE_FUNC, "\r\n start=%s.", ts.c_str());
		
		time_t t;
		util_string_to_time(ts, t);

		/* japan UTC+9, תΪUTC8*/
		t -= 60*60;
		contest->start_t = t;

		string_ = html + pos2 + strlen("</time></a>");
	}

	{
		/* url */
		string s = string_;

		int pos =s.find("<a href=\'");
		if (-1 == pos)
		{
			printf("\r\nError: Cound not found atcoder url.");
			return 1;
		}
		pos += strlen("<a href=\"");

		int pos2 =s.find("\'>");
		if (-1 == pos2)
		{
			printf("\r\nError: Cound not found atcoder url 2.");
			return 1;
		}

		string ts = s.substr(pos, pos2 - pos);

		contest->url = "https://atcoder.jp" + ts;

		Judge_Debug(DEBUG_TYPE_FUNC, "\r\n url=%s.", ts.c_str());
		
		string_ = s.substr(pos2 + 2);
	}


	{
		/* title */
		string s = string_;

		int pos =s.find("</a>");
		if (-1 == pos)
		{
			printf("\r\nError: Cound not found atcoder title 2. string_=%s.", string_.c_str());
			return 1;
		}

		string s_ = s.substr(0, pos);
		contest->title = trim(s_);

		Judge_Debug(DEBUG_TYPE_FUNC, "\r\n title=%s.",contest->title.c_str());
	}

	contest->oj_name = "AtCoder";
	contest->public_b = true;

	return 0;
}

int judge_get_contest_atcoder(vector<contest_t> &contest_v)
{
	FILE *fp = fopen(g_judge_recent_tmpfile, "wb+");
	CURL *curl;
	CURLcode res;

	Judge_Debug(DEBUG_TYPE_FUNC, "judge_get_contest_atcoder start...");
	
    curl = curl_easy_init();
    if(curl)
    {
		curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );

        char url[255] = {0};
        sprintf(url, "https://atcoder.jp");
        curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &judge_write_cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); 

        res = curl_easy_perform(curl);

        Judge_Debug(DEBUG_TYPE_FUNC, "judge_get_contest_atcoder, post request to atcoder. (url=%s, res=%d)",
                        url, res);

        curl_easy_cleanup(curl);
    }
	
    fclose(fp);

	string ts_ = judge_get_context(g_judge_recent_tmpfile);

	int pos =ts_.find("<h4>Upcoming Contests</h4>");
	if (-1 == pos)
	{
		Judge_Debug(DEBUG_TYPE_FUNC, "Error: hihocoder Cound not found Current or upcoming contests.");
		return 1;
	}
	pos += strlen("<h4>Upcoming Contests</h4>");

	int pos2 =ts_.find("<h4>Recent Contests</h4>");
	if (-1 == pos)
	{
		Judge_Debug(DEBUG_TYPE_FUNC, "Error: Cound not found Contest history.");
		return 1;
	}

	string ts = ts_.substr(pos, pos2 - pos);

	pos =ts.find("<tbody>");
	if (-1 == pos)
	{
		Judge_Debug(DEBUG_TYPE_FUNC, "Error: Cound not not found <tbody>.");
		return 0;
	}

	pos2 =ts.find("</tbody>");
	if (-1 == pos2)
	{
		Judge_Debug(DEBUG_TYPE_FUNC, "Error: Cound not not found </tbody>.");
		return 0;
	}

	//printf("\r\n pos=%u, pos2=%u", pos, pos2);
	ts = ts.substr(pos - 14, pos2 - (pos - 14));

	Pcre test;
	test.AddRule("contest", "<tr>([\\s\\S]*?)</tr>");

	vector<MatchResult> res1 = test.MatchAllRule(ts.c_str());

	for(int i=0; i<res1.size(); i++)
	{
		Judge_Debug(DEBUG_TYPE_FUNC, "Atcoder find size=%u", res1[i].value.size());

		for(int j = 0; j<res1[i].value.size(); j++)
		{
			contest_t contest;
			contest.oj_name = "";
			contest.title = "";
			contest.url = "";
			contest.start_t = 0;
			contest.public_b = true;

			int ret = judge_parse_contest_info_atcoder((char*)res1[i].value[j].c_str(), &contest);
			if (0 != ret)
			{
				continue;
			}

			string time_s = "";
			(VOID)util_time_to_string(time_s, contest.start_t);
			Judge_Debug(DEBUG_TYPE_FUNC, "Atcoder:\r\nOJ:%s\r\nTitle:(%s)\r\nurl=%s\r\ntime:%s\r\n%s\r\n",
					contest.oj_name.c_str(),
					contest.title.c_str(),
					contest.url.c_str(),
					time_s.c_str(),
					(true == contest.public_b)?"public":"private");

			contest_v.push_back(contest);
		}
	}

    return 0;
}

int judge_parse_contest_info_hihocoder(char *html, contest_t * contest)
{
	string string_ = html;

	if (NULL == html
		|| NULL == contest)
	{
		return 1;
	}

	//printf("\r\n ====%s====", html);

	{
		/* url */
		string s = string_;

		int pos =s.find("<a href=\"");
		if (-1 == pos)
		{
			printf("\r\nError: Cound not found hihocoder url 1.");
			return 1;
		}
		pos += strlen("<a href=\"");

		string_ = s.substr(pos);
		s = string_;

		int st = 0;
		while (s[st]!='"') st++;
			
		string ts = s.substr(0, st);

		contest->url = "https://hihocoder.com" + ts;

		Judge_Debug(DEBUG_TYPE_FUNC, "\r\n url=%s.", contest->url.c_str());
		
		string_ = s.substr(st + strlen("\">"));
	}
	
	{
		/* title */
		string s = string_;

		int pos =s.find("</a>");
		if (-1 == pos)
		{
			printf("\r\nError: Cound not found hihocoder title 1.");
			return 1;
		}

		string s_ = s.substr(0, pos);
		contest->title = trim(s_);

		Judge_Debug(DEBUG_TYPE_FUNC, "\r\n title=%s.",contest->title.c_str());

		string_ = s.substr(pos);
	}

	/* start */
	{
		string s = string_;

		//printf("\r\n ===%s==== \r\n", s.c_str());
		
		int pos =s.find("<span class=\"htzc\">");
		if (-1 == pos)
		{
			printf("\r\nError: Cound not found hihocoder start time.");
			return 1;
		}
		pos += strlen("<span class=\"htzc\">");

		int pos2 =s.find(" (");
		if (-1 == pos2)
		{
			printf("\r\nError: Cound not found hihocoder start time 2.");
			return 1;
		}

		string ts = s.substr(pos, pos2 - pos);

		int day, month, year, hour, min_;
		//2019-01-20 12:00
		sscanf(ts.c_str(), "%d-%d-%d %d:%d",&year, &month, &day, &hour, &min_);

		char ttt[128] = {0};
		sprintf(ttt, "%04d-%02d-%02d %02d:%02d:00", year, month, day, hour, min_);

		//printf("\r\n time2:%s,=%s=", ttt, ts.c_str());

		ts = ttt;

		Judge_Debug(DEBUG_TYPE_FUNC, "\r\n start=%s.", ts.c_str());
		
		time_t t;
		util_string_to_time(ts, t);

		contest->start_t = t;
	}

	contest->oj_name = "HihoCoder";
	contest->public_b = true;

	return 0;
}


int judge_get_contest_hihocoder(vector<contest_t> &contest_v)
{
	FILE *fp = fopen(g_judge_recent_tmpfile, "wb+");
	CURL *curl;
	CURLcode res;

	Judge_Debug(DEBUG_TYPE_FUNC, "judge_get_contest_hihocoder start...");
	
    curl = curl_easy_init();
    if(curl)
    {
		curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );

        char url[255] = {0};
        sprintf(url, "https://hihocoder.com/contests");
        curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &judge_write_cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); 
		

        res = curl_easy_perform(curl);

        Judge_Debug(DEBUG_TYPE_FUNC, "judge_get_contest_hihocoder, post request to hihocoder. (url=%s, res=%d)",
                        url, res);

        curl_easy_cleanup(curl);
    }

    fclose(fp);

	string ts = judge_get_context(g_judge_recent_tmpfile);

	int pos =ts.find("<ol class=\"upcoming\">");
	if (-1 == pos)
	{
		pos = ts.find("<ol class=\"ongoing\">");
		if (-1 == pos)
		{
			printf("\r\nError: hihocoder Cound not found ongoing or upcoming contests.");
			return 1;
		}

		//pos += strlen("<ol class=\"ongoing\">");
	}
	else
	{
		//pos += strlen("<ol class=\"upcoming\">");
	}

	Pcre test;
	test.AddRule("contest", "<li class=\"md-card md-summary clearfix\">([\\s\\S]*?)</li>");

	vector<MatchResult> res1 = test.MatchAllRule(ts.c_str());

	//printf("\r\n pos=%u, res1=%u, %s", pos, res1.size(), ts.c_str());
		
	int sum = 0;
	for(int i=0; i<res1.size(); i++)
	{
		Judge_Debug(DEBUG_TYPE_FUNC, "HihoCoder find size=%u", res1[i].value.size());
	
		for(int j = 0; j<res1[i].value.size(); j++)
		{
			contest_t contest;
			contest.oj_name = "";
			contest.title = "";
			contest.url = "";
			contest.start_t = 0;
			contest.public_b = true;

			//printf("\r\n====\r\n %s \r\n====\r\n", (char*)res1[i].value[j].c_str());

			int ret = judge_parse_contest_info_hihocoder((char*)res1[i].value[j].c_str(), &contest);
			if (0 != ret)
			{
				continue;
			}

			string time_s = "";
			(VOID)util_time_to_string(time_s, contest.start_t);

			Judge_Debug(DEBUG_TYPE_FUNC, "HihoCoder:\r\nOJ:%s\r\nTitle:%s\r\nurl=%s\r\ntime:%s\r\n%s\r\n",
					contest.oj_name.c_str(),
					contest.title.c_str(),
					contest.url.c_str(),
					time_s.c_str(),
					(true == contest.public_b)?"public":"private");


			contest_v.push_back(contest);
		}
	}

    return 0;
}

bool judge_comp(contest_t a, contest_t b)
{
    return a.start_t < b.start_t;
}

vector<contest_t> contest_v_zoj;
vector<contest_t> contest_v_hdu;
vector<contest_t> contest_v_cf;
vector<contest_t> contest_v_atcoder;
vector<contest_t> contest_v_hihocoder;

int g_judge_recent_running = 0;

int judge_recent_generate(void *p)
{
	int ret = 0;
	vector<contest_t> contest_v;
	vector<contest_t> contest_v_;

#if 0
	if (1 == g_judge_recent_running)
	{
		write_log(JUDGE_INFO, "Collect recent contests ,but now is running");
		//return 0;
	}

	g_judge_recent_running = 1;
#endif

	cJSON *array = cJSON_CreateArray();

	write_log(JUDGE_INFO, "Collect recent contests start. (json_path=%s)", g_judge_recent_json_path);

	ret = judge_get_contest_zoj(contest_v_);
	if (0 == ret)
	{
		contest_v_zoj.clear();
		contest_v_zoj.insert(contest_v_zoj.end(), contest_v_.begin(), contest_v_.end());
		write_log(JUDGE_INFO, "Collect zoj-contests ok. (size=%d)", contest_v_.size());
	}
	else
	{
		write_log(JUDGE_INFO, "Collect zoj-contests failed.");
	}
	contest_v.insert(contest_v.end(), contest_v_zoj.begin(), contest_v_zoj.end());

	contest_v_.clear();
	ret = judge_get_contest_hdu(contest_v_);
	if (0 == ret)
	{
		contest_v_hdu.clear();
		contest_v_hdu.insert(contest_v_hdu.end(), contest_v_.begin(), contest_v_.end());
		write_log(JUDGE_INFO, "Collect hdu-contests ok. (size=%d)", contest_v_.size());
	}
	else
	{
		write_log(JUDGE_INFO, "Collect hdu-contests failed.");
	}
	contest_v.insert(contest_v.end(), contest_v_hdu.begin(), contest_v_hdu.end());

	contest_v_.clear();
	ret = judge_get_contest_cf(contest_v_);
	if (0 == ret)
	{
		contest_v_cf.clear();
		contest_v_cf.insert(contest_v_cf.end(), contest_v_.begin(), contest_v_.end());
		write_log(JUDGE_INFO, "Collect codeforces-collect ok. (size=%d)", contest_v_.size());
	}
	else
	{
		write_log(JUDGE_INFO, "Collect codeforces-contests failed.");
	}
	contest_v.insert(contest_v.end(), contest_v_cf.begin(), contest_v_cf.end());

	contest_v_.clear();
	ret = judge_get_contest_atcoder(contest_v_);
	if (0 == ret)
	{
		contest_v_atcoder.clear();
		contest_v_atcoder.insert(contest_v_atcoder.end(), contest_v_.begin(), contest_v_.end());
		write_log(JUDGE_INFO, "Collect atcoder-contests ok. (size=%d)", contest_v_.size());
	}
	else
	{
		write_log(JUDGE_INFO, "Collect atcoder-contests failed.");
	}
	contest_v.insert(contest_v.end(), contest_v_atcoder.begin(), contest_v_atcoder.end());

	contest_v_.clear();
	ret = judge_get_contest_hihocoder(contest_v_);
	if (0 == ret)
	{
		contest_v_hihocoder.clear();
		contest_v_hihocoder.insert(contest_v_hihocoder.end(), contest_v_.begin(), contest_v_.end());
		write_log(JUDGE_INFO, "Collect hihocoder-contests ok. (size=%d)", contest_v_.size());
	}
	else
	{
		write_log(JUDGE_INFO, "Collect hihocoder-contests failed.");
	}
	contest_v.insert(contest_v.end(), contest_v_hihocoder.begin(), contest_v_hihocoder.end());


    sort(contest_v.begin(), contest_v.end(), judge_comp);

	for(int i=0; i<contest_v.size(); i++)
	{
		contest_t c = contest_v[i];

		string time_s = "";
		(VOID)util_time_to_string(time_s, c.start_t);

        char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        struct tm *t = gmtime(&c.start_t);
        //printf("%s,%s,%d", wday[t->tm_wday], asctime(t),t->tm_wday);

        #if 0
		printf("\r\nOJ:%s\r\nTitle:%s\r\nurl=%s\r\ntime:%s\r\n%s\r\n",
				c.oj_name.c_str(),
				c.title.c_str(),
				c.url.c_str(),
				time_s.c_str(),
				(true == c.public_b)?"public":"private");
		#endif

		cJSON *json = cJSON_CreateObject();
		if (NULL != json)
		{
			cJSON_AddItemToArray(array, json);

			cJSON_AddStringToObject(json, "oj", c.oj_name.c_str());
			cJSON_AddStringToObject(json, "link", c.url.c_str());
			cJSON_AddStringToObject(json, "name", c.title.c_str());
			cJSON_AddStringToObject(json, "start_time", time_s.c_str());
			cJSON_AddStringToObject(json, "week", wday[t->tm_wday]);
			cJSON_AddStringToObject(json, "access", (true == c.public_b)?"":"private");
		}
	}

	char *pjsonBuf = NULL;
	pjsonBuf = cJSON_Print(array);

	char *utf8_buf = pjsonBuf;
	if (NULL != utf8_buf)
	{
		//printf("%s", utf8_buf);
		char path_[MAX_PATH] = {0};
		sprintf(path_, "%s", g_judge_recent_json_path);

		FILE *fp = fopen(path_, "w+");
		if (NULL != fp)
		{
			fwrite(utf8_buf, sizeof(char), strlen(utf8_buf), fp);
			fclose(fp);
		}
		else
		{
			printf("\r\nError: open %s failed.", g_judge_recent_json_path);
		}

		//free(utf8_buf);
	}

	free(pjsonBuf);

	cJSON_Delete(array);

	write_log(JUDGE_INFO, "Collect recent contests ok. (json_path=%s)", g_judge_recent_json_path);

	//(void)util_remove(g_judge_recent_tmpfile);

	g_judge_recent_running = 0;

	return 0;
}

void judge_contests_show(int vtyId)
{
	vector<contest_t> contest_v;

	contest_v.insert(contest_v.end(), contest_v_zoj.begin(), contest_v_zoj.end());
	contest_v.insert(contest_v.end(), contest_v_hdu.begin(), contest_v_hdu.end());
	contest_v.insert(contest_v.end(), contest_v_cf.begin(), contest_v_cf.end());
	contest_v.insert(contest_v.end(), contest_v_atcoder.begin(), contest_v_atcoder.end());
	contest_v.insert(contest_v.end(), contest_v_hihocoder.begin(), contest_v_hihocoder.end());
	
    sort(contest_v.begin(), contest_v.end(), judge_comp);

	for(int i=0; i<contest_v.size(); i++)
	{
		contest_t c = contest_v[i];

		string time_s = "";
		(VOID)util_time_to_string(time_s, c.start_t);

        char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        struct tm *t = gmtime(&c.start_t);

		vty_printf(vtyId,
				"Contest %d:\r\n"
				" oj    : %s\r\n"
				" title : %s\r\n"
				" link  : %s\r\n"
				" time  : %s\r\n"
				" access: %s\r\n",
				i + 1,
				c.oj_name.c_str(),
				c.title.c_str(),
				c.url.c_str(),
				time_s.c_str(),
				(true == c.public_b)?"public":"private");
	}
}
