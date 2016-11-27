/*

	Author     : Jungle Wei
	Create Date: 2011-08
	Description: For Online Judge Core

	江南春．眉间心上

		夜静寂，
		枕头欹。
		念此事都来，
		无力相回避。
		谙尽唏嘘尤生泪，
		眉间心上皆似醉。

*/

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


#include "product\judge\include\judge_inc.h"


using namespace std;


char INI_filename[] = STARTUP_CFG;

int isDeleteTemp=0;
int isRestrictedFunction=0;
int  limitJudge=50;
DWORD OutputLimit=10000;
char workPath[MAX_PATH];
char judgeLogPath[MAX_PATH];
int JUDGE_LOG_BUF_SIZE = 200;
char dataPath[MAX_PATH];
char logPath[MAX_PATH]="log\\";
char judgePath[MAX_PATH];

#define PORT 5000
#define BUFFER 1024

int g_sock_port=PORT;

typedef struct tagJudge_Data_S
{
	int solutionId;

}JUDGE_DATA_S;

#if 0
typedef enum tagMsg_Type_E
{
	MSG_TYPE_NULL = 0,
	MSG_TYPE_JUDGE_REQUEST,
	MSG_TYPE_JUDGE_DATA,

	MSG_TYPE_MAX
}MSG_TYPE_E;

typedef struct tagMBuff_S
{
	sockaddr_in  sock_addr_des;
	sockaddr_in  sock_addr_src;

	int priority;

	int module_id_des;
	int module_id_src;

	int msg_type;

	int data_length;
	char * pszData;

}MBUF_S;
#endif

queue <JUDGE_DATA_S> g_JudgeQueue; /* 全局队列 */

extern void pdt_debug_print(const char *format, ...);

ULONG Judge_DebugSwitch(ULONG st)
{
	g_oj_debug_switch = st;

	return OS_TRUE;
}

int Judge_IsVirtualJudgeEnable()
{
	if (GL_vjudge_enable == OS_YES)
	{
		return OS_YES;
	}

	return OS_NO;
}

/* #pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup") */

int Judge_InitSocket()
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
	sin.sin_addr.S_un.S_addr = INADDR_ANY;


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
				WritePrivateProfileString("System","sock_port",szPort,INI_filename);
			}
			Sleep(10);
		}
	}

	pdt_debug_print("Info: Socket bind port(%u) ok.", g_sock_port);

	write_log(JUDGE_INFO,"Bind success...");

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
		pdt_debug_print("Error: Listen port(%u) failed......[code:%u]", g_sock_port, GetLastError());

		closesocket(g_sListen);
		WSACleanup();
		return 0;
	}

	pdt_debug_print("Info: Socket listen ok.");
	write_log(JUDGE_INFO,"Listen success...");

	return 1;
}
//////////////////////////////////////////////////////////////end socket

void Judge_Destroy()
{
	closesocket(g_sListen);
}

void Judge_InitConfigData()
{
	g_sock_port=GetPrivateProfileInt("System","sock_port",PORT,INI_filename);
	GetPrivateProfileString("System","JudgePath","",judgePath,sizeof(judgePath),INI_filename);

	isDeleteTemp=GetPrivateProfileInt("Judge","DeleteTemp",0,INI_filename);
	limitJudge=GetPrivateProfileInt("Judge","LimitJudge",20,INI_filename);
	OutputLimit=GetPrivateProfileInt("Judge","OutputLimit",10000,INI_filename);
	JUDGE_LOG_BUF_SIZE=GetPrivateProfileInt("Judge","judge_logbuf_size",500,INI_filename);
	GL_vjudge_enable=GetPrivateProfileInt("Judge","vjudge_enable",OS_NO,INI_filename);
	isRestrictedFunction=GetPrivateProfileInt("Judge","isRestrictedFunction",0,INI_filename);
	GetPrivateProfileString("Judge","WorkingPath","",workPath,sizeof(workPath),INI_filename);
	GetPrivateProfileString("Judge","DataPath","",dataPath,sizeof(dataPath),INI_filename);

	GetPrivateProfileString("Judge","JudgeLogPath","",judgeLogPath,sizeof(judgeLogPath),INI_filename);

	GetPrivateProfileString("MySQL","url","",Mysql_url,sizeof(Mysql_url),INI_filename);
	GetPrivateProfileString("MySQL","username","NULL",Mysql_username,sizeof(Mysql_username),INI_filename);
	GetPrivateProfileString("MySQL","password","NULL",Mysql_password,sizeof(Mysql_password),INI_filename);
	GetPrivateProfileString("MySQL","table","",Mysql_table,sizeof(Mysql_table),INI_filename);
	Mysql_port=GetPrivateProfileInt("MySQL","port",0,INI_filename);

	/* BEGIN: Added by weizengke,for hdu-vjudge*/
	GetPrivateProfileString("HDU","username","NULL",hdu_username,sizeof(hdu_username),INI_filename);
	GetPrivateProfileString("HDU","password","NULL",hdu_password,sizeof(hdu_password),INI_filename);
	GetPrivateProfileString("HDU","judgerIP","127.0.0.1",hdu_judgerIP,sizeof(hdu_judgerIP),INI_filename);
	hdu_sockport=GetPrivateProfileInt("HDU","sock_port",6606,INI_filename);
	hdu_remote_enable=GetPrivateProfileInt("HDU","remote_enable",OS_NO,INI_filename);
	hdu_vjudge_enable=GetPrivateProfileInt("HDU","vjudge_enable",OS_NO,INI_filename);

	/* BEGIN: Added by weizengke, for guet-dept3-vjudge */
	GetPrivateProfileString("GUET_DEPT3","username","NULL",guet_username,sizeof(guet_username),INI_filename);
	GetPrivateProfileString("GUET_DEPT3","password","NULL",guet_password,sizeof(guet_password),INI_filename);
	GetPrivateProfileString("GUET_DEPT3","judgerIP","127.0.0.1",guet_judgerIP,sizeof(guet_judgerIP),INI_filename);
	guet_sockport=GetPrivateProfileInt("GUET_DEPT3","sock_port",7706,INI_filename);
	guet_remote_enable=GetPrivateProfileInt("GUET_DEPT3","remote_enable",OS_NO,INI_filename);
	guet_vjudge_enable=GetPrivateProfileInt("GUET_DEPT3","vjudge_enable",OS_NO,INI_filename);

	write_log(JUDGE_INFO,"Socketport:%d, Data:%s, Workpath:%s",g_sock_port,dataPath,workPath);
	write_log(JUDGE_INFO,"MySQL:%s %s %s %s %d",Mysql_url,Mysql_username,Mysql_password,Mysql_table,Mysql_port);

}

