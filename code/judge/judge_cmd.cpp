

#include "judge/include/judge_inc.h"

#if (OS_YES == OSP_MODULE_JUDGE)

using namespace std;

VOID Judge_ShowBrief(ULONG vtyId)
{
	string strDateStr;
	extern time_t g_judge_last_judgetime;
	char buffTmp[65535] = {0};
	char *buff = buffTmp;
	extern int g_sock_port;
	extern char g_sysname[];	
	extern int SYSMNG_IsSocketActive();

	(VOID)util_time_to_string(strDateStr, g_judge_last_judgetime);

	buff += sprintf(buff, "# Local Judger Info\r\n");
	buff += sprintf(buff, "  Global Judge Is %s\r\n",
		  (g_judge_enable==OS_YES)?"Enable":"Disable");
	buff += sprintf(buff, "  Sysname       : %s\r\n", g_sysname);
	buff += sprintf(buff, "  Listening Port: %d (%s)\r\n",
		g_sock_port, (SYSMNG_IsSocketActive() == OS_OK)?"Active":"Inactive");
	buff += sprintf(buff, "  Judge Mode    : %s\r\n", (g_judge_mode==JUDGE_MODE_ACM)?"ACM":"OI");
	buff += sprintf(buff, "  Testcase Path : %s\r\n", g_judge_testcase_path);
	buff += sprintf(buff, "  Last Judge    : %s\r\n", strDateStr.c_str());
	buff += sprintf(buff, "  Queue Length  : %u\r\n", g_judge_queue.size());
	buff += sprintf(buff, "  AD Interval   : %u\r\n", g_judge_auto_detect_interval);
	buff += sprintf(buff, "  AD number     : %u\r\n", g_judge_auto_detect_num);
	buff += sprintf(buff, " =================================================="
		   "========================\r\n");

	buff += sprintf(buff, "# Virtual Judger Info\r\n");
	buff += sprintf(buff, "  Global Virtual Judge Is %s\r\n",
		  (g_vjudge_enable==OS_YES)?"Enable":"Disable");
	
	buff += sprintf(buff, "  HDU domain: %s\r\n", hdu_domain);

	buff += sprintf(buff, "  Judger | Account | Password | Status | Remote |"
		   "   Judger-IP   | J-Port\r\n");
	buff += sprintf(buff, "  -------------------------------------------------"
		   "------------------------\r\n");

	buff += sprintf(buff, " %-8s %-10s %-10s %-8s %-8s %-15s %-8d\r\n",
		  " HDU ", 
		   0 == strlen(hdu_username)?"-":hdu_username, 
		   0 == strlen(hdu_password)?"-":hdu_password,
		   (hdu_vjudge_enable==OS_YES)?"Enable":"Disable",
		   (hdu_remote_enable==OS_YES)?"Enable":"Disable",
		   hdu_judgerIP, hdu_sockport);

	buff += sprintf(buff, " %-8s %-10s %-10s %-8s %-8s %-15s %-8d\r\n",
		  " LeetCode", leetcode_username, leetcode_password,
		   (leetcode_vjudge_enable==OS_YES)?"Enable":"Disable",
		   (leetcode_remote_enable==OS_YES)?"Enable":"Disable",
		   leetcode_judgerIP, leetcode_sockport);

	buff += sprintf(buff, " =================================================="
		   "========================\r\n");

	buff += sprintf(buff, "# MySQL Info\r\n"
		  "  URL       : %s\r\n"
		  "  Username  : %s\r\n"
		  "  Password  : %s\r\n"
		  "  Table-Name: %s\r\n"
		  "  Port      : %d\r\n",
		Mysql_url,Mysql_username,Mysql_password,Mysql_table,Mysql_port);

	buff += sprintf(buff, " =================================================="
		   "========================\r\n");

	vty_printf(vtyId, buffTmp);
}

ULONG Judge_CFG_JudgeSolution(ULONG vtyId, ULONG solutionId)
{
	extern void judge_request_enqueue(int vtyId, int solutionId);
	judge_request_enqueue(vtyId, solutionId);

	return 0;
}

ULONG Judge_CFG_Show(VOID *pRcvMsg)
{
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	
	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	OS_DBGASSERT(iElemNum, "Judge_CFG_Show, element num is 0");

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case JUDGE_CMO_SHOW_JUDGE_BRIEF:	
				Judge_ShowBrief(vtyId);
				break;
			case JUDGE_CMO_SHOW_JUDGE_COLLECT_CONTESTS:
				extern void judge_contests_show(int vtyId);
				judge_contests_show(vtyId);
				break;
			default:
				break;
		}
	}
	
	return 0;

}

