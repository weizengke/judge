/*
	  风继续吹

	夜色如此放肆，
	从不知：
	风继续吹。
	瑟缩街中落泪，
	只有你，
	可细说，
	可倾诉。

            By Jungle Wei.

*/
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
#include "product\judge\include\judge_inc.h"
#include "product\thirdpart32\dbghelp\DbgHelp.h"

using namespace std;

vector <APP_INFO_S*> g_vector_appInfo;

#define APP_MAX_SYSNAME_SIZE 24
#define SOCKET_PORT 5000

char g_sysname[APP_MAX_SYSNAME_SIZE] = "judger";
char g_startup_config[256] = "config.cfg";
int  g_pdt_recovered = OS_NO;
HWND g_hWnd = NULL;

SOCKET g_sListen;
int g_sock_port = SOCKET_PORT;
char judgePath[MAX_PATH];
char logPath[MAX_PATH]="log\\";


enum PDT_CMO_TBLID_EM {
	PDT_CMO_TBL,		
};

enum PDT_CMO_ID_EM {
	PDT_CMO_UNDO = CMD_ELEMID_DEF(MID_OS, PDT_CMO_TBL, 0),
	PDT_CMO_VERSION,
	PDT_CMO_SYSNAME_STRING,
};

/* 判断是否是配置恢复阶段 */
ULONG PDT_IsCfgRecoverOver()
{
	return g_pdt_recovered;
}

char *PDT_GetSysname()
{
	return g_sysname;
}

int RegistAppInfo(APP_INFO_S *pstAppInfo)
{
	int ret = OS_OK;

	g_vector_appInfo.push_back(pstAppInfo);

	if (NULL != pstAppInfo->pfInitFunction)
	{
		ret = pstAppInfo->pfInitFunction();
		if (OS_OK != ret)
		{
			printf("%s Task init failed...\r\n", pstAppInfo->taskName);
			printf("%s Task RegistAppInfo failed...\r\n", pstAppInfo->taskName);
			return OS_ERR;
		}

		printf("%s Task init ok...\r\n", pstAppInfo->taskName);
	}

	printf("%s Task RegistAppInfo ok...\r\n", pstAppInfo->taskName);

	return OS_OK;

}

ULONG PDT_BuildRun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{
	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);
	
	pBuildrun = *ppBuildrun;

	pBuildrun += sprintf(pBuildrun, "#version %s", SOLFWARE_VERSION);
	pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"sysname %s", g_sysname);

	return OS_OK;
}

void PDT_CfgRecover()
{
	ULONG ulRet = OS_OK;
	CHAR cfgpath[256] = "config.cfg";
	CHAR line[1024] = {0};

	sprintf(cfgpath, "conf\\%s",g_startup_config);
	
	FILE * fp=fopen(cfgpath,"r");
	if (NULL == fp)
	{
		printf("No startup configuration file.\r\n");
		return;
	}

	printf("Recover configuration begin.\r\n");

	write_log(JUDGE_INFO, "Recover configuration begin.");

	cmd_pub_run("system-view");
	
	while (fgets(line,1024,fp))
	{
		if ('#' != line[0])
		{
			line[strlen(line) - 1] = '\0';
			ulRet = cmd_pub_run(line);
			//printf("\r\nRecover command:%s, ulRet=%u", line, ulRet);
			//write_log(JUDGE_INFO, "Recover command(ulRet=%u):%s", ulRet, line);
		}
	}
	
	fclose(fp);
	
	printf("Eecover configuration end.\r\n");
	write_log(JUDGE_INFO, "Eecover configuration end.");

	g_pdt_recovered = OS_YES;
	
	return;
}

void PDT_ShowCfg(ULONG vtyId)
{
	ULONG ulRet = OS_OK;
	char cfgpath[256] = "config.cfg";
	char line[1024] = {0};

	sprintf(cfgpath, "conf\\%s",g_startup_config);
	
	FILE * fp=fopen(cfgpath,"r");
	if (NULL == fp)
	{
		pdt_debug_print("No startup configuration file.");
		fclose(fp);
		return;
	}

	while (fgets(line,1024,fp))
	{
		vty_printf(vtyId,"%s", line);
	}

	vty_printf(vtyId, "\r\n");

	fclose(fp);

	return;
}