void Judge_InitJudgePath(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	if (NULL == pstJudgeSubmission)
	{
		write_log(JUDGE_ERROR,"Judge_InitJudgePath ERROR, pstJudgeSubmission is NULL....");
		return ;
	}

	if( (_access(workPath, 0 )) == -1 )
	{
		CreateDirectory(workPath,NULL);
	}

	char keyname[100]={0};
	sprintf(keyname,"Language%d", pstJudgeSubmission->stSolution.languageId);

	GetPrivateProfileString("Language",keyname,"",pstJudgeSubmission->languageName,100,INI_filename);

	GetPrivateProfileString("LanguageExt",pstJudgeSubmission->languageName,"",pstJudgeSubmission->languageExt,10,INI_filename);

	GetPrivateProfileString("LanguageExe",pstJudgeSubmission->languageName,"",pstJudgeSubmission->languageExe,10,INI_filename);

	GetPrivateProfileString("CompileCmd",pstJudgeSubmission->languageName,"",pstJudgeSubmission->compileCmd,1024,INI_filename);

	GetPrivateProfileString("RunCmd",pstJudgeSubmission->languageName,"",pstJudgeSubmission->runCmd,1024,INI_filename);

	GetPrivateProfileString("SourcePath",pstJudgeSubmission->languageName,"",pstJudgeSubmission->sourcePath,1024,INI_filename);

	GetPrivateProfileString("ExePath",pstJudgeSubmission->languageName,"",pstJudgeSubmission->exePath,1024,INI_filename);

	pstJudgeSubmission->isTranscoding=GetPrivateProfileInt("Transcoding",pstJudgeSubmission->languageName,0,INI_filename);

	pstJudgeSubmission->limitIndex=GetPrivateProfileInt("TimeLimit",pstJudgeSubmission->languageName,1,INI_filename);

	pstJudgeSubmission->nProcessLimit=GetPrivateProfileInt("ProcessLimit",pstJudgeSubmission->languageName,1,INI_filename);


	char buf[1024];
	sprintf(buf, "%d", pstJudgeSubmission->stSolution.solutionId);
	string name = buf;
	string compile_string=pstJudgeSubmission->compileCmd;
	replace_all_distinct(compile_string,"%PATH%",workPath);
	replace_all_distinct(compile_string,"%SUBPATH%",pstJudgeSubmission->subPath);
	replace_all_distinct(compile_string,"%NAME%",name);
	replace_all_distinct(compile_string,"%EXT%",pstJudgeSubmission->languageExt);
	replace_all_distinct(compile_string,"%EXE%",pstJudgeSubmission->languageExe);
	strcpy(pstJudgeSubmission->compileCmd,compile_string.c_str());       /* 编译命令行 */

	string runcmd_string=pstJudgeSubmission->runCmd;
	replace_all_distinct(runcmd_string,"%PATH%",workPath);
	replace_all_distinct(runcmd_string,"%SUBPATH%",pstJudgeSubmission->subPath);
	replace_all_distinct(runcmd_string,"%NAME%",name);
	replace_all_distinct(runcmd_string,"%EXT%",pstJudgeSubmission->languageExt);
	replace_all_distinct(runcmd_string,"%EXE%",pstJudgeSubmission->languageExe);
	strcpy(pstJudgeSubmission->runCmd,runcmd_string.c_str());			 /* 运行命令行*/

	string sourcepath_string=pstJudgeSubmission->sourcePath;
	replace_all_distinct(sourcepath_string,"%PATH%",workPath);
	replace_all_distinct(sourcepath_string,"%SUBPATH%",pstJudgeSubmission->subPath);
	replace_all_distinct(sourcepath_string,"%NAME%",name);
	replace_all_distinct(sourcepath_string,"%EXT%",pstJudgeSubmission->languageExt);
	strcpy(pstJudgeSubmission->sourcePath,sourcepath_string.c_str());		 /* 源程序路径*/

	string exepath_string=pstJudgeSubmission->exePath;
	replace_all_distinct(exepath_string,"%PATH%",workPath);
	replace_all_distinct(exepath_string,"%SUBPATH%",pstJudgeSubmission->subPath);
	replace_all_distinct(exepath_string,"%NAME%",name);
	replace_all_distinct(exepath_string,"%EXE%",pstJudgeSubmission->languageExe);
	strcpy(pstJudgeSubmission->exePath,exepath_string.c_str());				 /* 可执行文件路径*/

	sprintf(pstJudgeSubmission->DebugFile,"%s%s%s.txt",workPath,pstJudgeSubmission->subPath,name.c_str());  /* debug文件路径*/
	sprintf(pstJudgeSubmission->ErrorFile,"%s%s%s_re.txt",workPath,pstJudgeSubmission->subPath,name.c_str());  /* re文件路径*/

	if( (_access(judgeLogPath, 0 )) == -1 )
	{
		CreateDirectory(judgeLogPath,NULL);
	}

	sprintf(pstJudgeSubmission->judge_log_filename,"%sjudge-log-%d.log",judgeLogPath,pstJudgeSubmission->stSolution.solutionId);

}

void Judge_InitSubmissionData(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	pstJudgeSubmission->stSolution.verdictId = V_AC;
	pstJudgeSubmission->stSolution.contestId = 0;
	pstJudgeSubmission->stSolution.time_used = 0;
	pstJudgeSubmission->stSolution.memory_used = 0;
	pstJudgeSubmission->stSolution.testcase = 0;
	pstJudgeSubmission->stSolution.reJudge = 0;

	pstJudgeSubmission->stProblem.time_limit = 1000;
	pstJudgeSubmission->stProblem.memory_limit = 65535;

	pstJudgeSubmission->isTranscoding = 0;
	pstJudgeSubmission->limitIndex = 1;
	pstJudgeSubmission->nProcessLimit = 1;

	time_t timep;
	time(&timep);
	srand((int)time(0)*3);
	pstJudgeSubmission->ulSeed = timep + rand();

	sprintf(pstJudgeSubmission->subPath, "%d_%u\\", pstJudgeSubmission->stSolution.solutionId, pstJudgeSubmission->ulSeed);

	char fullPath[1024] = {0};
	sprintf(fullPath, "%s%s", workPath, pstJudgeSubmission->subPath);
	while( (_access(fullPath, 0 )) != -1 )
	{
		write_log(JUDGE_INFO,"Gernerate another Seed...(%u)", pstJudgeSubmission->ulSeed);
		Sleep(10);
		pstJudgeSubmission->ulSeed = timep + rand();

		sprintf(pstJudgeSubmission->subPath, "%d_%u\\", pstJudgeSubmission->stSolution.solutionId, pstJudgeSubmission->ulSeed);
		sprintf(fullPath, "%s%s", workPath, pstJudgeSubmission->subPath);
	}

	CreateDirectory(fullPath,NULL);

}

void Judge_ShowCfgContent()
{
	HANDLE hFile ;
	hFile= CreateFile(STARTUP_CFG, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_READONLY, NULL);
	if (hFile <= 0)
	{
		write_log(JUDGE_ERROR,"CreateFile inFileName(%s) Error:%s", STARTUP_CFG, GetLastError());
	}

	BOOL flag = FALSE;
	while (true)
	{
		char buffer[BUFSIZE] = {0};
		DWORD BytesRead, BytesWritten;
		flag = ReadFile(hFile, buffer, BUFSIZE, &BytesRead, NULL);
		if (!flag || (BytesRead == 0)) break;

		judge_outstring("%s",buffer);

		if (!flag){ break;}
	}

	CloseHandle(hFile);

}