ULONG Judge_CFG_Config(VOID *pRcvMsg)
{
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	ULONG isJudge = 0;
	ULONG isVJudge = 0;
	ULONG isEnable = 0;
	ULONG isundo = 0;
	ULONG ulSolutionId = 0;
	ULONG isMode = 0;
	ULONG ulMode = 0;
	CHAR  szpath[256]={0};
	ULONG isSece = 0;
	ULONG isAutoDetect= 0;
	ULONG isHDUJudge= 0;
	ULONG isHduRemoteJudge= 0;
	ULONG isHDUJudgeUser= 0;
	ULONG isHDUJudgeIp= 0;
	ULONG isLeetcodeJudge= 0;
	ULONG isLeetcodeRemoteJudge= 0;
	ULONG isLeetcodeJudgeUser= 0;
	ULONG isLeetcodeJudgeIp= 0;	
	CHAR Username[32] = {0};
	CHAR Password[32] = {0};
	char ip[25] = {0};
	ULONG port = 0;
	ULONG isIgnoreExtraSpace = 0;
	ULONG isContestColect = 0;
	ULONG isProxy = 0;
	char proxy_url[128] = {0};
	char proxy_user_pwd[128] = {0};
	int contestId_start = 0;
	int contestId_end = 0;

	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	OS_DBGASSERT(iElemNum, "Judge_CFG_Config, cmd element num is 0");

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case JUDGE_CMO_CFG_UNDO:
				isundo = OS_YES;
				break;
				
			case JUDGE_CMO_CFG_JUDGE:
				isJudge  = OS_YES;
				break;
				
			case JUDGE_CMO_CFG_VJUDGE:
				isVJudge  = OS_YES;
				break;

			case JUDGE_CMO_CFG_ENABLE:
				isEnable = OS_YES;
				break;
				
			case JUDGE_CMO_CFG_JUDGE_SOLUTION_ID:	
				ulSolutionId = cmd_get_ulong_param(pElem);
				break;
			case JUDGE_CMO_CFG_JUDGE_MGR_VIEW:
				vty_view_set(vtyId, VIEW_JUDGE_MGR);
				break;
				
			case JUDGE_CMO_CFG_VJUDGE_MGR_VIEW:
				vty_view_set(vtyId, VIEW_VJUDGE_MGR);
				break;

			case JUDGE_CMO_CFG_MODE:
				isMode = OS_YES;
				break;

			case JUDGE_CMO_CFG_MODE_ACM:
				ulMode = JUDGE_MODE_ACM;
				break;

			case JUDGE_CMO_CFG_MODE_OI:
				ulMode = JUDGE_MODE_OI;
				break;

			case JUDGE_CMO_CFG_SECURITY:
				isSece = VOS_YES;
				break;

			case JUDGE_CMO_CFG_AUTODETECT:
				isAutoDetect = VOS_YES;
				break;

			case JUDGE_CMO_CFG_TESTCASE_PATH:
				cmd_copy_string_param(pElem, szpath);
				if( (file_access(szpath, 0 )) == -1 )
			    {
			    	vty_printf(vtyId, "Error: Path '%s' is not exist.\r\n", szpath);
			        return OS_ERR;
			    }

				strcpy(g_judge_testcase_path, szpath);
				break;
				
			case JUDGE_CMO_CFG_AUTODETECT_INTERVAL:	
				g_judge_auto_detect_interval = cmd_get_ulong_param(pElem);
				break;

			case JUDGE_CMO_CFG_AUTODETECT_NUM:	
				g_judge_auto_detect_num = cmd_get_ulong_param(pElem);
				break;

			case JUDGE_CMO_CFG_HDU_JUDGE:
				isHDUJudge = OS_YES;
				break;

			case JUDGE_CMO_CFG_HDU_REMOTE_JUDGE:
				isHduRemoteJudge = OS_YES;
				break;

			case JUDGE_CMO_CFG_HDU_USER_NAME:
				isHDUJudgeUser = OS_YES;
				cmd_copy_string_param(pElem, Username);					
				break;
				
			case JUDGE_CMO_CFG_HDU_USER_PSW:
				cmd_copy_string_param(pElem, Password);
				break;

			case JUDGE_CMO_CFG_HDU_REMOTE_IP:
				{
					ULONG ulIp = 0;
					extern VOID cmd_ip_ulong_to_string(ULONG ip, CHAR *buf);

					isHDUJudgeIp= OS_YES;
					ulIp = cmd_get_ip_ulong_param(pElem);
					cmd_ip_ulong_to_string(ulIp, ip);

				}
				break;

			case JUDGE_CMO_CFG_HDU_REMOTE_PORT:	
				port = cmd_get_ulong_param(pElem);
				break;


			case JUDGE_CMO_CFG_LEETCODE_JUDGE:
				isLeetcodeJudge = OS_YES;
				break;

			case JUDGE_CMO_CFG_LEETCODE_REMOTE_JUDGE:
				isLeetcodeRemoteJudge = OS_YES;
				break;

			case JUDGE_CMO_CFG_LEETCODE_USER_NAME:
				isLeetcodeJudgeUser = OS_YES;
				cmd_copy_string_param(pElem, Username);					
				break;
				
			case JUDGE_CMO_CFG_LEETCODE_USER_PSW:
				cmd_copy_string_param(pElem, Password);
				break;

			case JUDGE_CMO_CFG_LEETCODE_REMOTE_IP:
				{
					ULONG ulIp = 0;
					extern VOID cmd_ip_ulong_to_string(ULONG ip, CHAR *buf);
					isLeetcodeJudgeIp= OS_YES;
					ulIp = cmd_get_ip_ulong_param(pElem);
					cmd_ip_ulong_to_string(ulIp, ip);

				}
				break;

			case JUDGE_CMO_CFG_LEETCODE_REMOTE_PORT:	
				port = cmd_get_ulong_param(pElem);
				break;

			case JUDGE_CMO_CFG_EXTRA_SPACE:
				isIgnoreExtraSpace = OS_YES;
				break;

			case JUDGE_CMO_CFG_CONTEST_COLECT:
				isContestColect = OS_YES;
				break;

			case JUDGE_CMO_CFG_CONTEST_COLECT_PATH:
				extern char g_judge_recent_json_path[];
				cmd_copy_string_param(pElem, szpath);
				strcpy(g_judge_recent_json_path, szpath);
				break;
				
			case JUDGE_CMO_CFG_CONTEST_COLECT_INTERVAL:	
				g_judge_contest_colect_interval = cmd_get_ulong_param(pElem);
				break;

			case JUDGE_CMO_CFG_HTTP_PROXY:
				isProxy = OS_YES;
				break;

			case JUDGE_CMO_CFG_HTTP_PROXY_URL:
				cmd_copy_string_param(pElem, g_judge_proxy_url);
				break;

			case JUDGE_CMO_CFG_HTTP_PROXY_UNAME_PWD:
				cmd_copy_string_param(pElem, g_judge_proxy_uname_passwd);
				break;
			
			case JUDGE_CMO_CFG_CALC_RATING_CONTESTID:
				{
					contestId_start = cmd_get_ulong_param(pElem);
					contestId_end = contestId_start;
					break ;
				}
			case JUDGE_CMO_CFG_CALC_RATING_CONTESTID_END:
				{
					contestId_end = cmd_get_ulong_param(pElem);
					if (contestId_end < contestId_start) {
						vty_printf(vtyId, "Error: The end contest ID must be larger than start contest ID.\r\n");
						return OS_ERR;
					}
					break ;
				}				
			default:
				break;
		}
	}

	if (0 != ulSolutionId)
	{
		(VOID)Judge_CFG_JudgeSolution(vtyId, ulSolutionId);
		return 0;
	}
	
	if (contestId_start != 0) {
		extern void judge_elo_rating_caculate(int contestId);
		for (int i = contestId_start; i <= contestId_end; i++) {
			judge_elo_rating_caculate(i);
		}
		return 0;
	}

	if (OS_YES == isJudge
		&& OS_YES == isEnable)
	{
		g_judge_enable = (OS_YES == isundo)?OS_NO:OS_YES;
		return 0;
	}

	if (OS_YES == isVJudge
		&& OS_YES == isEnable)
	{
		g_vjudge_enable = (OS_YES == isundo)?OS_NO:OS_YES;
		return 0;
	}

	if (OS_YES == isMode)
	{
		g_judge_mode = ulMode;
		return 0;		
	}

	if (OS_YES == isSece
		&& OS_YES == isEnable)
	{
		g_judge_api_hook_enable = (OS_YES == isundo)?OS_NO:OS_YES;
		return 0;	
	}

	if (OS_YES == isAutoDetect
		&& OS_YES == isEnable)
	{
		g_judge_timer_enable = (OS_YES == isundo)?OS_NO:OS_YES;
		return 0;	
	}

	if (OS_YES == isHduRemoteJudge
		&& OS_YES == isEnable)
	{
		hdu_remote_enable = (OS_YES == isundo)?OS_NO:OS_YES;
		return 0;	
	}
	
	if (OS_YES == isHDUJudge
		&& OS_YES == isEnable)
	{
		hdu_vjudge_enable = (OS_YES == isundo)?OS_NO:OS_YES;
		return 0;	
	}

	if (OS_YES == isHDUJudgeUser)
	{
		extern char hdu_username[1000];
		extern char hdu_password[1000];
		strcpy(hdu_username, Username);
		strcpy(hdu_password, Password);
		return 0;
	}

	if (OS_YES == isHDUJudgeIp)
	{
		extern char hdu_judgerIP[20];
		extern int hdu_sockport;		
		strcpy(hdu_judgerIP, ip);
		hdu_sockport = port;		
		return 0;
	}


	if (OS_YES == isLeetcodeRemoteJudge
		&& OS_YES == isEnable)
	{
		leetcode_remote_enable = (OS_YES == isundo)?OS_NO:OS_YES;
		return 0;	
	}
	
	if (OS_YES == isLeetcodeJudge
		&& OS_YES == isEnable)
	{
		leetcode_vjudge_enable = (OS_YES == isundo)?OS_NO:OS_YES;
		return 0;	
	}

	if (OS_YES == isLeetcodeJudgeUser)
	{
		strcpy(leetcode_username, Username);
		strcpy(leetcode_password, Password);
		return 0;
	}

	if (OS_YES == isLeetcodeJudgeIp)
	{		
		strcpy(leetcode_judgerIP, ip);
		leetcode_sockport = port;		
		return 0;
	}

	if (OS_YES == isIgnoreExtraSpace
		&& OS_YES == isEnable)
	{
		extern int g_judge_ignore_extra_space_enable;
		g_judge_ignore_extra_space_enable = (OS_YES == isundo)?OS_NO:OS_YES;
		return 0;	
	}

	if (OS_YES == isContestColect
		&& OS_YES == isEnable)
	{
		extern int g_judge_recent_enable;
		g_judge_recent_enable = (OS_YES == isundo)?OS_NO:OS_YES;
		
		if (OS_YES != isundo)
		{
			extern int judge_recent_generate(void *p);
			extern int g_judge_recent_running;
			g_judge_recent_running = 0;
			thread_create(judge_recent_generate, NULL);
		}

		return 0;	
	}

	if (OS_YES == isProxy)
	{
		if (OS_YES == isundo)
		{
			memset(g_judge_proxy_url, 0, sizeof(g_judge_proxy_url));
			memset(g_judge_proxy_uname_passwd, 0, sizeof(g_judge_proxy_uname_passwd));
		}
	}	
	return 0;

}
ULONG Judge_CFG_ConfigLogFtp(VOID *pRcvMsg)
{
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	ULONG isUndo = OS_NO;
	char ip[32] = {0};

	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	OS_DBGASSERT(iElemNum, "Judge_CFG_ConfigLogFtp, cmd element num is 0");

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case JUDGE_CMO_LOG_FTP_UNDO:
				isUndo = OS_YES;
				break;
			case JUDGE_CMO_LOG_FTP_JUDGE_LOG:
				if (isUndo == OS_YES) {
					g_judge_upload_log_ftp_enable = OS_NO;
					g_judge_upload_log_ftp_port = 0;
					memset(g_judge_upload_log_ftp_ip, 0, sizeof(g_judge_upload_log_ftp_ip));
					memset(g_judge_upload_log_ftp_user, 0, sizeof(g_judge_upload_log_ftp_user));
					memset(g_judge_upload_log_ftp_pwd, 0, sizeof(g_judge_upload_log_ftp_pwd));
				} else {
					g_judge_upload_log_ftp_enable = OS_YES;
				}
				break;
			case JUDGE_CMO_LOG_FTP_JUDGE_FTP_IP:
				{
					ULONG ulIp = 0;
					extern VOID cmd_ip_ulong_to_string(ULONG ip, CHAR *buf);
					ulIp = cmd_get_ip_ulong_param(pElem);
					cmd_ip_ulong_to_string(ulIp, g_judge_upload_log_ftp_ip);
				}
				break;		
			case JUDGE_CMO_LOG_FTP_JUDGE_FTP_PORT:	
				g_judge_upload_log_ftp_port = cmd_get_ulong_param(pElem);
				break;	

			case JUDGE_CMO_LOG_FTP_JUDGE_FTP_UNAME:
				cmd_copy_string_param(pElem, g_judge_upload_log_ftp_user);					
				break;
				
			case JUDGE_CMO_LOG_FTP_JUDGE_FTP_PASSWD:
				cmd_copy_string_param(pElem, g_judge_upload_log_ftp_pwd);
				break;				
			default:
				break;
		}
	}

	return OS_OK;	
}

