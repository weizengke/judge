#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#ifdef WIN32
#include <io.h>
#include <winsock2.h>
#endif

#include <stdio.h>
#include <time.h>

#ifdef _LINUX_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <assert.h>
#endif

#include "kernel.h"

#include "include/osp_common_def.h"
#include "include/pdt_common_inc.h"
#include "icli.h"
#include "ic/include/debug_center_inc.h"
#include "event/include/event_pub.h"
#include "aaa/aaa.h"


#if (OS_YES == OSP_MODULE_AAA)

using namespace std;

enum AAA_CMO_TBLID_EM {
	AAA_CMO_TBL_SHOW = 0,
	AAA_CMO_TBL_CFG,		
};

enum AAA_CMO_SHOW_ID_EM {
	AAA_CMO_SHOW_USERS = CMD_ELEMID_DEF(MID_AAA, AAA_CMO_TBL_SHOW, 0),
};

enum AAA_CMO_CFG_ID_EM {
	AAA_CMO_CFG_TELNET_UNDO = CMD_ELEMID_DEF(MID_AAA, AAA_CMO_TBL_CFG, 0),
	AAA_CMO_CFG_AAA ,
	AAA_CMO_CFG_LOCAL_USER,
	AAA_CMO_CFG_LOCAL_USER_NAME,
	AAA_CMO_CFG_LOCAL_USER_PSW,	
	AAA_CMO_CFG_LOCAL_USER_SERVICE_TYPE_TELNET,
	AAA_CMO_CFG_LOCAL_USER_SERVICE_TYPE_FTP,
	AAA_CMO_CFG_LOCAL_USER_FTP_DIR,	
	AAA_CMO_CFG_LOCAL_USER_FTP_DIR_STRING,	
};


ULONG AAA_CFG_Callback(VOID *pRcvMsg)
{
	ULONG ulRet = OS_OK;
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	ULONG isUndo = 0;
	ULONG isLocaluser = 0;
	ULONG isFtpDir = 0;
	ULONG ulServiceType = 0;
	CHAR Username[32] = {0};
	CHAR Password[32] = {0};
	CHAR Directory[256] = {0};

	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case AAA_CMO_CFG_TELNET_UNDO:
				isUndo = 1;
				break;
				
			case AAA_CMO_CFG_AAA:
				vty_view_set(vtyId, VIEW_AAA);
				break;
				
			case AAA_CMO_CFG_LOCAL_USER_NAME:
				isLocaluser = OS_YES;
				cmd_copy_string_param(pElem, Username);				
				break;

			case AAA_CMO_CFG_LOCAL_USER_PSW:
				cmd_copy_string_param(pElem, Password);
				break;

			case AAA_CMO_CFG_LOCAL_USER_SERVICE_TYPE_TELNET:
				AAA_MASK_SET(ulServiceType, AAA_SERVICE_TYPE_TELNET);
				break;
				
			case AAA_CMO_CFG_LOCAL_USER_SERVICE_TYPE_FTP:
				AAA_MASK_SET(ulServiceType, AAA_SERVICE_TYPE_FTP);
				break;

			case AAA_CMO_CFG_LOCAL_USER_FTP_DIR:
				isFtpDir = OS_YES;			
				break;

			case AAA_CMO_CFG_LOCAL_USER_FTP_DIR_STRING:
				cmd_copy_string_param(pElem, Directory);
				break;

			default:
				break;
		}
	}

	if (OS_YES == isLocaluser)
	{
		if (0 == isUndo)
		{
			if (OS_OK != AAA_AddUser(vtyId, Username, Password, ulServiceType)) {
				return OS_ERR;
			}

			if (isFtpDir == OS_YES) {
				if (OS_OK != AAA_SetUserFtpDirectory(vtyId, Username, Directory)) {
					return OS_ERR;
				}
			} 

			if (ulServiceType != 0) {
				if (OS_OK != AAA_SetUserServiceType(vtyId, Username, ulServiceType, OS_NO)) {
					return OS_ERR;
				}
			}
		}
		else
		{
			if (isFtpDir == OS_YES) {
				if (OS_OK != AAA_SetUserFtpDirectory(vtyId, Username, NULL)) {
					return OS_ERR;
				}
			} else if (ulServiceType != 0) {
				if (OS_OK != AAA_SetUserServiceType(vtyId, Username, ulServiceType, OS_YES)) {
					return OS_ERR;
				}
			} else {
				if (OS_OK != AAA_DelUser(vtyId, Username)) {
					return OS_ERR;
				}
			}
		}
		
		return 0;
	}

	return ulRet;

}