HANDLE Judge_CreateSandBox(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	HANDLE hjob =CreateJobObject(NULL,NULL);
	if(hjob!=NULL)
	{
		JOBOBJECT_BASIC_LIMIT_INFORMATION jobli;
		 memset(&jobli,0,sizeof(jobli));
		jobli.LimitFlags=JOB_OBJECT_LIMIT_PRIORITY_CLASS|JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
		jobli.PriorityClass=IDLE_PRIORITY_CLASS;
		jobli.ActiveProcessLimit=pstJudgeSubmission->nProcessLimit;
	//	jobli.MinimumWorkingSetSize= 1;
	//	jobli.MaximumWorkingSetSize= 1024*GL_memory_limit;|JOB_OBJECT_LIMIT_WORKINGSET|JOB_OBJECT_LIMIT_PROCESS_TIME
	//	jobli.PerProcessUserTimeLimit.QuadPart=10000*(GL_time_limit+2000);
		if(SetInformationJobObject(hjob,JobObjectBasicLimitInformation,&jobli,sizeof(jobli)))
		{
			JOBOBJECT_BASIC_UI_RESTRICTIONS jobuir;
			jobuir.UIRestrictionsClass=JOB_OBJECT_UILIMIT_NONE;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_EXITWINDOWS;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_READCLIPBOARD ;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_WRITECLIPBOARD ;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_HANDLES;

			if(SetInformationJobObject(hjob,JobObjectBasicUIRestrictions,&jobuir,sizeof(jobuir)))
			{
				return hjob;
			}
			else
			{
				write_log(JUDGE_SYSTEM_ERROR,"SetInformationJobObject  JOBOBJECT_BASIC_UI_RESTRICTIONS   [Error:%d]\n",GetLastError());
			}
		}
		else
		{
			write_log(JUDGE_SYSTEM_ERROR,"SetInformationJobObject  JOBOBJECT_BASIC_LIMIT_INFORMATION   [Error:%d]\n",GetLastError());
		}
	}
	else
	{
		write_log(JUDGE_SYSTEM_ERROR,"CreateJobObject     [Error:%d]\n",GetLastError());
	}
	return NULL;
}

bool Judge_ProcessToSandbox(HANDLE job,PROCESS_INFORMATION p)
{
	if(AssignProcessToJobObject(job,p.hProcess))
	{
		/* 顺便调整本进程优先级为高 */
		/*
		HANDLE   hPS   =   OpenProcess(PROCESS_ALL_ACCESS,   false,  p.dwProcessId);
		if(!SetPriorityClass(hPS,   HIGH_PRIORITY_CLASS))
		{
			write_log(JUDGE_SYSTEM_ERROR,"SetPriorityClass        [Error:%d]\n",GetLastError());
		}
		CloseHandle(hPS);
		*/
		return true;
	}
	else
	{
		write_log(JUDGE_SYSTEM_ERROR,"AssignProcessToJobObject Error:%s",GetLastError());
	}

	return false;
}

unsigned _stdcall Judge_CompileThread(void *pData)
{
	JUDGE_SUBMISSION_ST *pstJudgeSubmission = (JUDGE_SUBMISSION_ST *)pData;

	if (NULL == pstJudgeSubmission)
	{
		write_log(JUDGE_ERROR,"Judge_CompileThread ERROR, pstJudgeSubmission is NULL....");
		return 0;
	}

	write_log(JUDGE_INFO,"Enter Judge_CompileThread...");

	SQL_updateSolution(pstJudgeSubmission->stSolution.solutionId, V_C, 0, 0, 0);

	system(pstJudgeSubmission->compileCmd);

	write_log(JUDGE_INFO,"End Judge_CompileThread...");

	return 0;
}

int Judge_CompileProc(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	if (NULL == pstJudgeSubmission)
	{
		write_log(JUDGE_ERROR,"Judge_CompileProc ERROR, pstJudgeSubmission is NULL....");
		return 0;
	}

	if(strcmp(pstJudgeSubmission->runCmd,"NULL")==0) return 1;

	HANDLE hThread_com;

	hThread_com = (HANDLE)_beginthreadex(NULL, NULL, Judge_CompileThread, (void*)pstJudgeSubmission, 0, NULL);
	if(hThread_com == NULL)
	{
		write_log(JUDGE_ERROR,"Create Judge_CompileThread Error");
		CloseHandle(hThread_com);
		return 0;
	}

	write_log(JUDGE_INFO,"Create Judge_CompileThread ok...");

	DWORD status_ = WaitForSingleObject(hThread_com,30000);
	if(status_ > 0)
	{
		write_log(JUDGE_WARNING,"Compile over time_limit");
	}

	write_log(JUDGE_INFO,"WaitForSingleObject wait time ok...");

	if( (_access(pstJudgeSubmission->exePath, 0 )) != -1 )
	{
		/* ok */
		return 1;
	}
	else
	{
		return 0;
	}
}

BOOL Judge_ExistException(DWORD dw)
{
	switch(dw)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		return TRUE;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		return TRUE;
	case EXCEPTION_BREAKPOINT:
		return TRUE;
	case EXCEPTION_SINGLE_STEP:
		return TRUE;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		return TRUE;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		return TRUE;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		return TRUE;
	case EXCEPTION_FLT_INEXACT_RESULT:
		return TRUE;
	case EXCEPTION_FLT_INVALID_OPERATION:
		return TRUE;
	case EXCEPTION_FLT_OVERFLOW:
		return TRUE;
	case EXCEPTION_FLT_STACK_CHECK:
		return TRUE;
	case EXCEPTION_FLT_UNDERFLOW:
		return TRUE;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		return TRUE;
	case EXCEPTION_INT_OVERFLOW:
		return TRUE;
	case EXCEPTION_PRIV_INSTRUCTION:
		return TRUE;
	case EXCEPTION_IN_PAGE_ERROR:
		return TRUE;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		return TRUE;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		return TRUE;
	case EXCEPTION_STACK_OVERFLOW:
		return TRUE;
	case EXCEPTION_INVALID_DISPOSITION:
		return TRUE;
	case EXCEPTION_GUARD_PAGE:
		return TRUE;
	case EXCEPTION_INVALID_HANDLE:
		return TRUE;
	default:
		return FALSE;
	}
}

