#include "ftp_server.h"

#include "osp/debug/include/debug_center_inc.h"
#include "product/include/pdt_common_inc.h"
#include "osp/event/include/event_pub.h"
#include "osp/command/include/icli.h"
#include "osp/aaa/aaa.h"
#include "product/main/root.h"

#if (OS_YES == OSP_MODULE_FTPS)

enum FTPS_CMO_TBLID_EM {
	FTPS_CMO_TBL_CFG = 0,		
};

enum FTPS_CMO_CFG_ID_EM {
	FTPS_CMO_CFG_UNDO = CMD_ELEMID_DEF(MID_FTP, FTPS_CMO_TBL_CFG, 0),
	FTPS_CMO_CFG_ENABLE ,
};

extern BOOL g_ulFtpEnable;
extern ULONG FTPS_Enable();
extern ULONG FTPS_Disable();

ULONG FTPS_CFG_Callback(VOID *pRcvMsg)
{
	ULONG ulRet = OS_OK;
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	ULONG isUndo = 0;
	ULONG isEnable = 0;

	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case FTPS_CMO_CFG_UNDO:
				isUndo = 1;
				break;
				
			case FTPS_CMO_CFG_ENABLE:		
				isEnable = OS_YES;
				break;

			default:
				break;
		}
	}

	if (OS_YES == isEnable)
	{
		if (0 == isUndo)
		{
			ulRet = FTPS_Enable();
			if (OS_OK != ulRet)
			{
				vty_printf(vtyId, "Error: FTP server enable failed.\r\n");
			}			
		}
		else
		{
			ulRet = FTPS_Disable();
			if (OS_OK != ulRet)
			{
				vty_printf(vtyId, "Error: FTP server disable failed.\r\n");
			}
		}
		
		return 0;
	}
	
	return ulRet;
}


ULONG FTPS_CMD_Callback(VOID *pRcvMsg)
{
	ULONG iRet = 0;
	ULONG iTBLID = 0;
	
	iTBLID = cmd_get_first_elem_tblid(pRcvMsg);
		
	switch(iTBLID)
	{
		case FTPS_CMO_TBL_CFG:
			iRet = FTPS_CFG_Callback(pRcvMsg);
			break;

		default:
			break;
	}

	return iRet;
}


ULONG FTPS_RegCmdEnable()
{
	CMD_VECTOR_S * vec = NULL;

	/* 命令行注册四部曲1: 申请命令行向量 */
	CMD_VECTOR_NEW(vec);

	/* 命令行注册四部曲2: 定义命令字 */
	// 1
	cmd_regelement_new(FTPS_CMO_CFG_UNDO,		CMD_ELEM_TYPE_KEY,	  "undo",		   "Undo Operation", vec);
	// 
	cmd_regelement_new(CMD_ELEMID_NULL,		CMD_ELEM_TYPE_KEY,	  "ftp",		   "FTP protocol", vec);
	// 3
	cmd_regelement_new(CMD_ELEMID_NULL,		CMD_ELEM_TYPE_KEY,	  "server",		   "FTP Server",   vec);
	// 4
	cmd_regelement_new(FTPS_CMO_CFG_ENABLE,	CMD_ELEM_TYPE_KEY,	  "enable",		   "Enable FTP service", vec);

	/* 命令行注册四部曲3: 注册命令行 */
	cmd_install_command(MID_FTP, VIEW_SYSTEM,  " 1 2 3 4 ", vec);
	cmd_install_command(MID_FTP, VIEW_SYSTEM,  " 2 3 4 ", vec);
	
	/* 命令行注册四部曲4: 释放命令行向量 */
	CMD_VECTOR_FREE(vec);

}

ULONG FTPS_RegCmd()
{
	(VOID)cmd_regcallback(MID_FTP, FTPS_CMD_Callback);	

	(VOID)FTPS_RegCmdEnable();
	
	return OS_OK;
}

ULONG FTPS_BuildRun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{
	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);
	
	pBuildrun = *ppBuildrun;

	if (TRUE == g_ulFtpEnable)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"ftp server enable");
	}
	else
	{
		if (OS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"undo ftp server enable");
		}
	}
	
	return OS_OK;
}

ULONG FTPS_EVT_UserDel(ULONG keyId, ULONG cmdId, VOID *pData, VOID **ppInfo)
{

	return OS_OK;
}

int FTPS_Init()
{
	(VOID)AAA_EvtRegistFunc("FTPS", AAA_EVT_USER_DEL, 0, FTPS_EVT_UserDel);
	
	return FTP_OK;
}

int FTPS_TaskEntry(void *pEntry)
{	
	(VOID)FTPS_RegCmd();

	(VOID)BDN_RegistBuildRun(MID_FTP, VIEW_SYSTEM, BDN_PRIORITY_HIGH + 100, FTPS_BuildRun);
	
	/* 循环读取消息队列 */
	for(;;)
	{
		/* 放权 */
		Sleep(1000);
	}

	return FTP_OK;
}

APP_INFO_S g_FTPSAppInfo =
{
	MID_FTP,
	"FTPS",
	FTPS_Init,
	FTPS_TaskEntry
};

void FTPS_RegAppInfo()
{
	APP_RegistInfo(&g_FTPSAppInfo);
}

#endif