ULONG Judge_CfgCallback(VOID *pRcvMsg)
{
	ULONG iRet = 0;
	ULONG iTBLID = 0;
	
	iTBLID = cmd_get_first_elem_tblid(pRcvMsg);

	switch(iTBLID)
	{
		case JUDGE_TBL_SHOW:
			iRet = Judge_CFG_Show(pRcvMsg);
			break;
			
		case JUDGE_TBL_CFG:
			iRet = Judge_CFG_Config(pRcvMsg);
			break;

		case JUDGE_TBL_CFG_LOG_FTP:
			iRet = Judge_CFG_ConfigLogFtp(pRcvMsg);
			break;

		default:
			break;
	}

	return iRet;
}

ULONG Judge_RegCmdShow()
{
	CMD_VECTOR_S * vec = NULL;
		
	CMD_VECTOR_NEW(vec);

	cmd_regelement_new(CMD_ELEMID_NULL, 			CMD_ELEM_TYPE_KEY,    "display",  "Display", vec);
	cmd_regelement_new(CMD_ELEMID_NULL, 			CMD_ELEM_TYPE_KEY,    "judge", 	  "Judge", vec);
	cmd_regelement_new(JUDGE_CMO_SHOW_JUDGE_BRIEF,  CMD_ELEM_TYPE_KEY,  "brief",    "Judge brief Information", vec);
	cmd_regelement_new(JUDGE_CMO_SHOW_JUDGE_COLLECT_CONTESTS,  CMD_ELEM_TYPE_KEY,  "collect-contests",   "Collect-contests Information", vec);
	
	cmd_install_command(MID_JUDGE, VIEW_GLOBAL, " 1 2 3 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_GLOBAL, " 1 2 4 ", vec);
	
	CMD_VECTOR_FREE(vec);

	return 0;
}

ULONG Judge_RegCmdConfig()
{
	CMD_VECTOR_S * vec = NULL;
		
	CMD_VECTOR_NEW(vec);

	// 1 
	cmd_regelement_new(JUDGE_CMO_CFG_UNDO, CMD_ELEM_TYPE_KEY, "undo", "Undo Operation", vec);
	// 2
	cmd_regelement_new(JUDGE_CMO_CFG_JUDGE, CMD_ELEM_TYPE_KEY, "judge", "Judge", vec);
	// 3
	cmd_regelement_new(CMD_ELEMID_NULL, CMD_ELEM_TYPE_KEY, "solution", "The solution of submit", vec);
    // 4
	cmd_regelement_new(JUDGE_CMO_CFG_JUDGE_SOLUTION_ID, CMD_ELEM_TYPE_INTEGER, "INTEGER<1-4294967295>", "The solution ID",vec);
    // 5 
	cmd_regelement_new(JUDGE_CMO_CFG_JUDGE_MGR_VIEW, CMD_ELEM_TYPE_KEY, "judge-mgr", "Enter judge-mgr view", vec);
	// 6
	cmd_regelement_new(JUDGE_CMO_CFG_VJUDGE_MGR_VIEW, CMD_ELEM_TYPE_KEY, "virtual-judge-mgr", "Enter virtual-judge-mgr view", vec);
	// 7
	cmd_regelement_new(JUDGE_CMO_CFG_VJUDGE, CMD_ELEM_TYPE_KEY, "virtual-judge", "Virtual Judge", vec);
	// 8
	cmd_regelement_new(JUDGE_CMO_CFG_ENABLE, CMD_ELEM_TYPE_KEY, "enable", "Enable", vec);
	// 9
	cmd_regelement_new(JUDGE_CMO_CFG_MODE, CMD_ELEM_TYPE_KEY, "mode", "Mode of judger, default is ACM mode.", vec);
	// 10
	cmd_regelement_new(JUDGE_CMO_CFG_MODE_ACM, CMD_ELEM_TYPE_KEY, "acm", "ACM mode, the default mode.", vec);
	// 11
	cmd_regelement_new(JUDGE_CMO_CFG_MODE_OI, CMD_ELEM_TYPE_KEY, "oi", "OI mode", vec);
	// 12
	cmd_regelement_new(CMD_ELEMID_NULL, CMD_ELEM_TYPE_KEY, "testcase-path", "Testcase path", vec);
	// 13
	cmd_regelement_new(JUDGE_CMO_CFG_TESTCASE_PATH, CMD_ELEM_TYPE_STRING, "STRING<1-255>", "Testcase path, default is 'data'", vec);
	// 14
	cmd_regelement_new(JUDGE_CMO_CFG_SECURITY, CMD_ELEM_TYPE_KEY, "security", "Security function, only support API-Hook", vec);
	// 15
	cmd_regelement_new(JUDGE_CMO_CFG_AUTODETECT, CMD_ELEM_TYPE_KEY, "auto-detect", "Auto detect unfinish submition and then rejudge", vec);	
	// 16
	cmd_regelement_new(CMD_ELEMID_NULL, CMD_ELEM_TYPE_KEY, "interval", "Interval of auto-detect", vec);	
	// 17
	cmd_regelement_new(JUDGE_CMO_CFG_AUTODETECT_INTERVAL, CMD_ELEM_TYPE_INTEGER, "INTEGER<5-86400>", "Interval Time of auto-detect, default is 10 seconds",vec);
	// 18
	cmd_regelement_new(JUDGE_CMO_CFG_HDU_JUDGE, CMD_ELEM_TYPE_KEY, "hdu-judge", "The HDU virtual judge",vec);
	// 19
	cmd_regelement_new(JUDGE_CMO_CFG_HDU_REMOTE_JUDGE, CMD_ELEM_TYPE_KEY, "remote-judge", "Remote judge",vec);
	// 20
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "username",		"HDU username", vec);
	// 21
	cmd_regelement_new(JUDGE_CMO_CFG_HDU_USER_NAME,	CMD_ELEM_TYPE_STRING, "STRING<1-32>",	"HDU username", vec);
	// 22
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "password",		"HDU password", vec);
	// 23
	cmd_regelement_new(JUDGE_CMO_CFG_HDU_USER_PSW,	CMD_ELEM_TYPE_STRING, "STRING<1-32>",	"HDU password", vec);
	// 24
	cmd_regelement_new(CMD_ELEMID_NULL,				CMD_ELEM_TYPE_KEY, 	  "ip",	"IP address", vec);
	// 25
	cmd_regelement_new(JUDGE_CMO_CFG_HDU_REMOTE_IP,	CMD_ELEM_TYPE_IP, 	 CMD_ELMT_IP,	"IP address", vec);
	// 26
	cmd_regelement_new(CMD_ELEMID_NULL, 				CMD_ELEM_TYPE_KEY,	  "port", "Socket port of HDU remote judger", vec);
	// 27
	cmd_regelement_new(JUDGE_CMO_CFG_HDU_REMOTE_PORT, CMD_ELEM_TYPE_INTEGER, "INTEGER<1-65535>", "Socket port of HDU remote judger",vec);
	// 28
	cmd_regelement_new(CMD_ELEMID_NULL,				CMD_ELEM_TYPE_KEY, 	  "ignore",	"Ignore", vec);
	// 29
	cmd_regelement_new(JUDGE_CMO_CFG_EXTRA_SPACE,		CMD_ELEM_TYPE_KEY, 	  "extra-space",	"Extra space", vec);
	// 30
	cmd_regelement_new(JUDGE_CMO_CFG_CONTEST_COLECT,  CMD_ELEM_TYPE_KEY, 	  "contests-collect", "contests-collect", vec);
	// 31
	cmd_regelement_new(CMD_ELEMID_NULL,  				CMD_ELEM_TYPE_KEY, 	  "save-file", "contests-collect save file", vec);
	// 32
	cmd_regelement_new(JUDGE_CMO_CFG_CONTEST_COLECT_PATH,  CMD_ELEM_TYPE_STRING, "STRING<1-255>", "contests-collect save file, default is otheroj.json", vec);
	// 33
	cmd_regelement_new(JUDGE_CMO_CFG_CONTEST_COLECT_INTERVAL, CMD_ELEM_TYPE_INTEGER, "INTEGER<5-1440>", "Interval minute of auto-detect, default is 30 mins",vec);
	// 34
	cmd_regelement_new(CMD_ELEMID_NULL,  			 CMD_ELEM_TYPE_KEY, 	  "scan-number", "The number of submitions in each auto-detect scan period", vec);		
	// 35
	cmd_regelement_new(JUDGE_CMO_CFG_AUTODETECT_NUM, CMD_ELEM_TYPE_INTEGER, "INTEGER<1-128>", "Interval Time of auto-detect, default is 5",vec);
	// 36
	cmd_regelement_new(JUDGE_CMO_CFG_LEETCODE_JUDGE, CMD_ELEM_TYPE_KEY, "leetcode-judge", "The leetcode virtual judge",vec);
	// 37
	cmd_regelement_new(JUDGE_CMO_CFG_LEETCODE_REMOTE_JUDGE, CMD_ELEM_TYPE_KEY, "remote-judge", "Remote judge",vec);
	// 38
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "username",		"leetcode username", vec);
	// 39
	cmd_regelement_new(JUDGE_CMO_CFG_LEETCODE_USER_NAME,	CMD_ELEM_TYPE_STRING, "STRING<1-32>",	"leetcode username", vec);
	// 40
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "password",		"leetcode password", vec);
	// 41
	cmd_regelement_new(JUDGE_CMO_CFG_LEETCODE_USER_PSW,	CMD_ELEM_TYPE_STRING, "STRING<1-32>",	"leetcode password", vec);
	// 42
	cmd_regelement_new(CMD_ELEMID_NULL,				CMD_ELEM_TYPE_KEY, 	  "ip",	"IP address", vec);
	// 43
	cmd_regelement_new(JUDGE_CMO_CFG_LEETCODE_REMOTE_IP,	CMD_ELEM_TYPE_IP, 	 CMD_ELMT_IP,	"IP address", vec);
	// 44
	cmd_regelement_new(CMD_ELEMID_NULL, 				CMD_ELEM_TYPE_KEY,	  "port", "Socket port of leetcode remote judger", vec);
	// 45
	cmd_regelement_new(JUDGE_CMO_CFG_LEETCODE_REMOTE_PORT, CMD_ELEM_TYPE_INTEGER, "INTEGER<1-65535>", "Socket port of leetcode remote judger",vec);
	// 46
	cmd_regelement_new(CMD_ELEMID_NULL,  CMD_ELEM_TYPE_KEY, 	  "http", "Http", vec);
	// 47
	cmd_regelement_new(JUDGE_CMO_CFG_HTTP_PROXY,  CMD_ELEM_TYPE_KEY, 	  "local-proxy", "http local-proxy", vec);
	// 48
	cmd_regelement_new(JUDGE_CMO_CFG_HTTP_PROXY_URL,  CMD_ELEM_TYPE_STRING, 	"STRING<1-127>", "url, format:'127.0.0.1:8888'", vec);
	// 49
	cmd_regelement_new(JUDGE_CMO_CFG_HTTP_PROXY_UNAME_PWD,  CMD_ELEM_TYPE_STRING, 	"STRING<1-127>", "uname and password, format:'user:password'", vec);
	// 50
	cmd_regelement_new(CMD_ELEMID_NULL,  CMD_ELEM_TYPE_KEY, 	  "caculate-rating", "Caculate rating", vec);
	// 51 
	cmd_regelement_new(CMD_ELEMID_NULL,  CMD_ELEM_TYPE_KEY, 	  "contest", "Contest", vec);
	// 52
	cmd_regelement_new(JUDGE_CMO_CFG_CALC_RATING_CONTESTID, CMD_ELEM_TYPE_INTEGER, "INTEGER<1-65535>", "Contest ID",vec);
	// 53 
	cmd_regelement_new(CMD_ELEMID_NULL,  CMD_ELEM_TYPE_KEY, 	  "to", "To", vec);
	// 54
	cmd_regelement_new(JUDGE_CMO_CFG_CALC_RATING_CONTESTID_END, CMD_ELEM_TYPE_INTEGER, "INTEGER<1-65535>", "Contest ID End",vec);
	
	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 2 3 4 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 2 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 1 2 8 ", vec); 
	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 7 8 ", vec);	
	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 1 7 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_JUDGE_MGR, " 9 10 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_JUDGE_MGR, " 9 11 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_JUDGE_MGR, " 12 13 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_JUDGE_MGR, " 14 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_JUDGE_MGR, " 1 14 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_JUDGE_MGR, " 15 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_JUDGE_MGR, " 1 15 8 ", vec);	
	cmd_install_command(MID_JUDGE, VIEW_JUDGE_MGR, " 15 16 17 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 5 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 6 ", vec);	
	cmd_install_command(MID_JUDGE, VIEW_VJUDGE_MGR, " 18 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_VJUDGE_MGR, " 1 18 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_VJUDGE_MGR, " 18 19 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_VJUDGE_MGR, " 1 18 19 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_VJUDGE_MGR, " 18 20 21 22 23 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_VJUDGE_MGR, " 18 24 25 26 27 ", vec);
	
	cmd_install_command(MID_JUDGE, VIEW_JUDGE_MGR, " 28 29 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_JUDGE_MGR, " 1 28 29 8 ", vec);

	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 30 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 1 30 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 30 31 32 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 30 16 33 ", vec);

	cmd_install_command(MID_JUDGE, VIEW_JUDGE_MGR, " 15 34 35 ", vec);

	cmd_install_command(MID_JUDGE, VIEW_VJUDGE_MGR, " 36 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_VJUDGE_MGR, " 1 36 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_VJUDGE_MGR, " 36 37 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_VJUDGE_MGR, " 1 36 37 8 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_VJUDGE_MGR, " 36 38 39 40 41 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_VJUDGE_MGR, " 36 42 43 44 45 ", vec);

	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 46 47 48 49 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 1 46 47 ", vec);

	cmd_install_command(MID_JUDGE, VIEW_DIAGNOSE, " 2 50 51 52 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_DIAGNOSE, " 2 50 51 52 53 54 ", vec);

	CMD_VECTOR_FREE(vec);

	return 0;
}


ULONG Judge_RegCmdConfigLogUpload()
{
	CMD_VECTOR_S * vec = NULL;
		
	CMD_VECTOR_NEW(vec);

	// 1 
	cmd_regelement_new(JUDGE_CMO_LOG_FTP_UNDO, CMD_ELEM_TYPE_KEY, "undo", "Undo Operation", vec);
	// 2
	cmd_regelement_new(CMD_ELEMID_NULL, CMD_ELEM_TYPE_KEY, "judge", "Judge", vec);
	// 3
	cmd_regelement_new(JUDGE_CMO_LOG_FTP_JUDGE_LOG, CMD_ELEM_TYPE_KEY, "log", "log of judge result", vec);	
	// 4
	cmd_regelement_new(CMD_ELEMID_NULL, CMD_ELEM_TYPE_KEY, "upload", "Upload log of judge result", vec);	
	// 5
	cmd_regelement_new(CMD_ELEMID_NULL, CMD_ELEM_TYPE_KEY, "ftp", "FTP", vec);	
	// 6
	cmd_regelement_new(JUDGE_CMO_LOG_FTP_JUDGE_FTP_IP,	CMD_ELEM_TYPE_IP, 	 CMD_ELMT_IP,	"IP address", vec);
	// 7
	cmd_regelement_new(JUDGE_CMO_LOG_FTP_JUDGE_FTP_PORT, CMD_ELEM_TYPE_INTEGER, "INTEGER<1-65535>", "socket port",vec);
	// 8
	cmd_regelement_new(JUDGE_CMO_LOG_FTP_JUDGE_FTP_UNAME,	CMD_ELEM_TYPE_STRING, "STRING<1-32>",	"Username", vec);
	// 9
	cmd_regelement_new(JUDGE_CMO_LOG_FTP_JUDGE_FTP_PASSWD,	CMD_ELEM_TYPE_STRING, "STRING<1-32>",	"Password", vec);

	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 1 2 3 4 ", vec);
	cmd_install_command(MID_JUDGE, VIEW_SYSTEM, " 2 3 4 5 6 7 8 9 ", vec);

	CMD_VECTOR_FREE(vec);

	return 0;
}


ULONG judge_buildrun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{
	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);

	pBuildrun = *ppBuildrun;

	if (OS_YES == g_judge_enable)
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"judge enable");
		}
	}
	else
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"undo judge enable");
	}