unsigned _stdcall Judge_RunProgramThread(void *pData) //ac
{
	/* cmd/c solution.exe <data.in >data.out 2>error.txt */
	/* ChildIn_Write是子进程的输入句柄，ChildIn_Read是父进程用于写入子进程输入的句柄 */
	HANDLE ChildIn_Read, ChildIn_Write;
	/*ChildOut_Write是子进程的输出句柄，ChildOut_Read是父进程用于读取子进程输出的句柄*/
	HANDLE ChildOut_Read, ChildOut_Write;

	SECURITY_ATTRIBUTES saAttr = {0};

	JUDGE_SUBMISSION_ST *pstJudgeSubmission = (JUDGE_SUBMISSION_ST *)pData;
	if (NULL == pstJudgeSubmission)
	{
		write_log(JUDGE_ERROR,"Judge_RunProgramThread ERROR, pstJudgeSubmission is NULL....");
		return 0;
	}

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	CreatePipe(&ChildIn_Read, &ChildIn_Write, &saAttr, 0);
	SetHandleInformation(ChildIn_Write, HANDLE_FLAG_INHERIT, 0);
	CreatePipe(&ChildOut_Read, &ChildOut_Write, &saAttr, 0);
	SetHandleInformation(ChildOut_Read, HANDLE_FLAG_INHERIT, 0);

	SetErrorMode(SEM_NOGPFAULTERRORBOX );

	STARTUPINFO StartupInfo = {0};
	StartupInfo.cb = sizeof(STARTUPINFO);
	//	StartupInfo.hStdError = h_ErrorFile;
	StartupInfo.hStdOutput = ChildOut_Write;
	StartupInfo.hStdInput = ChildIn_Read;
	StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

	write_log(JUDGE_INFO,"CreateProcess(%s)", pstJudgeSubmission->runCmd);

	/* |CREATE_NEW_CONSOLE */
	if(CreateProcess(NULL, pstJudgeSubmission->runCmd, NULL, NULL, TRUE,
					 CREATE_SUSPENDED, NULL, NULL, &StartupInfo, &pstJudgeSubmission->pProRunInfo))
	{
		write_log(JUDGE_INFO,"CreateProcess ok...");

		pstJudgeSubmission->hJob = Judge_CreateSandBox(pstJudgeSubmission);
		if(pstJudgeSubmission->hJob != NULL)
		{
			write_log(JUDGE_INFO,"Judge_CreateSandBox ok...");

			if(Judge_ProcessToSandbox(pstJudgeSubmission->hJob, pstJudgeSubmission->pProRunInfo))
			{
				write_log(JUDGE_INFO,"Judge_ProcessToSandbox ok...");

				ResumeThread(pstJudgeSubmission->pProRunInfo.hThread);
				CloseHandle(pstJudgeSubmission->pProRunInfo.hThread);

				write_log(JUDGE_INFO,"CreateFile inFileName(%s)", pstJudgeSubmission->inFileName);

				pstJudgeSubmission->hInputFile = CreateFile(pstJudgeSubmission->inFileName,
												            GENERIC_READ, FILE_SHARE_READ, NULL,
												            OPEN_ALWAYS, FILE_ATTRIBUTE_READONLY, NULL);
				if (pstJudgeSubmission->hInputFile <= 0)
				{
					write_log(JUDGE_ERROR,"CreateFile inFileName(%s) Error:%s",
											pstJudgeSubmission->inFileName, GetLastError());
				}

				BOOL flag = FALSE;
				while (true)
				{
					char buffer[BUFSIZE] = {0};
					DWORD BytesRead, BytesWritten;
					flag = ReadFile(pstJudgeSubmission->hInputFile, buffer, BUFSIZE, &BytesRead, NULL);
					if (!flag || (BytesRead == 0)) break;
					flag = WriteFile(ChildIn_Write, buffer, BytesRead, &BytesWritten, NULL);

					if (!flag){ break;}
				}

				CloseHandle(pstJudgeSubmission->hInputFile);
				pstJudgeSubmission->hInputFile=NULL;
				CloseHandle(ChildIn_Write);
				CloseHandle(ChildOut_Write);

				write_log(JUDGE_INFO,"CreateFile outFileName(%s)", pstJudgeSubmission->outFileName);

				/* 读取子进程的标准输出，并将其传递给文件输出 */
				pstJudgeSubmission->hOutputFile= CreateFile(pstJudgeSubmission->outFileName, GENERIC_WRITE, 0,
															NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (NULL == pstJudgeSubmission->hOutputFile)
				{
					write_log(JUDGE_ERROR,"CreateFile outFileName(%s) Error:%s",
										   pstJudgeSubmission->outFileName, GetLastError());
				}

				pstJudgeSubmission->startt = clock();

				DWORD limit_output =0;
				while (true)
				{
					char buffer[BUFSIZE] = {0};
					DWORD BytesRead, BytesWritten;
					flag = ReadFile(ChildOut_Read, buffer, BUFSIZE, &BytesRead, NULL);
					if (!flag || (BytesRead == 0)) break;
					flag = WriteFile(pstJudgeSubmission->hOutputFile, buffer, BytesRead, &BytesWritten, NULL);
					if (!flag) break;

					limit_output+=BytesWritten;
					if(limit_output>OutputLimit)
					{
						write_log(JUDGE_INFO,"OLE");
						pstJudgeSubmission->stSolution.verdictId = V_OLE;
						//CloseHandle(pi.hProcess);
						break;
					}
				}

				pstJudgeSubmission->endt = clock();

				CloseHandle(ChildIn_Read);ChildIn_Read=NULL;
				CloseHandle(ChildOut_Read);ChildOut_Read=NULL;
				CloseHandle(pstJudgeSubmission->hOutputFile);pstJudgeSubmission->hOutputFile=NULL;

				write_log(JUDGE_INFO,"Judge_RunProgramThread test OK..inFileName(%s)",pstJudgeSubmission->inFileName);

				return 1;
			}else{
				write_log(JUDGE_SYSTEM_ERROR,"ProcessToSandBox Error:%s",GetLastError());
			}
		}
		else{
			write_log(JUDGE_SYSTEM_ERROR,"Judge_CreateSandBox Error:%s",GetLastError());
		}
	}
	else
	{
		write_log(JUDGE_SYSTEM_ERROR,"CreateProcess       [Error:%d]\n",GetLastError());
	}

	pstJudgeSubmission->stSolution.verdictId = V_SE;

	return 0;
}

int Judge_SpecialJudge(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	/* return 0 ====Error,return 1 ====Accepted */
	int judge ;
	char spj_path[MAX_PATH];

	sprintf(spj_path,"%s%d\\spj_%d.exe %s %s",
			dataPath,pstJudgeSubmission->stProblem.problemId,
			pstJudgeSubmission->stProblem.problemId,
			pstJudgeSubmission->inFileName, pstJudgeSubmission->outFileName);

	judge = system(spj_path) ;

	if (judge == -1)
	{
		return 0;
	}

	if(judge == 1)
	{
		return 1;
	}

	return 0;
}

int Judge_RunLocalSolution(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	long caseTime=0;
	int i,case_;
	char srcPath[MAX_PATH];
	char ansPath[MAX_PATH];
	char buf[40960];

	if (NULL == pstJudgeSubmission)
	{
		write_log(JUDGE_ERROR,"Judge_RunLocalSolution ERROR, pstJudgeSubmission is NULL....");
		return 0;
	}

	pstJudgeSubmission->dwProStatusCode = 0;

	for(i=0;;++i)
	{
		case_=i;

		if(i==0)
		{
			case_=1;
		}

		sprintf(pstJudgeSubmission->inFileName, "%s%d\\data%d.in",
				dataPath, pstJudgeSubmission->stProblem.problemId, case_);
		sprintf(pstJudgeSubmission->outFileName,"%s%s%d.out",
				workPath,pstJudgeSubmission->subPath,case_);

		sprintf(srcPath, "%s", pstJudgeSubmission->outFileName);
		sprintf(ansPath, "%s%d\\data%d.out",
				dataPath, pstJudgeSubmission->stProblem.problemId, case_);

		write_log(JUDGE_INFO,"TEST(%d)\r\n inFileName:%s\r\n outFileName:%s\r\n srcPath:%s\r\n ansPath:%s\r\n",
					i, pstJudgeSubmission->inFileName, pstJudgeSubmission->outFileName, srcPath, ansPath);

		if( (_access(pstJudgeSubmission->inFileName, 0 )) == -1 )
		{
			write_log(JUDGE_INFO,"Test over..");
			break ;
		}

		pstJudgeSubmission->stSolution.testcase = case_;

		SQL_updateSolution(pstJudgeSubmission->stSolution.solutionId ,
							V_RUN,
							case_,
							pstJudgeSubmission->stSolution.time_used - pstJudgeSubmission->stSolution.time_used%10,
							pstJudgeSubmission->stSolution.memory_used);

		HANDLE hThread_run;
		hThread_run = (HANDLE)_beginthreadex(NULL, NULL, Judge_RunProgramThread, (void*)pstJudgeSubmission, 0, NULL);
		if(hThread_run == NULL)
		{
			write_log(JUDGE_ERROR,"Create thread error");
			CloseHandle(hThread_run);
		}

		write_log(JUDGE_ERROR,"Create Judge_RunProgramThread ok...");

		DWORD status_ = WaitForSingleObject(hThread_run, pstJudgeSubmission->stProblem.time_limit + 2000);   /* 放宽时限2S,返回值大于零说明超时. */
		if(status_>0)
		{
			write_log(JUDGE_INFO,"hThread_run TIME LIMIT");
			pstJudgeSubmission->stSolution.time_used = pstJudgeSubmission->stProblem.time_limit;

			if(pstJudgeSubmission->stSolution.verdictId == V_AC)
			{
				pstJudgeSubmission->stSolution.verdictId = V_TLE;
			}
		}

		if(pstJudgeSubmission->hInputFile  != NULL) CloseHandle(pstJudgeSubmission->hInputFile);
		if(pstJudgeSubmission->hOutputFile != NULL) CloseHandle(pstJudgeSubmission->hOutputFile);

		//get memory info
		PROCESS_MEMORY_COUNTERS   pmc;
		unsigned long tmp_memory=0;

		#ifdef _WIN32_
		 //del for mingw
		if(GetProcessMemoryInfo(pstJudgeSubmission->pProRunInfo.hProcess, &pmc, sizeof(pmc)))
		{
			tmp_memory=pmc.PeakWorkingSetSize/1024;
			if(tmp_memory > pstJudgeSubmission->stSolution.memory_used)
			{
				pstJudgeSubmission->stSolution.memory_used= tmp_memory;
			}
		}
        #endif

		//get process state
		GetExitCodeProcess(pstJudgeSubmission->pProRunInfo.hProcess, &(pstJudgeSubmission->dwProStatusCode));
		if(Judge_ExistException(pstJudgeSubmission->dwProStatusCode))
		{
			pstJudgeSubmission->stSolution.verdictId=V_RE;
			goto l;
		}
		else if(pstJudgeSubmission->dwProStatusCode == STILL_ACTIVE)
		{
			puts("TIME LIMIT");
			TerminateProcess(pstJudgeSubmission->pProRunInfo.hProcess, 0);
			if(pstJudgeSubmission->stSolution.verdictId == V_AC)
			{
				pstJudgeSubmission->stSolution.verdictId = V_TLE;
				pstJudgeSubmission->stSolution.time_used = pstJudgeSubmission->stProblem.time_limit;
				goto l;
			}
		}

		caseTime = pstJudgeSubmission->endt - pstJudgeSubmission->startt;
		if(caseTime < 0)
		{
			caseTime = pstJudgeSubmission->stProblem.time_limit;
		}

		TerminateJobObject(pstJudgeSubmission->hJob,0);
		CloseHandle(pstJudgeSubmission->pProRunInfo.hProcess);
		CloseHandle(pstJudgeSubmission->hJob);pstJudgeSubmission->hJob = NULL;


		pstJudgeSubmission->stSolution.time_used = (caseTime>pstJudgeSubmission->stSolution.time_used)?caseTime:pstJudgeSubmission->stSolution.time_used;

		if(pstJudgeSubmission->stSolution.time_used >= pstJudgeSubmission->stProblem.time_limit)
		{
			pstJudgeSubmission->stSolution.verdictId = V_TLE;
			pstJudgeSubmission->stSolution.time_used = pstJudgeSubmission->stProblem.time_limit;
			goto l;
		}
		if(pstJudgeSubmission->stSolution.memory_used >= pstJudgeSubmission->stProblem.memory_limit)
		{
			pstJudgeSubmission->stSolution.verdictId = V_MLE;
			pstJudgeSubmission->stSolution.memory_used = pstJudgeSubmission->stProblem.memory_limit;
			goto l;
		}


		if(pstJudgeSubmission->stSolution.verdictId != V_AC)
		{
			goto l;
		}

		//spj
		if(pstJudgeSubmission->stProblem.isSpecialJudge == 1)
		{
			int verdict_ = Judge_SpecialJudge(pstJudgeSubmission);
			if(verdict_) pstJudgeSubmission->stSolution.verdictId=V_AC;
			else pstJudgeSubmission->stSolution.verdictId=V_WA;
		}
		else
		{
			pstJudgeSubmission->stSolution.verdictId = compare(srcPath,ansPath);
		}

l:		write_log(JUDGE_INFO,"ID:%d Test%d ,%s ,%d(%d)ms %d(%d)kb ,Return code:%u",
					pstJudgeSubmission->stSolution.solutionId, i,
					VERDICT_NAME[pstJudgeSubmission->stSolution.verdictId],
					caseTime, pstJudgeSubmission->stSolution.time_used,
					tmp_memory, pstJudgeSubmission->stSolution.memory_used,
					pstJudgeSubmission->dwProStatusCode);

		/* write judge-log */

		if (i == 0)
		{
			reset_file(pstJudgeSubmission->judge_log_filename);
			if (pstJudgeSubmission->stSolution.verdictId!=V_AC)
			{
				i  = 1;
				write_buffer(pstJudgeSubmission->judge_log_filename,
							"Test: #%d, time: %d ms, memory: %d kb, exit code: %d,verdict: %s",
							i, pstJudgeSubmission->stSolution.time_used - pstJudgeSubmission->stSolution.time_used%10,
							tmp_memory, pstJudgeSubmission->dwProStatusCode,
							VERDICT_NAME[pstJudgeSubmission->stSolution.verdictId]);

				memset(buf,0,sizeof(buf));
				read_buffer(pstJudgeSubmission->inFileName, buf, JUDGE_LOG_BUF_SIZE);
				write_buffer(pstJudgeSubmission->judge_log_filename,"\nInput\n",i);
				write_buffer(pstJudgeSubmission->judge_log_filename,buf);

				memset(buf,0,sizeof(buf));
				read_buffer(pstJudgeSubmission->outFileName, buf, JUDGE_LOG_BUF_SIZE);
				write_buffer(pstJudgeSubmission->judge_log_filename,"\nOutput\n",i);
				write_buffer(pstJudgeSubmission->judge_log_filename,buf);

				memset(buf,0,sizeof(buf));
				read_buffer(ansPath, buf, JUDGE_LOG_BUF_SIZE);
				write_buffer(pstJudgeSubmission->judge_log_filename,"\nAnswer\n");
				write_buffer(pstJudgeSubmission->judge_log_filename,buf);

				write_buffer(pstJudgeSubmission->judge_log_filename,"\n------------------------------------------------------------------\n");
				break;
			}
		}
		else
		{
			write_buffer(pstJudgeSubmission->judge_log_filename,
						"Test: #%d, time: %d ms, memory: %d kb, exit code: %d,verdict: %s",
						i, caseTime - caseTime%10, tmp_memory, pstJudgeSubmission->dwProStatusCode,
						VERDICT_NAME[pstJudgeSubmission->stSolution.verdictId]);

			memset(buf,0,sizeof(buf));
			read_buffer(pstJudgeSubmission->inFileName, buf, JUDGE_LOG_BUF_SIZE);
			write_buffer(pstJudgeSubmission->judge_log_filename,"\nInput\n");
			write_buffer(pstJudgeSubmission->judge_log_filename,buf);

			memset(buf,0,sizeof(buf));
			read_buffer(pstJudgeSubmission->outFileName, buf, JUDGE_LOG_BUF_SIZE);
			write_buffer(pstJudgeSubmission->judge_log_filename,"\nOutput\n");
			write_buffer(pstJudgeSubmission->judge_log_filename,buf);

			memset(buf,0,sizeof(buf));
			read_buffer(ansPath, buf, JUDGE_LOG_BUF_SIZE);
			write_buffer(pstJudgeSubmission->judge_log_filename,"\nAnswer\n");
			write_buffer(pstJudgeSubmission->judge_log_filename,buf);

			write_buffer(pstJudgeSubmission->judge_log_filename,"\n------------------------------------------------------------------\n");
		}

		if(pstJudgeSubmission->stSolution.verdictId != V_AC)
		{
			break;
		}

		if(i==0)
		{
			pstJudgeSubmission->stSolution.time_used = 0;
			pstJudgeSubmission->stSolution.memory_used = 0;
		}
	}
	return 0;

}

int Judge_Local(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	write_log(JUDGE_INFO,"Enter Judge_Local...");

	if(0 == Judge_CompileProc(pstJudgeSubmission))
	{
		pstJudgeSubmission->stSolution.verdictId=V_CE;
		SQL_updateCompileInfo(pstJudgeSubmission);

		reset_file(pstJudgeSubmission->judge_log_filename);

		write_buffer(pstJudgeSubmission->judge_log_filename,
					"Test: #1, time: 0 ms, memory: 0 kb, exit code: 0,verdict: %s\n",
					VERDICT_NAME[pstJudgeSubmission->stSolution.verdictId]);
	}
	else
	{
		write_log(JUDGE_INFO,"Start Run...");
		Judge_RunLocalSolution(pstJudgeSubmission);
	}

	write_log(JUDGE_INFO,"End Judge_Local...");

	return OS_TRUE;
}


/*****************************************************************************
*   Prototype    : Judge_SendToJudger
*   Description  : Send judge-request to remote judger
*   Input        : int solutionId  submition ID
*                  int port        judger socket-port
*                  char *ip        judger IP
*   Output       : None
*   Return Value : int
*   Calls        :
*   Called By    :
*
*   History:
*
*       1.  Date         : 2014/7/8
*           Author       : weizengke
*           Modification : Created function
*
*****************************************************************************/
int Judge_SendToJudger(int solutionId, int port,char *ip)
{

	SOCKET sClient_hdu;

    sClient_hdu = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sClient_hdu == INVALID_SOCKET)
	{
		pdt_debug_print("Judge_SendToJudger socket error");
		return OS_ERR;
	}

	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	servAddr.sin_addr.S_un.S_addr =inet_addr(ip);

	if(connect(sClient_hdu,(sockaddr*)&servAddr,sizeof(servAddr))==SOCKET_ERROR)
	{
		pdt_debug_print("Judge_SendToJudger connect error");
		closesocket(sClient_hdu);
		return OS_ERR;
	}

	send(sClient_hdu,(const char*)&solutionId,sizeof(solutionId),0);

	closesocket(sClient_hdu);

	return OS_OK;
}

