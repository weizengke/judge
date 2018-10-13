

#include "product/judge/include/judge_inc.h"

#if (OS_YES == OSP_MODULE_JUDGE)

using namespace std;

VOID Judge_ShowBrief(ULONG vtyId)
{
	string strDateStr;
	extern time_t g_lastjudgetime;
	char buffTmp[65535] = {0};
	char *buff = buffTmp;
	extern int g_sock_port;
	extern char dataPath[];
	extern char g_sysname[];
	extern queue <JUDGE_DATA_S> g_JudgeQueue;
	
	(VOID)util_time_to_string(strDateStr, g_lastjudgetime);

	buff += sprintf(buff, "# Local Judger Info\r\n");
	buff += sprintf(buff, "  Global Judge Is %s\r\n",
		  (g_judge_enable==OS_YES)?"Enable":"Disable");
	buff += sprintf(buff, "  Sysname       : %s\r\n", g_sysname);
	buff += sprintf(buff, "  Sock Port     : %d\r\n", g_sock_port);
	buff += sprintf(buff, "  Judge Mode    : %s\r\n", (g_judge_mode==JUDGE_MODE_ACM)?"ACM":"OI");
	buff += sprintf(buff, "  Testcase Path : %s\r\n", dataPath);
	buff += sprintf(buff, "  Last Judge    : %s\r\n", strDateStr.c_str());
	buff += sprintf(buff, "  Queue Length  : %u\r\n", g_JudgeQueue.size());
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
#if 0
	buff += sprintf(buff, " %-8s %-10s %-10s %-8s %-8s %-15s %-8d\r\n",
		  " GUET3", guet_username, guet_password,
		   (guet_vjudge_enable==OS_YES)?"Enable":"Disable",
		   (guet_remote_enable==OS_YES)?"Enable":"Disable",
		   guet_judgerIP, guet_sockport);
#endif

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

ULONG Judge_CFG_JudgeSolution(ULONG solutionId)
{

	extern int Judge_PushQueue(int solutionId);
	Judge_PushQueue(solutionId);

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
	ULONG ulAdInterval = 0;
	ULONG isHDUJudge= 0;
	ULONG isRemoteJudge= 0;
	ULONG isHDUJudgeUser= 0;
	CHAR Username[32] = {0};
	CHAR Password[32] = {0};
	ULONG isHDUJudgeIp= 0;
	char ip[25] = {0};
	ULONG port = 0;
	ULONG isIgnoreExtraSpace= 0;

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
				extern char dataPath[];
				cmd_copy_string_param(pElem, szpath);
				if( (file_access(szpath, 0 )) == -1 )
			    {
			    	vty_printf(vtyId, "Error: Path '%s' is not exist.\r\n", szpath);
			        return OS_ERR;
			    }

				strcpy(dataPath, szpath);
				break;
				
			case JUDGE_CMO_CFG_AUTODETECT_INTERVAL:	
				extern int g_judge_auto_detect_interval;
				ulAdInterval = cmd_get_ulong_param(pElem);
				g_judge_auto_detect_interval = ulAdInterval;
				break;

			case JUDGE_CMO_CFG_HDU_JUDGE:
				isHDUJudge = OS_YES;
				break;

			case JUDGE_CMO_CFG_REMOTE_JUDGE:
				isRemoteJudge = OS_YES;
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

			case JUDGE_CMO_CFG_EXTRA_SPACE:
				isIgnoreExtraSpace = OS_YES;
				break;
				
			default:
				break;
		}
	}

	if (0 != ulSolutionId)
	{
		(VOID)Judge_CFG_JudgeSolution(ulSolutionId);
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
		extern ULONG g_ulNeedApiHookFlag;
		g_ulNeedApiHookFlag = (OS_YES == isundo)?OS_FALSE:OS_TRUE;
		return 0;	
	}

	if (OS_YES == isAutoDetect
		&& OS_YES == isEnable)
	{
		extern int g_judge_timer_enable;
		g_judge_timer_enable = (OS_YES == isundo)?OS_NO:OS_YES;
		return 0;	
	}


	if (OS_YES == isRemoteJudge
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

	if (OS_YES == isIgnoreExtraSpace
		&& OS_YES == isEnable)
	{
		extern int g_judge_ignore_extra_space_enable;
		g_judge_ignore_extra_space_enable = (OS_YES == isundo)?OS_NO:OS_YES;
		return 0;	
	}
	
	return 0;

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

		case JUDGE_TBL_HDU:
			break;

		default:
			break;
	}

	return iRet;
}

ULONG Judge_RegCmdShow()
{
	CMD_VECTOR_S * vec = NULL;
		
	/* 命令行注册四部曲1: 申请命令行向量 */
	CMD_VECTOR_NEW(vec);

	/* 命令行注册四部曲2: 定义命令字 */
	cmd_regelement_new(CMD_ELEMID_NULL, 			CMD_ELEM_TYPE_KEY,    "display",  "Display", vec);
	cmd_regelement_new(CMD_ELEMID_NULL, 			CMD_ELEM_TYPE_KEY,    "judge", 	  "Judge", vec);
	cmd_regelement_new(JUDGE_CMO_SHOW_JUDGE_BRIEF,  CMD_ELEM_TYPE_KEY,    "brief",    "Judge brief Information", vec);

	/* 命令行注册四部曲3: 注册命令行 */
	cmd_install_command(MID_JUDGE, VIEW_GLOBAL, " 1 2 3 ", vec);

	/* 命令行注册四部曲4: 释放命令行向量 */
	CMD_VECTOR_FREE(vec);

	return 0;
}

ULONG Judge_RegCmdConfig()
{
	CMD_VECTOR_S * vec = NULL;
		
	/* 命令行注册四部曲1: 申请命令行向量 */
	CMD_VECTOR_NEW(vec);

	/* 命令行注册四部曲2: 定义命令字 */
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
	cmd_regelement_new(JUDGE_CMO_CFG_AUTODETECT_INTERVAL, CMD_ELEM_TYPE_INTEGER, "INTEGER<1-65535>", "Interval Time of auto-detect, default is 10 mins",vec);
	// 18
	cmd_regelement_new(JUDGE_CMO_CFG_HDU_JUDGE, CMD_ELEM_TYPE_KEY, "hdu-judge", "The HDU virtual judge",vec);
	// 19
	cmd_regelement_new(JUDGE_CMO_CFG_REMOTE_JUDGE, CMD_ELEM_TYPE_KEY, "remote-judge", "Remote judge",vec);
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

	/* 命令行注册四部曲3: 注册命令行 */
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
	/* 命令行注册四部曲4: 释放命令行向量 */
	CMD_VECTOR_FREE(vec);

	return 0;
}

ULONG Judge_RegCmd()
{
	/*注册视图 */
	(void)cmd_view_regist("judge-mgr-view", "judge-mgr", VIEW_JUDGE_MGR, VIEW_SYSTEM);
	(void)cmd_view_regist("virtual-judge-mgr-view", "virtual-judge-mgr", VIEW_VJUDGE_MGR, VIEW_SYSTEM);

	(VOID)cmd_regcallback(MID_JUDGE, Judge_CfgCallback);	
	
	(VOID)Judge_RegCmdShow();

	(VOID)Judge_RegCmdConfig();
}

#endif