#if (OS_YES == OSP_MODULE_JUDGE_VJUDGE)
		if (OS_YES == judge_is_vjudge_enable())
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"virtual-judge enable");
		}
		else
		{
			if (VOS_YES == ulIncludeDefault)
			{
				pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"undo virtual-judge enable");
			}
		}
#endif

	if (g_judge_upload_log_ftp_enable == OS_YES) {
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"judge log upload ftp %s %d %s %s",
							g_judge_upload_log_ftp_ip,
							g_judge_upload_log_ftp_port,
							g_judge_upload_log_ftp_user,
							g_judge_upload_log_ftp_pwd);
	} else {
		if (VOS_YES == ulIncludeDefault) {
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"undo judge log upload");
		}
	}

	if (0 == strlen(*ppBuildrun))
	{
		free(*ppBuildrun);
		*ppBuildrun = NULL;
	}

	return OS_OK;
}

ULONG judge_buildrun2(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{
	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);

	pBuildrun = *ppBuildrun;

	extern int g_judge_recent_enable;
	if (OS_YES != g_judge_recent_enable)
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"undo contests-collect enable");
		}
	}
	else
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"contests-collect enable");
	}

	if (30 == g_judge_contest_colect_interval)
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"contests-collect interval 30");
		}
	}
	else
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"contests-collect interval %u", g_judge_contest_colect_interval);
	}

	extern char g_judge_recent_json_path[];
	if (0 == strcmp(g_judge_recent_json_path, "otheroj.json"))
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"contests-collect save-file %s", g_judge_recent_json_path);
		}
	}
	else
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"contests-collect save-file %s", g_judge_recent_json_path);
	}

	if (0 == strlen(g_judge_proxy_url))
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"undo http local-proxy");
		}
	}
	else
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"http local-proxy %s %s", g_judge_proxy_url, g_judge_proxy_uname_passwd);
	}

	if (0 == strlen(*ppBuildrun))
	{
		free(*ppBuildrun);
		*ppBuildrun = NULL;
	}

	return OS_OK;
}

