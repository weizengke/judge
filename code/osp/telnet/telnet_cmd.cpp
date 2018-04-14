#include <iostream>
#include <windows.h>
#include <process.h>
#include <stdlib.h>
#include <string>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <queue>
#include "osp\command\include\icli.h"
#include "osp\common\include\osp_common_def.h"
#include "product\include\pdt_common_inc.h"

using namespace std;

enum TELNET_CMO_TBLID_EM {
	TELNET_CMO_TBL_SHOW = 0,
	TELNET_CMO_TBL_CFG,		
};

enum TELNET_CMO_SHOW_ID_EM {
	TELNET_CMO_SHOW_USERS = CMD_ELEMID_DEF(MID_TELNET, TELNET_CMO_TBL_SHOW, 0),
};

enum TELNET_CMO_CFG_ID_EM {
	TELNET_CMO_CFG_TELNET_UNDO = CMD_ELEMID_DEF(MID_TELNET, TELNET_CMO_TBL_CFG, 0),
	TELNET_CMO_CFG_TELNET_ENABLE ,
	TELNET_CMO_CFG_AAA ,
	TELNET_CMO_CFG_AUTH_MODE,
	TELNET_CMO_CFG_AUTH_MODE_NONE,
	TELNET_CMO_CFG_AUTH_MODE_AAA,
	TELNET_CMO_CFG_AUTH_MODE_PASSWORD,
	TELNET_CMO_CFG_LOCAL_USER,
	TELNET_CMO_CFG_LOCAL_USER_NAME,
	TELNET_CMO_CFG_LOCAL_USER_PSW,
	TELNET_CMO_CFG_TELNET_USER_NAME,
	TELNET_CMO_CFG_TELNET_USER_PSW,	
	
};


ULONG TELNET_CFG_Enabble(ULONG vtyId)
{
	ULONG ulRet = OS_OK;

	extern ULONG TELNET_ServerEnable();
	ulRet = TELNET_ServerEnable();
	if (OS_OK != ulRet)
	{
		vty_printf(vtyId, "Error: Telnet server enable failed.\r\n");
	}

	return ulRet;
}

ULONG TELNET_CFG_Disabble(ULONG vtyId)
{
	ULONG ulRet = OS_OK;
	
	extern ULONG TELNET_ServerDisable();
	ulRet = TELNET_ServerDisable();
	if (OS_OK != ulRet)
	{
		vty_printf(vtyId, "Error: Telnet server disable failed.\r\n");
	}

	return ulRet;
}

VOID TELNET_SHOW_Users(ULONG vtyId)
{
	int ret = 0;
	time_t t = time(NULL);
	long diff = 0;
	long sec = 0;
	long min = 0;
	long hour = 0;
	char time_delay[10] = {0};
	char acbuff[65556] = {0};
	char *buff = NULL;
	struct cmd_vty * vty = NULL;
	extern long getdiftime(time_t maxt,time_t mint);
	
	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return;
	}
	
	buff = acbuff;
		
	buff += sprintf(buff, "   #   Type     Delay        Network Address   Socket  Username   \r\n"
						  " ---------------------------------------------------------------------\r\n");
	
	diff = getdiftime(t, g_con_vty->user.lastAccessTime);  
	sec=diff%60;
	diff=diff/60;
	min=diff%60; 
	hour=diff=diff/60;
	sprintf(time_delay, "%02d:%02d:%02d", hour, min, sec);	
	buff += sprintf(buff, " %s %-3u %-7s  %-12s %-16s  %-7s %s\r\n",
					(vty->user.type == 0)?"+":" ", CMD_VTY_CONSOLE_ID, "Console", time_delay, "127.0.0.1", "-","-");

	for (int i = 0; i < CMD_VTY_MAXUSER_NUM; i++)
	{
		if (g_vty[i].used)
		{
			sockaddr_in addr;  
		    int addrlen = sizeof(addr);  
		    if(getpeername(g_vty[i].user.socket, (struct sockaddr*)&addr, &addrlen) == -1){          
		        continue;  
		    }  

			if (0 == g_vty[i].user.state)
			{
				sprintf(time_delay, "Unauthorized");
			}
			else
			{
				diff = getdiftime(t, g_vty[i].user.lastAccessTime);
				sec=diff%60;
				diff=diff/60;
				min=diff%60; 
				hour=diff=diff/60;
				sprintf(time_delay, "%02d:%02d:%02d", hour, min, sec);
			}

			buff += sprintf(buff, " %s %-3u %-7s  %-12s %-16s  %-7u %s\r\n",
				(vty->user.socket == g_vty[i].user.socket)?"+":" ", i , "Telnet", time_delay, inet_ntoa(addr.sin_addr), g_vty[i].user.socket, g_vty[i].user.state?g_vty[i].user.user_name:"-");
		}
	}

	vty_printf(vtyId, acbuff);

}