int Judge_Remote(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	int ret = OS_OK;

	do
	{
		if (0 == strcmp(pstJudgeSubmission->stProblem.szVirJudgerName,"HDU"))
		{
			pdt_debug_print("virtual-judge HDU.(vjudge_enable=%u,remote_enable=%d)",
							hdu_vjudge_enable, hdu_remote_enable);

			if (hdu_vjudge_enable == OS_NO)
			{
				pdt_debug_print("Error: hdu-judge is not enable.");
				return OS_ERR;
			}

			if (OS_YES == hdu_remote_enable)
			{
				pdt_debug_print("Send to remote judger(%s:%d).", hdu_judgerIP, hdu_sockport);

				ret = Judge_SendToJudger(pstJudgeSubmission->stSolution.solutionId, hdu_sockport, hdu_judgerIP);
				if (OS_OK == ret)
				{
					/* virdict置queue , 由远程judger继续执行 */
					pstJudgeSubmission->stSolution.verdictId = V_Q;
				}

				return ret;
			}

			/* local vjudge */
			ret = HDU_VJudge(pstJudgeSubmission);
			break;
		}

		if (0 == strcmp(pstJudgeSubmission->stProblem.szVirJudgerName,"GUET_DEPT3"))
		{
			pdt_debug_print("virtual-judge GUET_DEPT3.(vjudge_enable=%u,remote_anable=%d)",
							guet_vjudge_enable, guet_remote_enable);

			if (guet_vjudge_enable == OS_NO)
			{
				pdt_debug_print("Error: guet-judge is not enable.");
				return OS_ERR;
			}

			if (OS_YES == guet_remote_enable)
			{
				pdt_debug_print("Send to remote judger(%s:%d).", guet_judgerIP, guet_sockport);

				ret = Judge_SendToJudger(pstJudgeSubmission->stSolution.solutionId, guet_sockport, guet_judgerIP);
				if (OS_OK == ret)
				{
					/* virdict置quieue , 由远程judger继续执行 */
					pstJudgeSubmission->stSolution.verdictId = V_Q;
				}

				return ret;
			}

			/* local vjudge */
			ret = GUET_VJudge(pstJudgeSubmission);
			break;
		}

	}while(0);

	return ret;
}