ULONG judge_buildrun_mgr(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{
	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);

	pBuildrun = *ppBuildrun;


	pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"judge-mgr");

	if (JUDGE_MODE_ACM == g_judge_mode)
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"mode acm");
		}
	}
	else if(JUDGE_MODE_OI == g_judge_mode)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"mode oi");
	}
	else
	{

	}

	if (OS_YES == g_judge_timer_enable)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"auto-detect enable");
	}
	else
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"undo auto-detect enable");
		}
	}

	if (JUDGE_AUTO_INTERVAL_DEFAULT == g_judge_auto_detect_interval)
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"auto-detect interval %u", g_judge_auto_detect_interval);
		}
	}
	else
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"auto-detect interval %u", g_judge_auto_detect_interval);
	}

	if (JUDGE_AUTO_NUM_DEFAULT == g_judge_auto_detect_num)
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"auto-detect scan-number %u", g_judge_auto_detect_num);
		}
	}
	else
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"auto-detect scan-number %u", g_judge_auto_detect_num);
	}		

	if (OS_YES == g_judge_api_hook_enable)
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"security enable");
		}
	}
	else
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"undo security enable");
	}

	if (OS_NO == g_judge_ignore_extra_space_enable)
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"undo ignore extra-space enable");
		}
	}
	else
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"ignore extra-space enable");

	}


	if (0 == strcmp(g_judge_testcase_path, "data"))
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"testcase-path %s", g_judge_testcase_path);
		}
	}
	else
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"testcase-path %s", g_judge_testcase_path);
	}

	return OS_OK;
}

