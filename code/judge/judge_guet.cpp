/*

����GUETOJ virtual-judge
http://onlinejudge.guet.edu.cn/guetoj/index.html
Author: Jungle Wei
Create Date: 2013-12-28

*/

#include "judge/include/judge_inc.h"

#if (OS_YES == OSP_MODULE_JUDGE)

using namespace std;



char g_GUETtmps[VJUDGE_MAX_SIZE_BUF];

/* guet language list */
UCHAR g_aucGUETLanguageName[][VJUDGE_MAX_LANG_SIZE] = {
	"GCC",
	"G++",
	"C",
};

char guet_username[1000]="weizengke";
char guet_password[1000]="********";
char guet_judgerIP[20]="127.0.0.1";
int guet_sockport = 7706;
int guet_remote_enable=OS_NO;
int guet_vjudge_enable=OS_NO;

#if (OS_YES == OSP_MODULE_JUDGE_VJUDGE)
ULONG GUET_getLanguageNameByID(ULONG id, UCHAR *ucLanguageName)
{
	if (id < 0 || id >= sizeof(g_aucGUETLanguageName)/VJUDGE_MAX_LANG_SIZE)
	{
		return OS_FALSE;
	}

	strcpy((char *)ucLanguageName, (char *)g_aucGUETLanguageName[id]);
	return OS_TRUE;
}

ULONG GUET_getLanguageIDByName(UCHAR *ucLanguageName, ULONG *id)
{
	USHORT usLoop = 0;

	for (usLoop = 0; usLoop <= sizeof(g_aucGUETLanguageName)/VJUDGE_MAX_LANG_SIZE; ++usLoop)
	{
		if (strcmp((CHAR*)ucLanguageName, (CHAR*)g_aucGUETLanguageName[usLoop]) == 0)
		{
			*id = usLoop;
			return OS_TRUE;
		}
	}

	return OS_FALSE;
}


ULONG GUET_getResult(string s, string &status)
{
	/*
	<td>1487</td>
	<td class="link-column"><a href="/guetoj/problem/view/1000.html">1000</a></td>
	<td class="link-column"><a href="/guetoj/user/view/1000847.html">vjudge</a></td>
	<td>G++</td>
	<td>10</td>
	<td>800</td>
	<td>2013-12-28 01:58:42</td>
	<td>Accepted</td></tr>
	*/
	int pos = 0;
	int n_right = 0;
	int max_len = s.length();

	for (; n_right < 8; )
	{
		int pos=s.find("<td");
		if (-1 == pos)
		{
			return OS_FALSE;
			break;
		}

		n_right++;
		s.erase(0,pos+3);
	}

	pos = 0;
	while (s[pos]!='<') pos++;
	pos--;

	status = s.substr(1,pos);

	return OS_TRUE;
}

ULONG GUET_getRunid(string s, string &rid)
{


	/*
	<td class="link-column"><a href="/guetoj/run/view/1487.html">1487</td>
	<td class="link-column"><a href="/guetoj/problem/view/1000.html">1000</a></td>
	<td class="link-column"><a href="/guetoj/user/view/1000847.html">vjudge</a></td>
	<td>G++</td>
	<td>10</td>
	<td>800</td>
	<td>2013-12-28 01:58:42</td>
	<td>Accepted</td></tr>
	*/
	int pos = 0;
	int n_right = 0;
	int max_len = s.length();

	for (; n_right < 2; )
	{
		int pos=s.find(">");
		if (-1 == pos)
		{
			return OS_FALSE;
			break;
		}

		n_right++;
		s.erase(0,pos+1);

	}

	pos = 0;
	while (s[pos]!='<') pos++;
	//pos--;

	rid = s.substr(0,pos);

	return OS_TRUE;
}