unsigned _stdcall  Judge_Proc(void *pData)
{
	int ret = OS_OK;
	int isExist = OS_NO;
	int solutionId = *(int *)pData;

	JUDGE_SUBMISSION_ST stJudgeSubmission;

	memset(&stJudgeSubmission, 0, sizeof(stJudgeSubmission));

	stJudgeSubmission.stSolution.solutionId = solutionId;
	Judge_InitSubmissionData(&stJudgeSubmission);

	write_log(JUDGE_INFO, "Start judge solution %d.", stJudgeSubmission.stSolution.solutionId);

	ret = SQL_getSolutionByID(stJudgeSubmission.stSolution.solutionId, &(stJudgeSubmission.stSolution), &isExist);
	if (OS_ERR == ret || OS_NO == isExist)
	{
		pdt_debug_print("No such solution %d.", stJudgeSubmission.stSolution.solutionId);
		return OS_ERR;
	}

	Judge_InitJudgePath(&stJudgeSubmission);

	ret = SQL_getSolutionSource(&stJudgeSubmission);
	if (OS_OK != ret)
	{
		pdt_debug_print("SQL_getSolutionSource failed.(solutionId=%d)", stJudgeSubmission.stSolution.solutionId);
		write_log(JUDGE_INFO,"SQL_getSolutionSource failed.(solutionId=%d)", stJudgeSubmission.stSolution.solutionId);
		return OS_ERR;
	}

	write_log(JUDGE_INFO,"Do SQL_getSolutionSource ok. (solutionId=%d)", stJudgeSubmission.stSolution.solutionId);

	stJudgeSubmission.stProblem.problemId = stJudgeSubmission.stSolution.problemId;

	ret = SQL_getProblemInfo(&(stJudgeSubmission.stProblem));
	if (OS_OK != ret)
	{
		pdt_debug_print("SQL_getProblemInfo failed.(solutionId=%d)", stJudgeSubmission.stSolution.solutionId);
		write_log(JUDGE_INFO,"SQL_getProblemInfo failed.(solutionId=%d)", stJudgeSubmission.stSolution.solutionId);
		return OS_ERR;
	}

	if (OS_YES == stJudgeSubmission.stProblem.isVirtualJudge)
	{
		#if(JUDGE_VIRTUAL == VOS_YES)
		if (OS_YES == Judge_IsVirtualJudgeEnable())
		{
			ret = Judge_Remote(&stJudgeSubmission);
			if (OS_OK != ret)
			{
				stJudgeSubmission.stSolution.verdictId = V_SK;
				pdt_debug_print("virtua-judge is fail...");
			}
		}
		else
		{
			pdt_debug_print("Error: virtual-judge is not enable.");
			stJudgeSubmission.stSolution.verdictId = V_SK;
		}

		#else
		stJudgeSubmission.stSolution.verdictId = V_SK;
		pdt_debug_print("virtua-judge is not support.");
		#endif

		stJudgeSubmission.dwProStatusCode = 0;
		stJudgeSubmission.stSolution.testcase = 0;  /* 后续可能不填0 */

	}
	else
	{
		stJudgeSubmission.stProblem.time_limit *=stJudgeSubmission.limitIndex;
		stJudgeSubmission.stProblem.memory_limit *=stJudgeSubmission.limitIndex;

		ret = Judge_Local(&stJudgeSubmission);
	}

	write_log(JUDGE_INFO,"Do Judge finish. (solutionId=%d)", stJudgeSubmission.stSolution.solutionId);

	SQL_updateSolution(stJudgeSubmission.stSolution.solutionId,
					   stJudgeSubmission.stSolution.verdictId,
					   stJudgeSubmission.stSolution.testcase,
					   stJudgeSubmission.stSolution.time_used - stJudgeSubmission.stSolution.time_used%10,
					   stJudgeSubmission.stSolution.memory_used);

	SQL_updateProblem(stJudgeSubmission.stSolution.problemId);
	SQL_updateUser(stJudgeSubmission.stSolution.username);

	/* contest or not */
	if(stJudgeSubmission.stSolution.contestId > 0)
	{
		/* contest judge */
		time_t contest_s_time,contest_e_time;
		char num[10]={0};

		/* 获取contest problem题目标号 */
		SQL_getProblemInfo_contest(stJudgeSubmission.stSolution.contestId, stJudgeSubmission.stSolution.problemId,num);
		SQL_getContestInfo(stJudgeSubmission.stSolution.contestId,contest_s_time,contest_e_time);

		if(contest_e_time>stJudgeSubmission.stSolution.submitDate)
		{
			/* 比赛running ，修改Attend */
			SQL_updateAttend_contest(stJudgeSubmission.stSolution.contestId, stJudgeSubmission.stSolution.verdictId,
									stJudgeSubmission.stSolution.problemId, num, stJudgeSubmission.stSolution.username,
									contest_s_time,contest_e_time);
		}

		SQL_updateProblem_contest(stJudgeSubmission.stSolution.contestId, stJudgeSubmission.stSolution.problemId);
	}

	DeleteFile(stJudgeSubmission.sourcePath);
	DeleteFile(stJudgeSubmission.DebugFile);
	DeleteFile(stJudgeSubmission.exePath);


	string time_string_;
	API_TimeToString(time_string_, stJudgeSubmission.stSolution.submitDate);
	judge_outstring("\r\n -----------------------"
				"\r\n     *Judge verdict*"
				"\r\n -----------------------"
				"\r\n SolutionId   : %3d"
				"\r\n ProblemId    : %3d"
				"\r\n Pasted cases : %3d"
				"\r\n Time-used    : %3d ms"
				"\r\n Memory-used  : %3d kb"
				"\r\n Return code  : %3u"
				"\r\n Verdict      : %3s"
				"\r\n Submit Date  : %3s"
				"\r\n Username     : %3s"
				"\r\n -----------------------\r\n",
					stJudgeSubmission.stSolution.solutionId,
					stJudgeSubmission.stProblem.problemId,
					stJudgeSubmission.stSolution.testcase,
					stJudgeSubmission.stSolution.time_used - stJudgeSubmission.stSolution.time_used%10,
					stJudgeSubmission.stSolution.memory_used,
					stJudgeSubmission.dwProStatusCode,
					VERDICT_NAME[stJudgeSubmission.stSolution.verdictId],
					time_string_.c_str(), stJudgeSubmission.stSolution.username);

	return OS_OK;
}