#if (OS_YES == OSP_MODULE_JUDGE_VJUDGE)

ULONG judge_buildrun_vjudge(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{
	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);

	pBuildrun = *ppBuildrun;

	pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"virtual-judge-mgr");

	if (OS_YES == hdu_vjudge_enable)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"hdu-judge enable");
	}
	else
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"undo hdu-judge enable");
		}
	}

	if (0 != strlen(hdu_username) &&
		0 != strlen(hdu_password))
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"hdu-judge username %s password %s", hdu_username, hdu_password);
	}


	if (OS_YES == hdu_remote_enable)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"hdu-judge remote-judge enable");

		if (0 != strlen(hdu_judgerIP) &&
			0 != hdu_sockport)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"hdu-judge ip %s port %u", hdu_judgerIP, hdu_sockport);
		}
	}
	else
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"undo hdu-judge remote-judge enable");
		}
	}

	if (OS_YES == leetcode_vjudge_enable)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"leetcode-judge enable");
	}
	else
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"undo leetcode-judge enable");
		}
	}

	if (0 != strlen(leetcode_username) &&
		0 != strlen(leetcode_password))
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"leetcode-judge username %s password %s", leetcode_username, leetcode_password);
	}


	if (OS_YES == leetcode_remote_enable)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"leetcode-judge remote-judge enable");

		if (0 != strlen(leetcode_judgerIP) &&
			0 != leetcode_sockport)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"leetcode-judge ip %s port %u", leetcode_judgerIP, leetcode_sockport);
		}
	}
	else
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"undo leetcode-judge remote-judge enable");
		}
	}

	return OS_OK;
}
#endif

