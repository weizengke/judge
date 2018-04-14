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

#include "osp\common\include\osp_common_def.h"
#include "osp\command\include\icli.h"

using namespace std;

extern int cmd_init();
extern unsigned _stdcall  cmd_main_entry(void *pEntry);

enum CMD_CMO_TBLID_EM {
	CMD_CMO_TBL_SHOW = 0,
	CMD_CMO_TBL_CFG,		
};

enum CMD_CMO_SHOW_ID_EM {
	CMD_CMO_SHOW_CUR_CONFIG = CMD_ELEMID_DEF(MID_CMD, CMD_CMO_TBL_SHOW, 0),
	CMD_CMO_SHOW_SAVE_CONFIG,
	CMD_CMO_SHOW_THIS,
	CMD_CMO_SHOW_INC_DEFAULT,
	CMD_CMO_SHOW_HISTORY,
	CMD_CMO_SHOW_HISTORY_NUM,
	CMD_CMO_SHOW_VTY_ID,
};

enum CMD_CMO_CFG_ID_EM {
	CMD_CMO_CFG_SYSTEM = CMD_ELEMID_DEF(MID_CMD, CMD_CMO_TBL_CFG, 0),
	CMD_CMO_CFG_QUIT,
	CMO_CFG_CFG_DIAGVIEW,
	CMO_CFG_CFG_SAVE,
	CMO_CFG_CFG_RETURN,
};

VOID CMD_ShowCliState(ULONG vtyId, ULONG vtyId2)
{
	char buffTmp[65535] = {0};
	char *buff = buffTmp;
	struct cmd_vty *vty = NULL;
	string strDateStr;
	extern int API_TimeToString(string &strDateStr,const time_t &timeData);
	
	vty = cmd_vty_getById(vtyId2);
	if (NULL == vty)
	{
		return;
	}

	API_TimeToString(strDateStr, vty->user.lastAccessTime);
	
	buff += sprintf(buff, "Vty %u State:\r\n", vtyId2);
	buff += sprintf(buff, "  used          : %u\r\n", vty->used);
	buff += sprintf(buff, "  view_id       : %u\r\n", vty->view_id);
	buff += sprintf(buff, "  used_len      : %u\r\n", vty->used_len);
	buff += sprintf(buff, "  cur_pos       : %u\r\n", vty->cur_pos);
	buff += sprintf(buff, "  buffer        : %s\r\n", vty->buffer);
	buff += sprintf(buff, "  machine_prev  : %u\r\n", vty->inputMachine_prev);
	buff += sprintf(buff, "  machine_now   : %u\r\n", vty->inputMachine_now);
	buff += sprintf(buff, "  tabbingString : %s\r\n", vty->tabbingString);
	buff += sprintf(buff, "  tabString     : %s\r\n", vty->tabString);
	buff += sprintf(buff, "  tabStringLenth: %u\r\n", vty->tabStringLenth);
	buff += sprintf(buff, "  hpos          : %u\r\n", vty->hpos);
	buff += sprintf(buff, "  hindex        : %u\r\n", vty->hindex);
	buff += sprintf(buff, "  user.user_name: %s\r\n", vty->user.user_name);
	buff += sprintf(buff, "  user.state    : %u\r\n", vty->user.state);
	buff += sprintf(buff, "  user.type     : %u\r\n", vty->user.type);
	buff += sprintf(buff, "  lastAccessTime: %s\r\n", strDateStr.c_str());
	
	vty_printf(vtyId, buffTmp);

}