void Judge_PushQueue(int solutionId)
{
	JUDGE_DATA_S jd = {0};

	jd.solutionId = solutionId;
	g_JudgeQueue.push(jd);
}

/* virtual-judge & local-judge 应分两个队列 */
unsigned _stdcall Judge_DispatchThread(void *pEntry)
{
	JUDGE_DATA_S jd;

	for (;;)
	{
		if(g_JudgeQueue.size()>limitJudge)
		{
			return 0;
		}

		if(!g_JudgeQueue.empty())
		{
				jd = g_JudgeQueue.front();

				/* 启动评判 */
				Judge_Proc((void*)&(jd.solutionId));
				//_beginthreadex(NULL, NULL, Judge_Proc, (void*)&(jd.solutionId), 0, NULL);

				g_JudgeQueue.pop();
		}

		Sleep(1);
	}

	return 0;
}


unsigned _stdcall Judge_ListenThread(void *pEntry)
{
	sockaddr_in remoteAddr;
	SOCKET sClient;
	int nAddrLen = sizeof(remoteAddr);
	JUDGE_DATA_S j;

	while(TRUE)
	{
		sClient = accept(g_sListen, (SOCKADDR*)&remoteAddr, &nAddrLen);
		if(sClient == INVALID_SOCKET)
		{
			write_log(JUDGE_ERROR,"Accept() Error");
			continue;
		}

		int ret=recv(sClient,(char*)&j,sizeof(j),0);
		if(ret>0)
		{
			write_log(JUDGE_INFO,"Push SolutionId:%d into Judge Queue....",j.solutionId);
			g_JudgeQueue.push(j);
		}
		Sleep(1);
	}

	write_log(JUDGE_ERROR,"ListenThread Crash");
	closesocket(sClient);

	return 0;
}