long WINAPI PDT_ExceptionFilter(EXCEPTION_POINTERS * excp)
{
	extern SOCKET g_TelnetSSocket;

	pdt_debug_print("Main Thread Exit...[code:%u]", GetLastError());
	write_log(JUDGE_ERROR," Main Thread Exit after 5 second...(GetLastError=%u)",GetLastError());
	
	closesocket(g_sListen);
	closesocket(g_TelnetSSocket);
	WSACleanup();
    
    Sleep(5000);
    ShellExecuteA(NULL,"open","judge.exe",NULL,NULL,SW_SHOWNORMAL);

	return EXCEPTION_EXECUTE_HANDLER;
}

unsigned _stdcall PDT_ListenThread(void *pEntry)
{
	sockaddr_in remoteAddr;
	SOCKET sClient;
	int nAddrLen = sizeof(remoteAddr);
	char buff[1024] = {0};
	char buff1024[1024] = {0};
	char *pBuf = NULL;
	while(TRUE)
	{
		sClient = accept(g_sListen, (SOCKADDR*)&remoteAddr, &nAddrLen);
		if(sClient == INVALID_SOCKET)
		{
			//write_log(JUDGE_ERROR,"Accept() Error");
			continue;
		}

		memset(buff,0,sizeof(buff));
		memset(buff1024,0,sizeof(buff1024));
		pBuf = buff1024;
		int ret=recv(sClient,(char*)buff,sizeof(buff),0);
		if(ret>0)
		{
			int i = 0;
			for (i = 0; i < 64; i++)
			{
				pBuf += sprintf(pBuf,"%02x ", buff[i]);
				if ((i + 1 )  % 16 == 0)
				{
					pBuf += sprintf(pBuf,"\r\n");
				}
			}

			write_log(JUDGE_INFO,"Recieve packet from (ip:%s, port:%u):\r\n%s",inet_ntoa(remoteAddr.sin_addr), htons(remoteAddr.sin_port), buff1024);
				
			PDT_Debug(DEBUG_TYPE_MSG, "Recieve packet from (ip:%s, port:%u):\r\n%s",inet_ntoa(remoteAddr.sin_addr), htons(remoteAddr.sin_port), buff1024);
			PDT_Debug(DEBUG_TYPE_MSG, "Message:%s", buff);

			/* 需要检查发送源的合法性 */
	
			(void)cmd_pub_run("system-view");
			(void)cmd_pub_run(buff);
		}

	
		Sleep(1);
	}

	write_log(JUDGE_ERROR,"ListenThread Crash");
	closesocket(sClient);

	return 0;
}

int PDT_InitSocket()
{
	write_log(JUDGE_INFO,"Start initialization of Socket...");

	WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);

	if(WSAStartup(sockVersion, &wsaData) != 0)
		return 0;

	g_sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(g_sListen == INVALID_SOCKET)
	{
		write_log(JUDGE_SYSTEM_ERROR,"create socket error");
		return 0;
	}

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(g_sock_port);
	sin.sin_addr.S_un.S_addr = htonl(INADDR_ANY);


	int trybind=50;
	int ret=0;

	ret = bind(g_sListen,(LPSOCKADDR)&sin,sizeof(sin));

	while(ret == SOCKET_ERROR && trybind > 0)
	{
		bind(g_sListen,(LPSOCKADDR)&sin,sizeof(sin));
		//write_log(JUDGE_SYSTEM_ERROR,"bind failed:%d , it will try later...",WSAGetLastError());
		trybind--;
		Sleep(100);
	}

	if(ret<0)
	{
		while (ret == SOCKET_ERROR)
		{
			g_sock_port++;
			sin.sin_port = htons(g_sock_port);
			ret =  bind(g_sListen,(LPSOCKADDR)&sin,sizeof(sin));
			if (ret != SOCKET_ERROR)
			{
				char szPort[10] = {0};
				(void)itoa(g_sock_port, szPort ,10);
			}
			Sleep(10);
		}
	}

	printf("Socket bind port %u ok...\r\n", g_sock_port);

	write_log(JUDGE_INFO,"Socket bind port %u ok...", g_sock_port);

	//进入监听状态
	int trylisten=50; //重试listen次数
	while((ret=listen(g_sListen,20))==SOCKET_ERROR&&trylisten)
	{
		write_log(JUDGE_SYSTEM_ERROR,"listen failed:%d , it will try later..",WSAGetLastError());
		trylisten--;
		Sleep(100);
	}

	if(ret<0)
	{
		write_log(JUDGE_SYSTEM_ERROR,"Listen failed...");
		printf("Error: Listen port(%u) failed......[code:%u]\r\n", g_sock_port, GetLastError());

		closesocket(g_sListen);
		WSACleanup();
		return 0;
	}

	printf("Socket listen ok...\r\n");
	write_log(JUDGE_INFO,"Socket listen ok...");

	return 1;
}

