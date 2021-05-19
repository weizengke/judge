
#include <iostream>
#include <ctime>
#include <queue>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#include <io.h>
#include <conio.h>
#endif

#ifdef _LINUX_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif

#include "kernel.h"

#include "osp_common_def.h"
#include "ic/include/debug_center_inc.h"
#include "util/util.h"
#include "root.h"
#include "icli.h"
#include "cli_history.h"

#if (OS_YES == OSP_MODULE_CLI)

using namespace std;

extern int cmd_init();
extern int cmd_main_entry(void *pEntry);

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
	CMD_VTY_S *vty = NULL;
	string strDateStr;

	vty = cmd_vty_getById(vtyId2);
	if (NULL == vty)
	{
		return;
	}


	(VOID)util_time_to_string(strDateStr, vty->user.lastAccessTime);

	buff += sprintf(buff, "Vty %u State:\r\n", vtyId2);
	buff += sprintf(buff, "  used          : %u\r\n", vty->used);
	buff += sprintf(buff, "  view_id       : %u\r\n", vty->view_id);
	buff += sprintf(buff, "  ulUsedLen     : %u\r\n", vty->ulUsedLen);
	buff += sprintf(buff, "  ulCurrentPos  : %u\r\n", vty->ulCurrentPos);
	buff += sprintf(buff, "  szBuffer      : %s\r\n", vty->szBuffer);
	buff += sprintf(buff, "  ucKeyTypePre  : %u\r\n", vty->ucKeyTypePre);
	buff += sprintf(buff, "  ucKeyTypeNow  : %u\r\n", vty->ucKeyTypeNow);
	buff += sprintf(buff, "  tabbingString : %s\r\n", vty->tabbingString);
	buff += sprintf(buff, "  tabString     : %s\r\n", vty->tabString);
	buff += sprintf(buff, "  ulhpos        : %u\r\n", vty->ulhpos);
	buff += sprintf(buff, "  ulhNum        : %u\r\n", vty->ulhNum);
	buff += sprintf(buff, "  user.user_name: %s\r\n", vty->user.user_name);
	buff += sprintf(buff, "  user.state    : %u\r\n", vty->user.state);
	buff += sprintf(buff, "  user.type     : %u\r\n", vty->user.type);
	buff += sprintf(buff, "  user.td       : %u\r\n", vty->user.terminal_debugging);
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
		extern VOID cli_bdn_show_by_current_view(ULONG vtyId, ULONG ulIncludeDefault);					
		cli_bdn_show_by_current_view(vtyId, incDefault);

		return 0;
	}

	if (OS_YES == isCurCfg)
	{
		extern VOID cli_bdn_show(ULONG vtyId, ULONG ulIncludeDefault);
		cli_bdn_show(vtyId, incDefault);
		return 0;
	}

	if (OS_YES == isSaveCfg)
	{
		extern void SYSMNG_ShowCfg(ULONG vtyId);
		SYSMNG_ShowCfg(vtyId);

		return 0;
	}	

	if (OS_YES == isHistory)
	{
		int try_idx = 0;
		int i = 0;
		CMD_VTY_S * vty = NULL;
		
		extern CMD_VTY_S *cmd_vty_getById(ULONG vtyId);
		vty = cmd_vty_getById(vtyId);
		if (NULL != vty) {
			cmd_vty_show_history(vty, ulNum);
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
	extern ULONG cli_bdn_system_buildrun(CHAR **ppBuildrun, ULONG ulIncludeDefault);

	sprintf(filename, "conf\/%s", g_startup_config);

	if( (file_access("conf", 0 )) == -1 )
	{
		if (false == create_directory("conf"))
		{
			vty_printf(vtyId, "Error: create directory conf failed.\r\n");
			return OS_ERR;
		}
	}
		
	fp = fopen(filename,"w+");
	if (NULL == fp)
	{
		vty_printf(vtyId, "Error: Open file %s failed.\r\n", filename);
		return OS_ERR;
	}
	
	ulRet = cli_bdn_system_buildrun(&pBuildrun, OS_NO);
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
	CHAR szHisCmd[32] = {0};
	CHAR szVtyId[32] = {0};
	
	sprintf(szHisCmd, "INTEGER<1-%u>", HISTORY_MAX_SIZE);
	sprintf(szVtyId, "INTEGER<0-%u>", CMD_VTY_MAXUSER_NUM);
	
	/* ??????????????1: ?????????????? */
	CMD_VECTOR_NEW(vec);

	/* ??????????????2: ?????????? */
	cmd_regelement_new(CMD_ELEMID_NULL, 			CMD_ELEM_TYPE_KEY,    "display",     	   		 "Display", vec);
	cmd_regelement_new(CMD_CMO_SHOW_THIS, 		CMD_ELEM_TYPE_KEY,    "this",      	 			 "This", 	 vec);
	cmd_regelement_new(CMD_CMO_SHOW_CUR_CONFIG, 	CMD_ELEM_TYPE_KEY,    "current-configuration",   "Current configuration", 	 vec);
	cmd_regelement_new(CMD_CMO_SHOW_SAVE_CONFIG, 	CMD_ELEM_TYPE_KEY,    "save-configuration",      "Save configuration", 	 vec);
	cmd_regelement_new(CMD_CMO_SHOW_INC_DEFAULT, 	CMD_ELEM_TYPE_KEY,    "include-default",      	 "include-default", 	 vec);
	cmd_regelement_new(CMD_CMO_SHOW_HISTORY, 		CMD_ELEM_TYPE_KEY,    "history",      	 		 "Command excute in the history", 	 vec);
	cmd_regelement_new(CMD_CMO_SHOW_HISTORY_NUM, 	CMD_ELEM_TYPE_INTEGER, szHisCmd,      	 		 "Number of history command show",  vec);
	cmd_regelement_new(CMD_ELEMID_NULL, 			CMD_ELEM_TYPE_KEY,     "vty",      	 		 	 "VTY", 	 vec);
	cmd_regelement_new(CMD_CMO_SHOW_VTY_ID, 		CMD_ELEM_TYPE_INTEGER, szVtyId,      	 		 "VTY ID",  vec);
	
	/* ??????????????3: ????????? */
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 2 ", vec);
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 2 5 ", vec);
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 3 ", vec);
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 3 5 ", vec);
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 4 ", vec);
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 6 ", vec);
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 6 7 ", vec);
	cmd_install_command(MID_CMD, VIEW_DIAGNOSE,  " 1 8 9 ", vec);
	
	/* ??????????????4: ????????????? */
	CMD_VECTOR_FREE(vec);
	
	return 0;
}


ULONG CMD_ADP_RegConfigCmd()
{
	CMD_VECTOR_S * vec = NULL;
	
	/* ??????????????1: ?????????????? */
	CMD_VECTOR_NEW(vec);

	/* ??????????????2: ?????????? */
	cmd_regelement_new(CMD_CMO_CFG_QUIT, 			CMD_ELEM_TYPE_KEY,    "quit",     	   "Quit from current view", vec);
	cmd_regelement_new(CMD_CMO_CFG_SYSTEM, 			CMD_ELEM_TYPE_KEY,    "system-view",   "Enter system-view", 	 vec);
	cmd_regelement_new(CMO_CFG_CFG_DIAGVIEW, 		CMD_ELEM_TYPE_KEY,    "diagnose-view", "Enter diagnose-view", 	 vec);
	cmd_regelement_new(CMO_CFG_CFG_SAVE, 			CMD_ELEM_TYPE_KEY,    "save",  		   "Save configuration", 	 vec);
	cmd_regelement_new(CMO_CFG_CFG_RETURN, 			CMD_ELEM_TYPE_KEY,    "return",  	   "Return to user view", 	 vec);
	
	/* ??????????????3: ????????? */
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 1 ", vec);
	cmd_install_command(MID_CMD, VIEW_USER,    " 2 ", vec);
	cmd_install_command(MID_CMD, VIEW_SYSTEM,  " 3 ", vec);
	cmd_install_command(MID_CMD, VIEW_SYSTEM,  " 4 ", vec);
	cmd_install_command(MID_CMD, VIEW_GLOBAL,  " 5 ", vec);
	
	/* ??????????????4: ????????????? */
	CMD_VECTOR_FREE(vec);
	
	return 0;
}

ULONG CMD_ADP_RegCmd()
{	
	(VOID)CMD_ADP_RegShowCmd();

	(VOID)CMD_ADP_RegConfigCmd();
	
	return 0;
}

int CMD_Hook_Reg()
{
#if 0
	extern int send(ULONG s, char *buf, int len, int flags);
	extern int recv(ULONG s, char *buf, int len, int flags);
	extern int closesocket(ULONG s);

	CMD_HOOK_RegCallback_socketsend(send);
	CMD_HOOK_RegCallback_Socketrecv(recv);
	CMD_HOOK_RegCallback_Socketclose(closesocket);
#endif

	return 0;
}

int CMD_ADP_Init()
{
	(VOID)CMD_Hook_Reg();
	
	(VOID)cmd_init();
	
	(VOID)CMD_ADP_RegCmd();

	(VOID)DEBUG_PUB_RegModuleDebugs(MID_CMD, "cli", "Command line interface");
	
	/* ????????��?????? */
	(VOID)cmd_regcallback(MID_CMD, CMD_RUN_Callback);	

	return 0;
}

APP_INFO_S g_CMDAppInfo =
{
	MID_CMD,
	"Command",
	CMD_ADP_Init,
	cmd_main_entry
};

void Cmd_RegAppInfo()
{
	APP_RegistInfo(&g_CMDAppInfo);
}

#endif