ULONG TELNET_SHOW_Callback(VOID *pRcvMsg)
{
	ULONG ulRet = OS_OK;
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	ULONG isUsers = OS_NO;
	
	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case TELNET_CMO_SHOW_USERS:
				isUsers = OS_YES;
				break;
				
			default:
				break;
		}
	}
	
	if (OS_YES == isUsers)
	{
		(VOID)TELNET_SHOW_Users(vtyId);
	}
	
	return ulRet;

}


ULONG TELNET_CFG_Callback(VOID *pRcvMsg)
{
	ULONG ulRet = OS_OK;
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	ULONG isUndo = 0;
	ULONG isTelnetEnable = 0;
	ULONG isAuthMode = 0;
	ULONG isLocaluser = 0;
	ULONG isTelnetuser = 0;
	CHAR Username[32] = {0};
	CHAR Password[32] = {0};

	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case TELNET_CMO_CFG_TELNET_UNDO:
				isUndo = 1;
				break;
				
			case TELNET_CMO_CFG_TELNET_ENABLE:		
				isTelnetEnable = OS_YES;
				break;

			case TELNET_CMO_CFG_AAA:
				vty_view_set(vtyId, VIEW_AAA);
				break;

			case TELNET_CMO_CFG_AUTH_MODE_NONE:		
				extern ULONG g_TelnetAuthMode ;
				extern char g_szTelnetUsername[32];
				extern char g_szTelnetPassword[32];				
				g_TelnetAuthMode = 0;
				memset(g_szTelnetUsername, 0, sizeof(g_szTelnetUsername));
				memset(g_szTelnetPassword, 0, sizeof(g_szTelnetPassword));
				break;
				
			case TELNET_CMO_CFG_AUTH_MODE_PASSWORD:	
				extern ULONG g_TelnetAuthMode ;				
				g_TelnetAuthMode = 1;				
				vty_printf(vtyId, "Info: Please create telnet username and password.\r\n");				
				break;
				
			case TELNET_CMO_CFG_AUTH_MODE_AAA:		
				extern ULONG g_TelnetAuthMode ;
				extern char g_szTelnetUsername[32];
				extern char g_szTelnetPassword[32];				
				g_TelnetAuthMode = 2;
				memset(g_szTelnetUsername, 0, sizeof(g_szTelnetUsername));
				memset(g_szTelnetPassword, 0, sizeof(g_szTelnetPassword));				
				vty_printf(vtyId, "Info: Please create AAA username and password.\r\n");
				break;
			case TELNET_CMO_CFG_LOCAL_USER_NAME:
				isLocaluser = OS_YES;
				cmd_copy_string_param(pElem, Username);				
				break;

			case TELNET_CMO_CFG_LOCAL_USER_PSW:
				cmd_copy_string_param(pElem, Password);
				break;

			case TELNET_CMO_CFG_TELNET_USER_NAME:
				isTelnetuser = OS_YES;
				cmd_copy_string_param(pElem, Username);					
				break;
				
			case TELNET_CMO_CFG_TELNET_USER_PSW:
				cmd_copy_string_param(pElem, Password);
				break;

			default:
				break;
		}
	}

	if (OS_YES == isTelnetEnable)
	{
		if (0 == isUndo)
		{
			ulRet = TELNET_CFG_Enabble(vtyId);
		}
		else
		{
			ulRet = TELNET_CFG_Disabble(vtyId);
		}
		
		return 0;
	}

	if (OS_YES == isLocaluser)
	{
		if (0 == isUndo)
		{
			extern ULONG AAA_AddUser(ULONG vtyId, char *uname, char *psw);
			if (OS_OK != AAA_AddUser(vtyId, Username, Password))
			{
				return OS_ERR;
			}
		}
		else
		{
			extern ULONG AAA_DelUser(ULONG vtyId, char *uname);
			if (OS_OK != AAA_DelUser(vtyId, Username))
			{
				return OS_ERR;
			}
		}
		
		return 0;
	}

	if (OS_YES == isTelnetuser)
	{
		extern ULONG g_TelnetAuthMode ;
		extern char g_szTelnetUsername[];
		extern char g_szTelnetPassword[];
		
		if (g_TelnetAuthMode != 1)
		{
			vty_printf(vtyId, "Error: The telnet authentication-mode is not password, please change to mode password.\r\n");
			return OS_ERR;
		}
		
		strcpy(g_szTelnetUsername, Username);
		strcpy(g_szTelnetPassword, Password);

		return 0;
	}
	
	return ulRet;

}

ULONG TELNET_CMD_Callback(VOID *pRcvMsg)
{
	ULONG iRet = 0;
	ULONG iTBLID = 0;
	
	iTBLID = cmd_get_first_elem_tblid(pRcvMsg);
		
	switch(iTBLID)
	{
		case TELNET_CMO_TBL_SHOW:
			iRet = TELNET_SHOW_Callback(pRcvMsg);
			break;
			
		case TELNET_CMO_TBL_CFG:
			iRet = TELNET_CFG_Callback(pRcvMsg);
			break;

		default:
			break;
	}

	return iRet;
}