void PDT_DestroySocket()
{
	closesocket(g_sListen);
	WSACleanup();
	
	printf("Socket close ok...\r\n");
}

void PDT_InitConfigData()
{
	extern int g_judge_mode;
	extern int isDeleteTemp;
	extern DWORD OutputLimit;
	extern int JUDGE_LOG_BUF_SIZE;
	extern int g_vjudge_enable;
	extern int isRestrictedFunction;
	extern char workPath[MAX_PATH];
	extern char dataPath[MAX_PATH];
	extern char judgeLogPath[MAX_PATH];
	extern char Mysql_url[255];
	extern char Mysql_username[255];
	extern char Mysql_password[255];
	extern char Mysql_table[255];
	extern int  Mysql_port;
	extern char hdu_domain[256];
	extern char hdu_username[1000];
	extern char hdu_password[1000];
	extern char hdu_judgerIP[20];
	extern int hdu_sockport;
	extern int hdu_remote_enable;
	extern int hdu_vjudge_enable;

	GetPrivateProfileString("System","startup_config","config.cfg",g_startup_config,sizeof(g_startup_config),INI_filename);
	GetPrivateProfileString("System","sysname","Judge-Kernel",g_sysname,sizeof(g_sysname),INI_filename);
	g_sock_port=GetPrivateProfileInt("System","sock_port",SOCKET_PORT,INI_filename);
	GetPrivateProfileString("System","JudgePath","",judgePath,sizeof(judgePath),INI_filename);
	
	g_judge_mode=GetPrivateProfileInt("Judge","judge_mode",0,INI_filename);
	isDeleteTemp=GetPrivateProfileInt("Judge","DeleteTemp",0,INI_filename);
	OutputLimit=GetPrivateProfileInt("Judge","OutputLimit",10000,INI_filename);
	JUDGE_LOG_BUF_SIZE=GetPrivateProfileInt("Judge","judge_logbuf_size",500,INI_filename);
	g_vjudge_enable=GetPrivateProfileInt("Judge","vjudge_enable",OS_NO,INI_filename);
	isRestrictedFunction=GetPrivateProfileInt("Judge","isRestrictedFunction",0,INI_filename);
	GetPrivateProfileString("Judge","WorkingPath","",workPath,sizeof(workPath),INI_filename);
	GetPrivateProfileString("Judge","DataPath","D:\\OJ\\data\\",dataPath,sizeof(dataPath),INI_filename);

    if( (_access(dataPath, 0 )) == -1 )
    {
    	printf("Warnning: Data path '%s' is not exist, please check.", dataPath);
    	write_log(JUDGE_ERROR,"Judge data path '%s' is not exist, please check.", dataPath);
    }
    
	GetPrivateProfileString("Judge","JudgeLogPath","",judgeLogPath,sizeof(judgeLogPath),INI_filename);

	GetPrivateProfileString("MySQL","url","",Mysql_url,sizeof(Mysql_url),INI_filename);
	GetPrivateProfileString("MySQL","username","NULL",Mysql_username,sizeof(Mysql_username),INI_filename);
	GetPrivateProfileString("MySQL","password","NULL",Mysql_password,sizeof(Mysql_password),INI_filename);
	GetPrivateProfileString("MySQL","table","",Mysql_table,sizeof(Mysql_table),INI_filename);
	Mysql_port=GetPrivateProfileInt("MySQL","port",0,INI_filename);

	/* BEGIN: Added by weizengke,for hdu-vjudge*/
    GetPrivateProfileString("HDU","domain","http://acm.hdu.edu.cn",hdu_domain,sizeof(hdu_domain),INI_filename);
	GetPrivateProfileString("HDU","username","NULL",hdu_username,sizeof(hdu_username),INI_filename);
	GetPrivateProfileString("HDU","password","NULL",hdu_password,sizeof(hdu_password),INI_filename);
	GetPrivateProfileString("HDU","judgerIP","127.0.0.1",hdu_judgerIP,sizeof(hdu_judgerIP),INI_filename);
	hdu_sockport=GetPrivateProfileInt("HDU","sock_port",SOCKET_PORT,INI_filename);
	hdu_remote_enable=GetPrivateProfileInt("HDU","remote_enable",OS_NO,INI_filename);
	hdu_vjudge_enable=GetPrivateProfileInt("HDU","vjudge_enable",OS_NO,INI_filename);

#if 0
	/* BEGIN: Added by weizengke, for guet-dept3-vjudge */
	GetPrivateProfileString("GUET_DEPT3","username","NULL",guet_username,sizeof(guet_username),INI_filename);
	GetPrivateProfileString("GUET_DEPT3","password","NULL",guet_password,sizeof(guet_password),INI_filename);
	GetPrivateProfileString("GUET_DEPT3","judgerIP","127.0.0.1",guet_judgerIP,sizeof(guet_judgerIP),INI_filename);
	guet_sockport=GetPrivateProfileInt("GUET_DEPT3","sock_port",7706,INI_filename);
	guet_remote_enable=GetPrivateProfileInt("GUET_DEPT3","remote_enable",OS_NO,INI_filename);
	guet_vjudge_enable=GetPrivateProfileInt("GUET_DEPT3","vjudge_enable",OS_NO,INI_filename);
#endif

	write_log(JUDGE_INFO,"DataPath:%s, Workpath:%s",dataPath,workPath);
	write_log(JUDGE_INFO,"MySQL:%s %s %s %s %d",Mysql_url,Mysql_username,Mysql_password,Mysql_table,Mysql_port);

}


