#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#ifdef _LINUX_
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#endif

#ifdef WIN32
#include <io.h>
#include <winsock2.h>

#endif

#include "kernel.h"

#include "osp_common_def.h"
#include "pdt_common_inc.h"
#include "../include/icli.h"
#include "ic/include/debug_center_inc.h"
#include "util/util.h"

#if (OS_YES == OSP_MODULE_TELNETS)

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
	TELNET_CMO_CFG_TELNET_SERVER_PORT,
	TELNET_CMO_CFG_TELNET_SERVER_PORT_NUM,
	TELNET_CMO_CFG_AUTH_MODE,
	TELNET_CMO_CFG_AUTH_MODE_NONE,
	TELNET_CMO_CFG_AUTH_MODE_AAA,
	TELNET_CMO_CFG_AUTH_MODE_PASSWORD,
	TELNET_CMO_CFG_TELNET_USER_NAME,
	TELNET_CMO_CFG_TELNET_USER_PSW,	
	TELNET_CMO_CFG_TELNET_CLIENT_IP,
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

ULONG TELNET_CFG_SetPort(ULONG port)
{
	ULONG ulRet = OS_OK;
	extern ULONG g_TelnetServerEnable;
	extern int g_telnet_port;
	extern ULONG TELNET_ServerDisable();
	extern ULONG TELNET_ServerEnable();
	ULONG portOld = g_telnet_port;

	if (port == g_telnet_port) {
		return OS_OK;
	}
	
	TELNET_ServerDisable();
	g_telnet_port = port;

	if (OS_YES != g_TelnetServerEnable) {
		return OS_OK;
	}

	if (OS_OK != TELNET_ServerEnable()) {
		g_telnet_port = portOld;
		TELNET_ServerEnable();
		return OS_ERR;
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
	char time_delay[128] = {0};
	char acbuff[65556] = {0};
	char *buff = NULL;
	CMD_VTY_S * vty = NULL;

	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return;
	}
	
	buff = acbuff;
		
	buff += sprintf(buff, "   #   Type     Delay        Network Address   Socket  Username   \r\n"
						  " ---------------------------------------------------------------------\r\n");
	
	CMD_VTY_S * vty_con = cmd_vty_getById(CMD_VTY_CONSOLE_ID);
	if (vty_con) {
		diff = util_getdiftime(t, vty_con->user.lastAccessTime);  
		sec=diff%60;
		diff=diff/60;
		min=diff%60; 
		hour=diff=diff/60;
		sprintf(time_delay, "%02d:%02d:%02d", hour, min, sec);	
		buff += sprintf(buff, " %s %-3u %-7s  %-12s %-16s  %-7s %s\r\n",
						(vty->user.type == 0)?"+":" ", CMD_VTY_CONSOLE_ID, "Console", time_delay, "127.0.0.1", "-","-");
	}

	for (int i = 0; i < CMD_VTY_MAXUSER_NUM; i++)
	{
		CMD_VTY_S * vty_tel = cmd_vty_getById(i);
		if (vty_tel == NULL) {
			continue;
		}

		if (vty_tel->used == 0)
		{
			continue;
		}

		sockaddr_in addr;  
		socklen_t addrlen = sizeof(addr);  
		if(getpeername(vty_tel->user.socket, (struct sockaddr*)&addr, &addrlen) == -1){          
			continue;  
		}
		
		if (0 == vty_tel->user.state)
		{
			sprintf(time_delay, "Unauthorized");
		}
		else
		{
			diff = util_getdiftime(t, vty_tel->user.lastAccessTime);
			sec=diff%60;
			diff=diff/60;
			min=diff%60; 
			hour=diff=diff/60;
			sprintf(time_delay, "%02d:%02d:%02d", hour, min, sec);
			
		}

		buff += sprintf(buff, " %s %-3u %-7s  %-12s %-16s  %-7u %s\r\n",
			(vty->user.socket == vty_tel->user.socket)?"+":" ", vty_tel->vtyId, "Telnet", time_delay, inet_ntoa(addr.sin_addr), vty_tel->user.socket, vty_tel->user.state?vty_tel->user.user_name:"-");
	
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
	ULONG isTelnetuser = 0;
	CHAR Username[32] = {0};
	CHAR Password[32] = {0};
	ULONG isPort = 0;
	ULONG portNum = 23;
	ULONG isTelnetC = 0;
	char ip[25] = {0};
	extern void TELNET_KillAllVty();
	
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
				
			case TELNET_CMO_CFG_AUTH_MODE_NONE:		
				extern ULONG g_TelnetAuthMode ;
				extern char g_szTelnetUsername[32];
				extern char g_szTelnetPassword[32];
				
				if (g_TelnetAuthMode != 0)
				{
					TELNET_KillAllVty();
				}
				
				g_TelnetAuthMode = 0;
				memset(g_szTelnetUsername, 0, sizeof(g_szTelnetUsername));
				memset(g_szTelnetPassword, 0, sizeof(g_szTelnetPassword));
				break;
				
			case TELNET_CMO_CFG_AUTH_MODE_PASSWORD:	
				extern ULONG g_TelnetAuthMode ;	

				if (g_TelnetAuthMode != 1)
				{
					TELNET_KillAllVty();
				}
								
				g_TelnetAuthMode = 1;				
				vty_printf(vtyId, "Info: Please create telnet username and password.\r\n");				
				break;
				
			case TELNET_CMO_CFG_AUTH_MODE_AAA:		
				extern ULONG g_TelnetAuthMode ;
				extern char g_szTelnetUsername[32];
				extern char g_szTelnetPassword[32];	

				if (g_TelnetAuthMode != 2)
				{
					TELNET_KillAllVty();
				}
								
				g_TelnetAuthMode = 2;
				memset(g_szTelnetUsername, 0, sizeof(g_szTelnetUsername));
				memset(g_szTelnetPassword, 0, sizeof(g_szTelnetPassword));				
				vty_printf(vtyId, "Info: Please create AAA username and password.\r\n");
				break;
			
			case TELNET_CMO_CFG_TELNET_USER_NAME:
				isTelnetuser = OS_YES;
				cmd_copy_string_param(pElem, Username);					
				break;
				
			case TELNET_CMO_CFG_TELNET_USER_PSW:
				cmd_copy_string_param(pElem, Password);
				break;

			case TELNET_CMO_CFG_TELNET_CLIENT_IP:
				{
					ULONG ulIp = 0;					
					extern VOID cmd_ip_ulong_to_string(ULONG ip, CHAR *buf);
					
					ulIp = cmd_get_ip_ulong_param(pElem);
					cmd_ip_ulong_to_string(ulIp, ip);

					isTelnetC = OS_YES;
				}
			
				break;
			case TELNET_CMO_CFG_TELNET_SERVER_PORT:
				isPort = OS_YES;				
				break;
			case TELNET_CMO_CFG_TELNET_SERVER_PORT_NUM:		
				portNum = cmd_get_ulong_param(pElem);
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

		if (strcmp(g_szTelnetUsername, Username)
			|| strcmp(g_szTelnetPassword, Password))
		{
			TELNET_KillAllVty();
		}
		
		strcpy(g_szTelnetUsername, Username);
		strcpy(g_szTelnetPassword, Password);

		return 0;
	}
	
	if (isPort == OS_YES) {
		ulRet = TELNET_CFG_SetPort(portNum);
		if (OS_OK != ulRet){
			vty_printf(vtyId, "Error: Telnet server port set failed.\r\n");
		}
	}

	if (isTelnetC == OS_YES) {
		extern ULONG TELNETC_Run(ULONG vtyId,ULONG ulPort,CHAR* szIP);
		TELNETC_Run(vtyId, portNum, ip);
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

	CMD_VECTOR_NEW(vec);

	// 1
	cmd_regelement_new(CMD_ELEMID_NULL,				CMD_ELEM_TYPE_KEY,	  "display",	   "Display", vec);
	// 2
	cmd_regelement_new(TELNET_CMO_SHOW_USERS,			CMD_ELEM_TYPE_KEY,	  "users",		   "Users", vec);

	cmd_install_command(MID_TELNET, VIEW_GLOBAL,  " 1 2 ", vec);

	CMD_VECTOR_FREE(vec);

	return 0;
}

ULONG TELNET_RegCmdEnable()
{
	CMD_VECTOR_S * vec = NULL;

	CMD_VECTOR_NEW(vec);

	// 1
	cmd_regelement_new(TELNET_CMO_CFG_TELNET_UNDO,		CMD_ELEM_TYPE_KEY,	  "undo",		   "Undo Operation", vec);
	// 2
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "telnet",		   "Telnet protocol", vec);
	// 3
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "server",		   "Telnet Server",   vec);
	// 4
	cmd_regelement_new(TELNET_CMO_CFG_TELNET_ENABLE,	CMD_ELEM_TYPE_KEY,	  "enable",		   "Enable Telnet protocol", vec);
	// 5
	cmd_regelement_new(TELNET_CMO_CFG_AUTH_MODE,		CMD_ELEM_TYPE_KEY,	  "authentication-mode","Telnet authentication mode", vec);
	// 6
	cmd_regelement_new(TELNET_CMO_CFG_AUTH_MODE_NONE,	CMD_ELEM_TYPE_KEY,	  "none",			"Authentication mode none", vec);
	// 7
	cmd_regelement_new(TELNET_CMO_CFG_AUTH_MODE_AAA,	CMD_ELEM_TYPE_KEY,	  "aaa",			"Authentication mode aaa", vec);
	// 8
	cmd_regelement_new(TELNET_CMO_CFG_AUTH_MODE_PASSWORD,CMD_ELEM_TYPE_KEY,	  "password",		"Authentication mode password", vec);
	// 9
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "username",		"Telnet authentication username", vec);
	// 10
	cmd_regelement_new(TELNET_CMO_CFG_TELNET_USER_NAME,	CMD_ELEM_TYPE_STRING, "STRING<1-32>",	"Telnet authentication username", vec);
	// 11
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "password",		"Telnet authentication password", vec);
	// 12
	cmd_regelement_new(TELNET_CMO_CFG_TELNET_USER_PSW,	CMD_ELEM_TYPE_STRING, "STRING<1-32>",	"Telnet authentication password", vec);
	// 13
	cmd_regelement_new(TELNET_CMO_CFG_TELNET_CLIENT_IP,	CMD_ELEM_TYPE_IP, CMD_ELMT_IP,	"IP Address", vec);
	// 14
	cmd_regelement_new(TELNET_CMO_CFG_TELNET_SERVER_PORT, CMD_ELEM_TYPE_KEY,	  "port",		"Telnet server port", vec);
	// 15
	cmd_regelement_new(TELNET_CMO_CFG_TELNET_SERVER_PORT_NUM, CMD_ELEM_TYPE_INTEGER, "INTEGER<23-65535>",	"Telnet server port", vec);

	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 1 2 3 4 ", vec);
	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 2 3 4 ", vec);
	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 2 5 6 ", vec);
	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 2 5 7 ", vec);
	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 2 5 8 ", vec);
	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 2 9 10 11 12 ", vec);
	
	cmd_install_command(MID_TELNET, VIEW_GLOBAL,  " 2 13 ", vec);
	cmd_install_command(MID_TELNET, VIEW_GLOBAL,  " 2 13 15 ", vec);

	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 1 2 3 14 ", vec);
	cmd_install_command(MID_TELNET, VIEW_SYSTEM,  " 2 3 14 15 ", vec);

	CMD_VECTOR_FREE(vec);

	return 0;
}

ULONG TELNET_RegCmd()
{
	(VOID)cmd_regcallback(MID_TELNET, TELNET_CMD_Callback);	

	(VOID)TELNET_RegCmdShow();
	
	(VOID)TELNET_RegCmdEnable();

	(VOID)DEBUG_PUB_RegModuleDebugs(MID_TELNET, "telnet", "Telnet protocol");
	
	return OS_OK;
}

#endif