ULONG Judge_RegCmd()
{
	(void)cli_bdn_regist(MID_JUDGE, VIEW_SYSTEM, BDN_PRIORITY_NORMAL, judge_buildrun);
	(void)cli_bdn_regist(MID_JUDGE, VIEW_SYSTEM, BDN_PRIORITY_NORMAL - 1, judge_buildrun2);
	(void)cli_bdn_regist(MID_JUDGE, VIEW_JUDGE_MGR, BDN_PRIORITY_LOW, judge_buildrun_mgr);
	#if (OS_YES == OSP_MODULE_JUDGE_VJUDGE)
	(void)cli_bdn_regist(MID_JUDGE, VIEW_VJUDGE_MGR, BDN_PRIORITY_LOW - 1, judge_buildrun_vjudge);
	#endif

	(void)cmd_view_regist("judge-mgr-view", "judge-mgr", VIEW_JUDGE_MGR, VIEW_SYSTEM);
	(void)cmd_view_regist("virtual-judge-mgr-view", "virtual-judge-mgr", VIEW_VJUDGE_MGR, VIEW_SYSTEM);

	(VOID)cmd_regcallback(MID_JUDGE, Judge_CfgCallback);	
	
	(VOID)Judge_RegCmdShow();
	(VOID)Judge_RegCmdConfig();
	(VOID)Judge_RegCmdConfigLogUpload();

	(VOID)DEBUG_PUB_RegModuleDebugs(MID_JUDGE, "judge", "Judge kernel");
	
	return 0;
}

#endif