string GUET_getCEinfo_brief(char *filename)
{
	string res="",ts;
    FILE * fp=fopen(filename,"r");

    while (fgets(g_GUETtmps,1000000,fp))
    {
        ts=g_GUETtmps;
        if (ts.find("<p>Compile Error</p>")!=string::npos)
        {
			Judge_Debug(DEBUG_TYPE_FUNC, "Found Compile Info.");
            while (fgets(g_GUETtmps,1000000,fp))
			{
                ts=g_GUETtmps;
				int pos = ts.find("<pre class=\"text\">");
                if (pos !=string::npos)
				{
					int len_ = strlen("<pre class=\"text\">");
					res = ts.erase(0, pos + len_);
					while (fgets(g_GUETtmps,1000000,fp))
					{
						ts=g_GUETtmps;
						if (ts.find("</div>")!=string::npos)
						{
							Judge_Debug(DEBUG_TYPE_FUNC, "Get Compile Info ok.");
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



ULONG GUET_getUsedTime(string s, string &timeuse)
{

	/*
	<td>1487</td>
	<td class="link-column"><a href="/guetoj/problem/view/1000.html">1000</a></td>
	<td class="link-column"><a href="/guetoj/user/view/1000847.html">vjudge</a></td>
	<td>G++</td>
	<td>10</td>
	<td>800</td>
	<td>2013-12-28 01:58:42</td>
	<td>Accepted</td></tr>
	*/
	int pos = 0;
	int n_right = 0;
	int max_len = s.length();

	for (; n_right < 5; )
	{
		int pos=s.find("<td");
		if (-1 == pos)
		{
			break;
		}

		n_right++;
		s.erase(0,pos+3);
	}

	pos = 0;
	while (s[pos]!='<') pos++;
	pos--;

	timeuse = s.substr(1,pos);

	return OS_TRUE;
}

ULONG GUET_getUsedMem(string s, string &memuse)
{

	/*
	<td>1487</td>
	<td class="link-column"><a href="/guetoj/problem/view/1000.html">1000</a></td>
	<td class="link-column"><a href="/guetoj/user/view/1000847.html">vjudge</a></td>
	<td>G++</td>
	<td>10</td>
	<td>800</td>
	<td>2013-12-28 01:58:42</td>
	<td>Accepted</td></tr>
	*/
	int pos = 0;
	int n_right = 0;
	int max_len = s.length();

	for (; n_right < 6; )
	{
		int pos=s.find("<td");
		if (-1 == pos)
		{
			break;
		}

		n_right++;
		s.erase(0,pos+3);
	}

	pos = 0;
	while (s[pos]!='<') pos++;
	pos--;

	memuse = s.substr(1,pos);

	return OS_TRUE;
}

string GUET_getLineFromFile(char *filename,int line)
{
    string res="";
    FILE * fp=fopen(filename,"r");
    int cnt=0;

	if (NULL == fp)
	{
		Judge_Debug(DEBUG_TYPE_ERROR, "open filename:%s error.",filename);
		return res;
	}

    while (fgets(g_GUETtmps,10000000,fp))
	{
        cnt++;
        res=g_GUETtmps;
        if (res.find("Submit Date</a></th>")!=-1)
		{
            fgets(g_GUETtmps,10000000,fp);
            fgets(g_GUETtmps,10000000,fp);
			fgets(g_GUETtmps,10000000,fp);
			fgets(g_GUETtmps,10000000,fp);
            res=g_GUETtmps;
            break;
        }
    }

    fclose(fp);

    return res;
}


/*
1	GCC
2	GCC
3	C
*/

int GUET_getGUETLangID(int GDOJlangID)
{
	int alang[25] = {1,2,3,2,1,1,1,1,1,1,1,1};

	return alang[GDOJlangID];
}


ULONG GUET_getStatus(string username, int lang, string &runid, string &result,string& ce_info,string &tu,string &mu)
{
	ULONG ulRet = OS_TRUE;
	tu=mu="0";
	string ts;

	MSG_OUPUT_DBG("Do get status...");

	ts = GUET_getLineFromFile(g_Vjudgetfilename,77);

	if(OS_FALSE == GUET_getUsedTime(ts, tu))
	{
		++ulRet;
		MSG_OUPUT_DBG("GUET_getUsedTime failed.");
	}

	if(OS_FALSE == GUET_getUsedMem(ts, mu))
	{
		++ulRet;
		MSG_OUPUT_DBG("GUET_getUsedMem failed.");
	}

	if(OS_FALSE == GUET_getRunid(ts, runid))
	{
		++ulRet;
		MSG_OUPUT_DBG("GUET_getRunid failed.");
	}

	if(OS_FALSE == GUET_getResult(ts, result))
	{
		++ulRet;
		MSG_OUPUT_DBG("GUET_getResult failed.");
	}

	//judge_outstring("Info: problem[%d] language[%d]  verdict[%s] submissionID[%s] time[%s ms] memory[%s kb].\r\n", pid, lang, result.c_str(), runid.c_str(), tu.c_str(), mu.c_str());

	if (OS_TRUE != ulRet)
	{
		MSG_OUPUT_DBG("get record failed.");
		return OS_FALSE;
	}

	MSG_OUPUT_DBG("Get status success...");

	return OS_TRUE;
}

int GUET_Login(char *uname, char *psw)
{
	char cmd_string[MAX_PATH];
	sprintf(cmd_string,"python -O guet-vjudge.py login %s %s",uname, psw);
	system(cmd_string) ;

	return 0;
}

int GUET_Judge_python(JUDGE_SUBMISSION_S *pstJudgeSubmission)
{
	char current_path[MAX_PATH] = {0};
	char tmp_source_path[MAX_PATH] = {0};
	char tmp_return_path[MAX_PATH] = {0};
	get_current_directory(sizeof(current_path),current_path);

	sprintf(tmp_source_path, "%s\\%s",current_path,pstJudgeSubmission->sourcePath);
	sprintf(tmp_return_path, "%s\\OJ_TMP\\guetjudge-%d.tmp",current_path,pstJudgeSubmission->solution.solutionId);

	strcpy(g_Vjudgetfilename,tmp_return_path);

	do
	{
		int lang_id = GUET_getGUETLangID(pstJudgeSubmission->solution.languageId);

		char cmd_string[MAX_PATH];
		sprintf(cmd_string,"python -O guet-vjudge.py submit %s %d %s %s %s %s",
				pstJudgeSubmission->problem.virtualPID, lang_id, guet_username, guet_password, tmp_source_path,tmp_return_path);
		system(cmd_string) ;

		int tryTime = 10;
		int ret = OS_FALSE;
		string runid, result,ce_info,tu,mu;

		//lang_id += 1;

		while (ret != OS_TRUE)
		{
			result = "";
			MSG_OUPUT_DBG("Get Status...");

			Sleep(6000);

			sprintf(cmd_string,"python -O guet-vjudge.py status %s %s",
				guet_username,tmp_return_path);

			system(cmd_string) ;

			ret =GUET_getStatus(guet_username, lang_id, runid, result,ce_info,tu,mu);

			if (result.find("Queuing")!=string::npos
				|| result.find("Compiling")!=string::npos
				|| result.find("Running")!=string::npos)
			{
				Judge_Debug(DEBUG_TYPE_FUNC, "Get Status, Queuing or Compiling or Running , try again...");
				ret = OS_FALSE;
			}

			if (result.find("Compile Error")!=string::npos)
			{
				//��ȡ���������Ϣ
				sprintf(cmd_string,"python -O guet-vjudge.py ce %s %s %s %s",guet_username, guet_password, runid.c_str(), tmp_return_path);
				system(cmd_string) ;

				string CE_Info = GUET_getCEinfo_brief(g_Vjudgetfilename);
				ce_info = CE_Info;
			}

			tryTime --;
			/* ѭ���ȴ�60s */
			if (0 == tryTime)
			{
				break;
			}
		}

		if (OS_FALSE == ret)
		{
			Judge_Debug(DEBUG_TYPE_ERROR, "Get Status Error...");
			pstJudgeSubmission->solution.verdictId = V_SE;
		}
		else
		{
			if (result.find("Accepted")!=string::npos)
			{
				pstJudgeSubmission->solution.verdictId = V_AC;
			}
			else if (result.find("Presentation Error")!=string::npos)
			{
				pstJudgeSubmission->solution.verdictId = V_PE;
			}
			else if (result.find("Runtime Error")!=string::npos)
			{
				pstJudgeSubmission->solution.verdictId = V_RE;
			}
			else if (result.find("Time Limit Exceeded")!=string::npos)
			{
				pstJudgeSubmission->solution.verdictId = V_TLE;
			}
			else if (result.find("Memory Limit Exceeded")!=string::npos)
			{
				pstJudgeSubmission->solution.verdictId = V_TLE;
			}
			else if (result.find("Output Limit Exceeded")!=string::npos)
			{
				pstJudgeSubmission->solution.verdictId = V_OLE;
			}
			else if (result.find("Wrong Answer")!=string::npos)
			{
				pstJudgeSubmission->solution.verdictId = V_WA;
			}
			else if (result.find("Compile Error")!=string::npos)
			{
				pstJudgeSubmission->solution.verdictId = V_CE;
				FILE *fp;
				char buffer[4096]={0};
				if ((fp = fopen (pstJudgeSubmission->DebugFile, "w")) == NULL){
					write_log(JUDGE_ERROR,"DebugFile open error");
					Judge_Debug(DEBUG_TYPE_ERROR, "DebugFile open error. File:%s.",pstJudgeSubmission->DebugFile);
					break;
				}
				fputs(ce_info.c_str(),fp);
				fclose(fp);
			}
			else
			{
				pstJudgeSubmission->solution.verdictId = V_SE;
			}
		}

		pstJudgeSubmission->solution.time_used= atoi(tu.c_str());
		pstJudgeSubmission->solution.memory_used= atoi(mu.c_str());

	} while (0);

	util_remove(tmp_return_path);

	return OS_TRUE;

}

int GUET_VJudge(JUDGE_SUBMISSION_S *pstJudgeSubmission)
{
	return GUET_Judge_python(pstJudgeSubmission);
}

#endif


#endif