ULONG CMD_SHOW_Callback(VOID *pRcvMsg)
{
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	ULONG isThis = OS_NO;
	ULONG isCurCfg = OS_NO;
	ULONG isSaveCfg = OS_NO;
	ULONG incDefault = OS_NO;
	ULONG isHistory = OS_NO;
	ULONG ulNum = HISTORY_MAX_SIZE;
	ULONG isVtyState = OS_NO;
	ULONG ulInVtyId = 0;
	
	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case CMD_CMO_SHOW_THIS:
				isThis = OS_YES;
				break;
				
			case CMD_CMO_SHOW_CUR_CONFIG:
				isCurCfg = OS_YES;
				break;
				
			case CMD_CMO_SHOW_SAVE_CONFIG:	
				isSaveCfg = OS_YES;
				break;

			case CMD_CMO_SHOW_INC_DEFAULT:
				incDefault = OS_YES;
				break;

			case CMD_CMO_SHOW_HISTORY:
				isHistory = OS_YES;
				break;
			case CMD_CMO_SHOW_HISTORY_NUM:
				ulNum = cmd_get_ulong_param(pElem);
				break;
			case CMD_CMO_SHOW_VTY_ID:
				isVtyState = OS_YES;
				ulInVtyId = cmd_get_ulong_param(pElem);
				break;
				
			default:
				break;
		}
	}

	if (OS_YES == isThis)
	{
		extern void BDN_ShowCurrentViewBuildRun(ULONG vtyId, ULONG ulIncludeDefault);
		if (OS_YES == incDefault)
		{						
			BDN_ShowCurrentViewBuildRun(vtyId, OS_YES);
		}
		else
		{
			BDN_ShowCurrentViewBuildRun(vtyId, OS_NO);
		}
		
		return 0;
	}

	if (OS_YES == isCurCfg)
	{
		void BDN_ShowBuildRun(ULONG vtyId);
		BDN_ShowBuildRun(vtyId);
		return 0;
	}

	if (OS_YES == isSaveCfg)
	{
		extern void PDT_ShowCfg(ULONG vtyId);
		PDT_ShowCfg(vtyId);

		return 0;
	}	

	if (OS_YES == isHistory)
	{
		int try_idx = 0;
		int i = 0;
		struct cmd_vty * vty = NULL;
		
		extern struct cmd_vty *cmd_vty_getById(ULONG vtyId);
		vty = cmd_vty_getById(vtyId);
		if (NULL != vty)
		{
			for (i = 0;  i < HISTORY_MAX_SIZE; i++)
			{
				if (vty->history[i] == NULL)
					break;
			}
			
			for (i = i-1; i >= 0 && ulNum > 0; i--,ulNum--)
			{
				if (vty->history[i] == NULL)
					break;
			
				vty_printf(vtyId, "%s \r\n", vty->history[i]);
			}

		}

		return 0;
	}

	if (OS_YES == isVtyState)
	{
		CMD_ShowCliState(vtyId, ulInVtyId);
		return 0;
	}
	return 0;

}

ULONG CMD_Save_Config(ULONG vtyId)
{
	char filename[257] = {0};
	ULONG ulRet = OS_OK;
	CHAR *pBuildrun = NULL;
	FILE * fp= NULL;
	extern ULONG BDN_SystemBuildRun(CHAR **ppBuildrun);
	
	sprintf(filename, "conf\\config.cfg");

	fp = fopen(filename,"w+");
	if (NULL == fp)
	{
		vty_printf(vtyId, "Error: Open file %s failed.\r\n", filename);
		return OS_ERR;
	}
	
	ulRet = BDN_SystemBuildRun(&pBuildrun);
	if (OS_OK != ulRet)
	{
		vty_printf(vtyId, "Error: Save configuration failed.\r\n");
		fclose(fp);
		return OS_ERR;
	}
	
	fputs(pBuildrun, fp);
	
	free(pBuildrun);
	fclose(fp);

	vty_printf(vtyId, "Info: Save configuration successfully.\r\n");
	
	return OS_OK;
}


ULONG CMD_CFG_Callback(VOID *pRcvMsg)
{
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	
	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case CMD_CMO_CFG_SYSTEM:
				vty_view_set(vtyId, VIEW_SYSTEM);
				break;
				
			case CMO_CFG_CFG_DIAGVIEW:
				vty_view_set(vtyId, VIEW_DIAGNOSE);
				break;
				
			case CMD_CMO_CFG_QUIT:	
				vty_view_quit(vtyId);
				break;

			case CMO_CFG_CFG_SAVE:
				(VOID)CMD_Save_Config(vtyId);
				break;
				
			case CMO_CFG_CFG_RETURN:
				vty_view_set(vtyId, VIEW_USER);
				break;
				
			default:
				break;
		}
	}

	return 0;
}

ULONG CMD_RUN_Callback(VOID *pRcvMsg)
{
	ULONG iRet = 0;
	ULONG iTBLID = 0;
	
	iTBLID = cmd_get_first_elem_tblid(pRcvMsg);
		
	switch(iTBLID)
	{
		case CMD_CMO_TBL_SHOW:
			iRet = CMD_SHOW_Callback(pRcvMsg);
			
		case CMD_CMO_TBL_CFG:
			iRet = CMD_CFG_Callback(pRcvMsg);
			break;
	
		default:
			break;
	}

	return iRet;
}