ULONG TELNET_RegCmdShow()
{
	CMD_VECTOR_S * vec = NULL;

	/* 命令行注册四部曲1: 申请命令行向量 */
	CMD_VECTOR_NEW(vec);

	/* 命令行注册四部曲2: 定义命令字 */
	// 1
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "display",	   "Display", vec);
	// 2
	cmd_regelement_new(TELNET_CMO_SHOW_USERS,			CMD_ELEM_TYPE_KEY,	  "users",		   "Users", vec);

	/* 命令行注册四部曲3: 注册命令行 */
	cmd_install_command(MID_TELNET, VIEW_GLOBAL,  " 1 2 ", vec);

	/* 命令行注册四部曲4: 释放命令行向量 */
	CMD_VECTOR_FREE(vec);

}

ULONG TELNET_RegCmdEnable()
{
	CMD_VECTOR_S * vec = NULL;

	/* 命令行注册四部曲1: 申请命令行向量 */
	CMD_VECTOR_NEW(vec);

	/* 命令行注册四部曲2: 定义命令字 */
	// 1
	cmd_regelement_new(TELNET_CMO_CFG_TELNET_UNDO,		CMD_ELEM_TYPE_KEY,	  "undo",		   "Undo Operation", vec);
	// 2
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "telnet",		   "Telnet protocol", vec);
	// 3
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "server",		   "Telnet Server",   vec);
	// 4
	cmd_regelement_new(TELNET_CMO_CFG_TELNET_ENABLE,	CMD_ELEM_TYPE_KEY,	  "enable",		   "Enable Telnet protocol", vec);
	// 5
	cmd_regelement_new(TELNET_CMO_CFG_AAA,				CMD_ELEM_TYPE_KEY,	  "aaa",		   "Authentication Authorization Accounting", vec);
	// 6
	cmd_regelement_new(TELNET_CMO_CFG_AUTH_MODE,		CMD_ELEM_TYPE_KEY,	  "authentication-mode","Telnet authentication mode", vec);
	// 7
	cmd_regelement_new(TELNET_CMO_CFG_AUTH_MODE_NONE,	CMD_ELEM_TYPE_KEY,	  "none",			"Authentication mode none", vec);
	// 8
	cmd_regelement_new(TELNET_CMO_CFG_AUTH_MODE_AAA,	CMD_ELEM_TYPE_KEY,	  "aaa",			"Authentication mode aaa", vec);
	// 9
	cmd_regelement_new(TELNET_CMO_CFG_AUTH_MODE_PASSWORD,CMD_ELEM_TYPE_KEY,	  "password",		"Authentication mode password", vec);
	// 10
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "local-user",		"AAA Local User", vec);
	// 11
	cmd_regelement_new(TELNET_CMO_CFG_LOCAL_USER_NAME,	CMD_ELEM_TYPE_STRING, "STRING<1-32>",	"AAA Local User name", vec);
	// 12
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "password",		"AAA Local User password", vec);
	// 13
	cmd_regelement_new(TELNET_CMO_CFG_LOCAL_USER_PSW,	CMD_ELEM_TYPE_STRING, "STRING<1-32>",	"AAA Local User password", vec);	
	// 14
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "username",		"Telnet authentication username", vec);
	// 15
	cmd_regelement_new(TELNET_CMO_CFG_TELNET_USER_NAME,	CMD_ELEM_TYPE_STRING, "STRING<1-32>",	"Telnet authentication username", vec);
	// 16
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "password",		"Telnet authentication password", vec);
	// 17
	cmd_regelement_new(TELNET_CMO_CFG_TELNET_USER_PSW,	CMD_ELEM_TYPE_STRING, "STRING<1-32>",	"Telnet authentication password", vec);
	
	/* 命令行注册四部曲3: 注册命令行 */
	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 1 2 3 4 ", vec);
	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 2 3 4 ", vec);
	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 5 ", vec);
	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 2 6 7 ", vec);
	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 2 6 8 ", vec);
	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 2 6 9 ", vec);
	cmd_install_command(MID_TELNET, VIEW_AAA,  	  " 10 11 12 13 ", vec);
	cmd_install_command(MID_TELNET, VIEW_AAA,     " 1 10 11 ", vec);
	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 2 14 15 16 17 ", vec);
	
	/* 命令行注册四部曲4: 释放命令行向量 */
	CMD_VECTOR_FREE(vec);

}

ULONG TELNET_RegCmd()
{
	(VOID)cmd_view_regist("aaa-view", "aaa", VIEW_AAA, VIEW_SYSTEM);
	
	(VOID)cmd_regcallback(MID_TELNET, TELNET_CMD_Callback);	

	(VOID)TELNET_RegCmdShow();
	
	(VOID)TELNET_RegCmdEnable();
	
	return OS_OK;
}