ULONG pdt_cfg_callback(VOID *pRcvMsg)
{
	ULONG ulRet = OS_OK;
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	ULONG isVersion = 0;
	ULONG isSysname = 0;
	
	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case PDT_CMO_VERSION:
				isVersion = OS_YES;
				break;
				
			case PDT_CMO_SYSNAME_STRING:
				cmd_copy_string_param(pElem, g_sysname);
				isSysname = OS_YES;
				break;
				
			default:
				break;
		}
	}
	
	if (OS_YES == isVersion)
	{
		vty_printf(vtyId, 
			" Kernel version: %s, released at %s %s. \r\n Copyright @ 2011-2018 debugforces.com. All Rights Reserved. \r\n",
			SOLFWARE_VERSION,
			__TIME__,
			__DATE__);
		
		return OS_OK;
	}

	if (OS_YES == isSysname)
	{
		::SetConsoleTitle(g_sysname);
		return OS_OK;
	}	
	
	return ulRet;

}

ULONG pdt_cmd_callback(VOID *pRcvMsg)
{
	ULONG iRet = 0;
	ULONG iTBLID = 0;
	
	iTBLID = cmd_get_first_elem_tblid(pRcvMsg);
		
	switch(iTBLID)
	{
		case PDT_CMO_TBL:
			iRet = pdt_cfg_callback(pRcvMsg);
			break;

		default:
			break;
	}

	return iRet;
}