long WINAPI Judge_ExceptionFilter(EXCEPTION_POINTERS * lParam)
{
	pdt_debug_print("Judge Thread Exit...[code:%u]", GetLastError());
	write_log(JUDGE_ERROR,"Judge Thread Exit after 10 second...(GetLastError=%u)",GetLastError());
	Sleep(1);

	/* ShellExecuteA(NULL,"open",judgePath,NULL,NULL,SW_SHOWNORMAL); */

	closesocket(g_sListen);
	WSACleanup();

	return EXCEPTION_EXECUTE_HANDLER;
}

int GetProcessThreadList()
{
	HANDLE hThreadSnap;
	THREADENTRY32 th32;
	DWORD th32ProcessID = GetCurrentProcessId();

	printf(" ProcessID: %ld\n", th32ProcessID);

	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, th32ProcessID);

	if (hThreadSnap == INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	th32.dwSize = sizeof(THREADENTRY32);
	if (!Thread32First(hThreadSnap, &th32))
	{
		CloseHandle(hThreadSnap);
		return 1;
	}

	do
	{
		if (th32.th32OwnerProcessID == th32ProcessID)
		{
			printf(" ThreadID: %ld\n", th32.th32ThreadID);

		}
	}while(Thread32Next(hThreadSnap, &th32));

	CloseHandle(hThreadSnap);
	return 0;
}

int OJ_Init()
{
	SetUnhandledExceptionFilter(Judge_ExceptionFilter);
 	SetErrorMode(SEM_NOGPFAULTERRORBOX );

	if( (_access(logPath, 0 )) == -1 )
	{
		CreateDirectory(logPath,NULL);
	}

	Judge_InitConfigData();

	Judge_DebugSwitch(JUDGE_DEBUG_OFF);

	return OS_OK;
}

int OJ_InitData()
{

	if(SQL_InitMySQL()==0)
	{
		write_log(JUDGE_ERROR,"Init MySQL JUDGE_ERROR...");
		pdt_debug_print("Error: Judge can not connect to MySQL.");
	}

	if(Judge_InitSocket()==0)
	{
		write_log(JUDGE_ERROR,"Init Socket JUDGE_ERROR...");
		pdt_debug_print("Error: Judge task killed itself...[code:%u]", GetLastError());
	}

}


/*
unsigned long _beginthreadex( void *security,
								unsigned stack_size,
								unsigned ( __stdcall *start_address )( void * ),
								void *arglist,
								unsigned initflag,
								unsigned *thrdaddr );

//第1个参数：安全属性，NULL为默认安全属性
//第2个参数：指定线程堆栈的大小。如果为0，则线程堆栈大小和创建它的线程的相同。一般用0
//第3个参数：指定线程函数的地址，也就是线程调用执行的函数地址(用函数名称即可，函数名称就表示地址)
//第4个参数：传递给线程的参数的指针，可以通过传入对象的指针，在线程函数中再转化为对应类的指针
//第5个参数：线程初始状态，0:立即运行；CREATE_SUSPEND：suspended（悬挂）
//第6个参数：用于记录线程ID的地址

*/

unsigned _stdcall  OJ_TaskEntry(void *pEntry)
{
	write_log(JUDGE_INFO,"Running Judge Core...");

	(void)OJ_InitData();

	_beginthreadex(NULL, 0, Judge_DispatchThread, NULL, NULL, NULL);
	_beginthreadex(NULL, 0, Judge_ListenThread, NULL, NULL, NULL);

	//WaitForSingleObject(handle, INFINITE);
	//CloseHandle(handle);

	write_log(JUDGE_INFO,"Judge Task init ok...");

	/* 循环读取消息队列 */
	for(;;)
	{
		/* 放权 */
		Sleep(10);
	}

	closesocket(g_sListen);
	WSACleanup();

	return 0;
}

APP_INFO_S g_judgeAppInfo =
{
	NULL,
	"Judge",
	OJ_Init,
	OJ_TaskEntry
};

#if 0
int  Test1(int evtId, int cmdId, void *pData, void **ppInfo)
{
	pdt_debug_print("Do test1, evtId=%d, cmdId=%d..", evtId, cmdId);

	return OS_OK;
}

int  Test2(int evtId, int cmdId, void *pData, void **ppInfo)
{
	pdt_debug_print("Do test2, evtId=%d, cmdId=%d..", evtId, cmdId);
	return OS_OK;
}

int  Test3(int evtId, int cmdId, void *pData, void **ppInfo)
{
	pdt_debug_print("Do test3, evtId=%d, cmdId=%d..", evtId, cmdId);
	return OS_OK;
}
int  Test4(int evtId, int cmdId, void *pData, void **ppInfo)
{
	pdt_debug_print("Do test4, evtId=%d, cmdId=%d..", evtId, cmdId);
	return OS_OK;
}
#endif

void Judge_RegAppInfo()
{
	RegistAppInfo(&g_judgeAppInfo);

#if 0
	EVENT_RegistFunc("Test1", EVENT_NTF_JUDGE, EVENT_NTF_CMD_NONE, 100, Test1);
	EVENT_RegistFunc("Test2", EVENT_NTF_JUDGE, EVENT_NTF_CMD_NONE, 102, Test2);
	EVENT_RegistFunc("Test3", EVENT_NTF_CMD, EVENT_NTF_CMD_NONE, 102, Test3);
	EVENT_RegistFunc("Test4", EVENT_NTF_JUDGE, EVENT_NTF_CMD_NONE, 102, Test4);
	EVENT_RegistFunc("Test5", EVENT_NTF_JUDGE, EVENT_NTF_CMD_NONE, 102, Test2);
	EVENT_RegistFunc("Test6", EVENT_NTF_JUDGE, EVENT_NTF_CMD_NONE, 106, Test2);
	EVENT_RegistFunc("Test7", EVENT_NTF_JUDGE, 1, 99, Test2);
	EVENT_RegistFunc("Test8", EVENT_NTF_JUDGE, EVENT_NTF_CMD_NONE, 97, Test2);
	EVENT_RegistFunc("Test9", EVENT_NTF_JUDGE, EVENT_NTF_CMD_NONE, 98, Test2);
	EVENT_RegistFunc("Test10", EVENT_NTF_JUDGE, 2, 120, Test2);
	EVENT_RegistFunc("Test11", EVENT_NTF_JUDGE, EVENT_NTF_CMD_NONE, 98, Test2);

	EVENT_Ntf_Show();

	EVENT_Ntf_Notify(EVENT_NTF_JUDGE, 1, (void *)NULL, (void **)NULL);
#endif

}

#if 0
int main(int argc, char **argv)
{
	OJ_TaskEntry();
	return 0;
}
#endif