ULONG CMD_ADP_RegShowCmd()
{
	CMD_VECTOR_S * vec = NULL;
	
	/* 命令行注册四部曲1: 申请命令行向量 */
	CMD_VECTOR_NEW(vec);

	/* 命令行注册四部曲2: 定义命令字 */
	cmd_regelement_new(CMD_ELEMID_NULL, 			CMD_ELEM_TYPE_KEY,    "display",     	   		 "Display", vec);
	cmd_regelement_new(CMD_CMO_SHOW_THIS, 			CMD_ELEM_TYPE_KEY,    "this",      	 			 "This", 	 vec);
	cmd_regelement_new(CMD_CMO_SHOW_CUR_CONFIG, 	CMD_ELEM_TYPE_KEY,    "current-configuration",   "Current configuration", 	 vec);
	cmd_regelement_new(CMD_CMO_SHOW_SAVE_CONFIG, 	CMD_ELEM_TYPE_KEY,    "save-configuration",      "Save configuration", 	 vec);
	cmd_regelement_new(CMD_CMO_SHOW_INC_DEFAULT, 	CMD_ELEM_TYPE_KEY,    "include-default",      	 "include-default", 	 vec);
	cmd_regelement_new(CMD_CMO_SHOW_HISTORY, 		CMD_ELEM_TYPE_KEY,    "history",      	 		 "Command excute in the history", 	 vec);
	cmd_regelement_new(CMD_CMO_SHOW_HISTORY_NUM, 	CMD_ELEM_TYPE_INTEGER,"INTEGER<1-200>",      	 "Number of history command show",  vec);
	cmd_regelement_new(CMD_ELEMID_NULL, 			CMD_ELEM_TYPE_KEY,    "vty",      	 		 	 "VTY", 	 vec);
	cmd_regelement_new(CMD_CMO_SHOW_VTY_ID, 		CMD_ELEM_TYPE_INTEGER,"INTEGER<0-32>",      	 "VTY ID",  vec);
	
	/* 命令行注册四部曲3: 注册命令行 */
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 2 ", vec);
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 2 5 ", vec);
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 3 ", vec);
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 4 ", vec);
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 6 ", vec);
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 6 7 ", vec);
	cmd_install_command(MID_CMD, VIEW_DIAGNOSE,  " 1 8 9 ", vec);
	
	/* 命令行注册四部曲4: 释放命令行向量 */
	CMD_VECTOR_FREE(vec);
	
}


ULONG CMD_ADP_RegConfigCmd()
{
	CMD_VECTOR_S * vec = NULL;
	
	/* 命令行注册四部曲1: 申请命令行向量 */
	CMD_VECTOR_NEW(vec);

	/* 命令行注册四部曲2: 定义命令字 */
	cmd_regelement_new(CMD_CMO_CFG_QUIT, 			CMD_ELEM_TYPE_KEY,    "quit",     	   "Quit from current view", vec);
	cmd_regelement_new(CMD_CMO_CFG_SYSTEM, 			CMD_ELEM_TYPE_KEY,    "system-view",   "Enter system-view", 	 vec);
	cmd_regelement_new(CMO_CFG_CFG_DIAGVIEW, 		CMD_ELEM_TYPE_KEY,    "diagnose-view", "Enter diagnose-view", 	 vec);
	cmd_regelement_new(CMO_CFG_CFG_SAVE, 			CMD_ELEM_TYPE_KEY,    "save",  		   "Save configuration", 	 vec);
	cmd_regelement_new(CMO_CFG_CFG_RETURN, 			CMD_ELEM_TYPE_KEY,    "return",  	   "Return to user view", 	 vec);
	
	/* 命令行注册四部曲3: 注册命令行 */
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 ", vec);
	cmd_install_command(MID_CMD, VIEW_USER,    " 2 ", vec);
	cmd_install_command(MID_CMD, VIEW_SYSTEM,  " 3 ", vec);
	cmd_install_command(MID_CMD, VIEW_SYSTEM,  " 4 ", vec);
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 5 ", vec);
	
	/* 命令行注册四部曲4: 释放命令行向量 */
	CMD_VECTOR_FREE(vec);
	
}

ULONG CMD_ADP_RegCmd()
{	
	(VOID)CMD_ADP_RegShowCmd();

	(VOID)CMD_ADP_RegConfigCmd();
	
	return 0;
}


ULONG CMD_ADP_Init()
{
	(VOID)CMD_ADP_RegCmd();
	
	/* 注册命令行处理回调 */
	(VOID)cmd_regcallback(MID_CMD, CMD_RUN_Callback);	
}

APP_INFO_S g_CMDAppInfo =
{
	MID_CMD,
	"Command",
	cmd_init,
	cmd_main_entry
};

void Cmd_RegAppInfo()
{
	RegistAppInfo(&g_CMDAppInfo);
}