ULONG pdt_reg_cmd()
{
	CMD_VECTOR_S * vec = NULL;

	/* 命令行注册四部曲1: 申请命令行向量 */
	CMD_VECTOR_NEW(vec);

	/* 命令行注册四部曲2: 定义命令字 */
	cmd_regelement_new(PDT_CMO_VERSION,					CMD_ELEM_TYPE_KEY,	  "version",		   "Version", vec);
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "sysname",		   "Change System Name", vec);
	cmd_regelement_new(PDT_CMO_SYSNAME_STRING,			CMD_ELEM_TYPE_STRING, "STRING<1-24>",	   "System Name",   vec);

	/* 命令行注册四部曲3: 注册命令行 */
	cmd_install_command(MID_OS, VIEW_DIAGNOSE,  " 1 ", vec);
	cmd_install_command(MID_OS, VIEW_SYSTEM,    " 2 3 ", vec);

	/* 命令行注册四部曲4: 释放命令行向量 */
	CMD_VECTOR_FREE(vec);

	return OS_OK;
}


void pdt_init()
{
	SetUnhandledExceptionFilter(PDT_ExceptionFilter);
 	SetErrorMode(SEM_NOGPFAULTERRORBOX );

	if( (_access(logPath, 0 )) == -1 )
	{
		CreateDirectory(logPath,NULL);
	}

	PDT_InitConfigData();

	::SetConsoleTitle(g_sysname);

	g_hWnd=::GetConsoleWindow();

	if (g_hWnd !=NULL)
	{
		HANDLE hIcon=NULL;
		hIcon=::LoadImage(GetModuleHandle(NULL),"beyond.ico",
		IMAGE_ICON,32,32,LR_LOADFROMFILE);
		if (hIcon!=NULL)
		{
			::SendMessage(g_hWnd,WM_SETICON,ICON_SMALL,(LPARAM)hIcon);
		}

	}
	else
	{
		pdt_debug_print("Set judge kernel ico failed. ");
	}

	
}

int main()
{
	pdt_init();

	printf("=====================================================\r\nOS Main-task Running...\r\n");

	extern void Cmd_RegAppInfo();
	Cmd_RegAppInfo();

	extern void Debug_RegAppInfo();
	Debug_RegAppInfo();

	extern void TELNET_RegAppInfo();
	TELNET_RegAppInfo();

	extern void Judge_RegAppInfo();
	Judge_RegAppInfo();

	extern void NDP_RegAppInfo();
	NDP_RegAppInfo();
	
	for (vector<int>::size_type ix = 0; ix < g_vector_appInfo.size(); ++ix)
	{
		if (NULL != g_vector_appInfo[ix]->pfTaskMain)
		{
			_beginthreadex(NULL, 0, g_vector_appInfo[ix]->pfTaskMain, NULL, NULL, NULL);
			//_beginthread(g_vector_appInfo[ix]->pfTaskMain,0,NULL);

			printf("%s Task running ok...\r\n", g_vector_appInfo[ix]->taskName);
		}
	}

	printf("OS Main-task init ok...\r\n");

	if(PDT_InitSocket()==0)
	{
		write_log(JUDGE_ERROR,"Init Socket JUDGE_ERROR...");
	}

	(void)BDN_RegistBuildRun(MID_OS, VIEW_SYSTEM, BDN_PRIORITY_HIGH + 4094, PDT_BuildRun);

	{
		extern ULONG CMD_ADP_Init();
		CMD_ADP_Init();
	}

	pdt_reg_cmd();
	
	(VOID)cmd_regcallback(MID_OS, pdt_cmd_callback);	

	Sleep(1000);
	
	/* 配置恢复处理 */
	PDT_CfgRecover();

	//printf("=====================================================\r\n");
	
	_beginthreadex(NULL, 0, PDT_ListenThread, NULL, NULL, NULL);

	printf("Press any key to continue.\r\n");


	/* 循环读取消息队列 */
	for ( ; ; )
	{
		RunDelay(1);
	}


	return 0;
}