ULONG AAA_CMD_Callback(VOID *pRcvMsg)
{
	ULONG iRet = 0;
	ULONG iTBLID = 0;
	
	iTBLID = cmd_get_first_elem_tblid(pRcvMsg);
			
	switch(iTBLID)
	{
		case AAA_CMO_TBL_CFG:
			iRet = AAA_CFG_Callback(pRcvMsg);
			break;

		default:
			break;
	}

	return iRet;
}

ULONG AAA_RegCmdEnable()
{
	CMD_VECTOR_S * vec = NULL;

	CMD_VECTOR_NEW(vec);

	// 1
	cmd_regelement_new(AAA_CMO_CFG_TELNET_UNDO,		CMD_ELEM_TYPE_KEY,	  "undo",		    "Undo Operation", vec);
	// 2
	cmd_regelement_new(AAA_CMO_CFG_AAA,				CMD_ELEM_TYPE_KEY,	  "aaa",		    "Authentication Authorization Accounting", vec);
	// 3
	cmd_regelement_new(CMD_ELEMID_NULL,				CMD_ELEM_TYPE_KEY,	  "local-user",		"AAA Local User", vec);
	// 4
	cmd_regelement_new(AAA_CMO_CFG_LOCAL_USER_NAME,	CMD_ELEM_TYPE_STRING, "STRING<1-32>",	"AAA Local User name", vec);
	// 5
	cmd_regelement_new(CMD_ELEMID_NULL,				CMD_ELEM_TYPE_KEY,	  "password",		"AAA Local User password", vec);
	// 6
	cmd_regelement_new(AAA_CMO_CFG_LOCAL_USER_PSW,  	CMD_ELEM_TYPE_STRING, "STRING<1-32>",	"AAA Local User password", vec);	
	// 7
	cmd_regelement_new(CMD_ELEMID_NULL,				CMD_ELEM_TYPE_KEY,	  "service-type",	"AAA user service type", vec);
	// 8
	cmd_regelement_new(AAA_CMO_CFG_LOCAL_USER_SERVICE_TYPE_TELNET,  	
														CMD_ELEM_TYPE_KEY, 	  "telnet",			"Service type Telnet", vec);	
	// 9
	cmd_regelement_new(AAA_CMO_CFG_LOCAL_USER_SERVICE_TYPE_FTP, CMD_ELEM_TYPE_KEY,    "ftp",			"Service type FTP", vec);	
	// 10
	cmd_regelement_new(AAA_CMO_CFG_LOCAL_USER_FTP_DIR,  CMD_ELEM_TYPE_KEY,    "ftp-directory",	"FTP directory", vec);	
	// 11
	cmd_regelement_new(AAA_CMO_CFG_LOCAL_USER_FTP_DIR_STRING,  	CMD_ELEM_TYPE_STRING, "STRING<1-255>",	"FTP directory", vec);	

	cmd_install_command(MID_AAA, VIEW_SYSTEM,  " 2 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 3 4 5 6 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 1 3 4 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 3 4 7 8 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 3 4 7 9 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 3 4 7 8 9 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 3 4 7 9 8 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 3 4 5 6 7 8 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 3 4 5 6 7 9 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 3 4 5 6 7 8 9 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 3 4 5 6 7 9 8 ", vec);	
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 3 4 10 11 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 3 4 7 9 10 11 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 3 4 5 6 7 9 10 11 ", vec);

	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 1 3 4 7 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 1 3 4 7 8 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 1 3 4 7 9 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 1 3 4 7 8 9 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 1 3 4 7 9 8 ", vec);
	cmd_install_command(MID_AAA, VIEW_AAA,  	 " 1 3 4 10 ", vec);

	CMD_VECTOR_FREE(vec);

	return 0;
}

ULONG AAA_RegCmd()
{
	(VOID)cmd_view_regist("aaa-view", "aaa", VIEW_AAA, VIEW_SYSTEM);
	
	(VOID)cmd_regcallback(MID_AAA, AAA_CMD_Callback);	

	(VOID)AAA_RegCmdEnable();

	(VOID)DEBUG_PUB_RegModuleDebugs(MID_AAA, "aaa", "Authentication Authorization Accounting");
	
	return OS_OK;
}

#endif
