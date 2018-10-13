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


	未来分布式部署思路
	1) client上线后，server维护client信息(ip,socket,判题能力)
	2) server打包发送data文件给client(client上线是触发、data目录文件变化时触发)
	3) client下线后通知server
	4) server定时ping client 心跳维持，若长时间无回应则删除client

*/

#include "product/judge/include/judge_inc.h"


#if (OS_YES == OSP_MODULE_JUDGE)

using namespace std;


char INI_filename[] = STARTUP_CFG;

int isDeleteTemp=0;
int isRestrictedFunction=0;
DWORD OutputLimit=6553500;
char workPath[MAX_PATH];
char judgeLogPath[MAX_PATH];
int JUDGE_LOG_BUF_SIZE = 200;
char dataPath[MAX_PATH];

#define BUFFER 1024

int g_judge_mode = JUDGE_MODE_ACM;
int g_judge_timer_enable = OS_NO;
int g_judge_auto_detect_interval = 10;
time_t g_lastjudgetime = 0;
int g_judge_ignore_extra_space_enable = OS_NO;

queue <JUDGE_DATA_S> g_JudgeQueue; /* 全局队列 */

ULONG Judge_DebugSwitch(ULONG st)
{
	g_oj_debug_switch = st;

	return OS_TRUE;
}

#if (OS_YES == OSP_MODULE_JUDGE_VJUDGE)
int Judge_IsVirtualJudgeEnable()
{
	if (g_vjudge_enable == OS_YES)
	{
		return OS_YES;
	}

	return OS_NO;
}
#endif


void Judge_InitJudgePath(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	if (NULL == pstJudgeSubmission)
	{
		write_log(JUDGE_ERROR,"Judge_InitJudgePath ERROR, pstJudgeSubmission is NULL....");
		return ;
	}

	char keyname[100]={0};
	sprintf(keyname,"Language%d", pstJudgeSubmission->stSolution.languageId);

	util_ini_get_string("Language",keyname,"",pstJudgeSubmission->languageName,100,INI_filename);

	util_ini_get_string("LanguageExt",pstJudgeSubmission->languageName,"",pstJudgeSubmission->languageExt,10,INI_filename);

	util_ini_get_string("LanguageExe",pstJudgeSubmission->languageName,"",pstJudgeSubmission->languageExe,10,INI_filename);

	util_ini_get_string("CompileCmd",pstJudgeSubmission->languageName,"",pstJudgeSubmission->compileCmd,1024,INI_filename);

	util_ini_get_string("RunCmd",pstJudgeSubmission->languageName,"",pstJudgeSubmission->runCmd,1024,INI_filename);

	util_ini_get_string("SourcePath",pstJudgeSubmission->languageName,"",pstJudgeSubmission->sourcePath,1024,INI_filename);

	util_ini_get_string("ExePath",pstJudgeSubmission->languageName,"",pstJudgeSubmission->exePath,1024,INI_filename);

	pstJudgeSubmission->isTranscoding=util_ini_get_int("Transcoding",pstJudgeSubmission->languageName,0,INI_filename);

	pstJudgeSubmission->limitIndex=util_ini_get_int("TimeLimit",pstJudgeSubmission->languageName,1,INI_filename);

	pstJudgeSubmission->nProcessLimit=util_ini_get_int("ProcessLimit",pstJudgeSubmission->languageName,1,INI_filename);


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

	if( (file_access(judgeLogPath, 0 )) == -1 )
	{
		create_directory(judgeLogPath);
	}

	sprintf(pstJudgeSubmission->judge_log_filename,"%sjudge-log-%d.log",judgeLogPath,pstJudgeSubmission->stSolution.solutionId);

    write_log(JUDGE_INFO, "Judge_InitJudgePath %d ok.", pstJudgeSubmission->stSolution.solutionId);
        
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

	pstJudgeSubmission->hInputFile = INVALID_HANDLE_VALUE;
	pstJudgeSubmission->hOutputFile = INVALID_HANDLE_VALUE;

	if( (file_access(workPath, 0 )) == -1 )
	{
		create_directory(workPath);
	}
	
	time_t timep;
	time(&timep);
	srand((int)time(0)*3);
	pstJudgeSubmission->ulSeed = timep + rand();

	sprintf(pstJudgeSubmission->subPath, "%d_%u\\", pstJudgeSubmission->stSolution.solutionId, pstJudgeSubmission->ulSeed);

	char fullPath[1024] = {0};
	sprintf(fullPath, "%s%s", workPath, pstJudgeSubmission->subPath);
	while( (file_access(fullPath, 0 )) != -1 )
	{
		write_log(JUDGE_INFO,"Gernerate another Seed...(%u)", pstJudgeSubmission->ulSeed);
		Sleep(10);
		pstJudgeSubmission->ulSeed = timep + rand();

		sprintf(pstJudgeSubmission->subPath, "%d_%u\\", pstJudgeSubmission->stSolution.solutionId, pstJudgeSubmission->ulSeed);
		sprintf(fullPath, "%s%s", workPath, pstJudgeSubmission->subPath);
	}

	create_directory(fullPath);

}

int Judge_DisableAllPriv(HANDLE ProcessHandle)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;
	//打开进程令牌环
	if(!OpenProcessToken(ProcessHandle,
		TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
		&hToken) )
	{
		printf("OpenProcessToken error\n");
		return 1;
	}
	//获得进程本地唯一ID
	if(!LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&luid))
	{
		printf("LookupPrivilege error!\n");
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes =SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = luid;
	//调整进程权限
	if(!AdjustTokenPrivileges(hToken,TRUE,&tp,sizeof(TOKEN_PRIVILEGES),NULL,NULL) )
	{
		printf("AdjustTokenPrivileges error!\n");
		return 1;
	}
	return 0;
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
	return true;
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

		Judge_DisableAllPriv(p.hProcess);
		
		return true;
	}
	else
	{
		write_log(JUDGE_SYSTEM_ERROR,"AssignProcessToJobObject Error:%d",GetLastError());
	}

	return false;
}

int Judge_CompileThread(void *pData)
{
	JUDGE_SUBMISSION_ST *pstJudgeSubmission = (JUDGE_SUBMISSION_ST *)pData;

	if (NULL == pstJudgeSubmission)
	{
		write_log(JUDGE_ERROR,"Judge_CompileThread ERROR, pstJudgeSubmission is NULL....");
		return OS_ERR;
	}

	write_log(JUDGE_INFO,"Enter Judge_CompileThread...");

	SQL_updateSolution(pstJudgeSubmission->stSolution.solutionId, V_C, 0, 0, 0, 0);

	system(pstJudgeSubmission->compileCmd);

	write_log(JUDGE_INFO,"End Judge_CompileThread...");

	return OS_OK;
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

	hThread_com = (HANDLE)thread_create(Judge_CompileThread, (void*)pstJudgeSubmission);
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

	if( (file_access(pstJudgeSubmission->exePath, 0 )) != -1 )
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


/* 全局APIKooh标记 */
ULONG g_ulNeedApiHookFlag = OS_TRUE;
char szApiHookDllPath[MAX_PATH] = "hook.dll"; 

int EnableDebugPriv(const char * name)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;
	//打开进程令牌环
	if(!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
		&hToken) )
	{
		write_log(JUDGE_ERROR,"OpenProcessToken error\n");
		return 1;
	}
	//获得进程本地唯一ID
	if(!LookupPrivilegeValue(NULL,name,&luid))
	{
		write_log(JUDGE_ERROR,"LookupPrivilege error!\n");
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes =SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = luid;
	//调整进程权限
	if(!AdjustTokenPrivileges(hToken,0,&tp,sizeof(TOKEN_PRIVILEGES),NULL,NULL) )
	{
		write_log(JUDGE_ERROR,"AdjustTokenPrivileges error!\n");
		return 1;
	}
	return 0;
}

BOOL InjectAPIHookDll(DWORD dwProcessID, char* dllPath)
{
    FARPROC FuncAddr = NULL;  

	if( (file_access(dllPath, 0 )) == -1 )
	{
		write_log(JUDGE_ERROR,"InjectAPIHookDll failed, the file %s is not exist.", dllPath);
		return FALSE;
	}

	if(EnableDebugPriv(SE_DEBUG_NAME))
	{
		write_log(JUDGE_ERROR,"add privilege error %u", GetLastError());
		return FALSE;
	}
	
    HMODULE hdll = LoadLibrary(TEXT("Kernel32.dll"));
    if (hdll != NULL)
	{  
        FuncAddr = GetProcAddress(hdll, "LoadLibraryA");
        if (FuncAddr == NULL)
		{
			write_log(JUDGE_ERROR,"GetProcAddress error %u", GetLastError());
			return FALSE;  
        }
    }  
  
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
    if (hProcess == NULL)
	{
		write_log(JUDGE_ERROR,"OpenProcess error %u", GetLastError());
		return FALSE;  
    }
	
    DWORD dwSize = strlen(dllPath) + 1;  
    LPVOID RemoteBuf = VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
    if (NULL == RemoteBuf)
    {
		write_log(JUDGE_ERROR,"VirtualAllocEx error %u", GetLastError());
		return FALSE;  
	}
	
    DWORD dwRealSize;  
    if (WriteProcessMemory(hProcess, RemoteBuf, dllPath, dwSize, &dwRealSize))
    {  
        HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)FuncAddr, RemoteBuf, 0, NULL);
        if (hRemoteThread == NULL)  
        {  
            VirtualFreeEx(hProcess, RemoteBuf, dwSize, MEM_COMMIT);  
            CloseHandle(hProcess); 
			write_log(JUDGE_ERROR,"CreateRemoteThread error %u", GetLastError());
            return FALSE;  
        }  
		
        /* 释放资源 */
        WaitForSingleObject(hRemoteThread, INFINITE);  
        CloseHandle(hRemoteThread);  
        VirtualFreeEx(hProcess, RemoteBuf, dwSize, MEM_COMMIT);  
        CloseHandle(hProcess);  
        return TRUE;  
    }  
    else  
    {  
		write_log(JUDGE_ERROR,"WriteProcessMemory error %u", GetLastError());
        VirtualFreeEx(hProcess, RemoteBuf, dwSize, MEM_COMMIT);  
        CloseHandle(hProcess);  
        return FALSE;  
    }  
}  

int Judge_RunProgramThread(void *pData)
{
	JUDGE_SUBMISSION_ST *pstJudgeSubmission = (JUDGE_SUBMISSION_ST *)pData;
	
	if (NULL == pstJudgeSubmission)
	{
		write_log(JUDGE_ERROR,"Judge_RunProgramThread ERROR, pstJudgeSubmission is NULL....");
		return OS_ERR;
	}

	SetErrorMode(SEM_NOGPFAULTERRORBOX );

	SECURITY_ATTRIBUTES sa = {0};
	sa.nLength = sizeof(sa);  
    sa.lpSecurityDescriptor = NULL;  
    sa.bInheritHandle = TRUE; 
	/* 创建输入文件句柄 */
	pstJudgeSubmission->hInputFile = CreateFile(pstJudgeSubmission->inFileName,  
					 GENERIC_READ,	
					 0,  
					 &sa,
					 OPEN_EXISTING,
					 FILE_ATTRIBUTE_NORMAL,
					 NULL);  
	if(pstJudgeSubmission->hInputFile == INVALID_HANDLE_VALUE)  
	{	
		write_log(JUDGE_ERROR,"Judge_RunProgramThread ERROR, CreateFile hInputFile failed...");
		return OS_ERR;
	}	

	/* 创建输出文件句柄 */
	pstJudgeSubmission->hOutputFile  = CreateFile(pstJudgeSubmission->outFileName,	
					 GENERIC_WRITE,  
					 0,  
					 &sa,
					 CREATE_ALWAYS,
					 FILE_ATTRIBUTE_NORMAL,
					 NULL);  
	if(pstJudgeSubmission->hOutputFile == INVALID_HANDLE_VALUE)	
	{	
		write_log(JUDGE_ERROR,"Judge_RunProgramThread ERROR, CreateFile hOutputFile failed...");

		CloseHandle(pstJudgeSubmission->hInputFile); 
		pstJudgeSubmission->hInputFile = INVALID_HANDLE_VALUE;
		return OS_ERR;  
	}	

	
	STARTUPINFO si = {sizeof(STARTUPINFO)};  
	si.hStdInput = pstJudgeSubmission->hInputFile;  
	si.hStdOutput= pstJudgeSubmission->hOutputFile;  
	si.hStdError = pstJudgeSubmission->hOutputFile; 
	si.dwFlags = STARTF_USESTDHANDLES;  
	/* 创建用户程序进程，先挂起 */
	if(CreateProcess(NULL, 
					pstJudgeSubmission->runCmd, 
					NULL,
					NULL,
					TRUE,
					CREATE_SUSPENDED,
					NULL,
					NULL,
					&si,
					&pstJudgeSubmission->pProRunInfo))  
	{	
		
		Judge_DisableAllPriv(pstJudgeSubmission->pProRunInfo.hProcess);

		write_log(JUDGE_INFO,"CreateProcess(%s) ok...", pstJudgeSubmission->runCmd);

		/* 安全防护api hook */
		/* Begin add for apihook 2013/05/18 */
		if (OS_TRUE == g_ulNeedApiHookFlag)		
		{		
			if (FALSE == InjectAPIHookDll(pstJudgeSubmission->pProRunInfo.dwProcessId, szApiHookDllPath))		
			{		
				write_log(JUDGE_ERROR,"InjectAPIHookDll Error, dwProcessId[%u]", pstJudgeSubmission->pProRunInfo.dwProcessId);			
			}		
			else
			{
				write_log(JUDGE_INFO,"InjectAPIHookDll OK, dwProcessId[%u]", pstJudgeSubmission->pProRunInfo.dwProcessId);
			}
		}		
		/* End add for apihook 2013/05/18 */

		/* 唤醒进程 */
		ResumeThread(pstJudgeSubmission->pProRunInfo.hThread);

		/* 计时等待程序执行结束 */
		pstJudgeSubmission->startt = clock();
		WaitForSingleObject(pstJudgeSubmission->pProRunInfo.hProcess, INFINITE);  
		pstJudgeSubmission->endt = clock();

		/* 释放句柄 */
		//CloseHandle(pstJudgeSubmission->pProRunInfo.hProcess);  pstJudgeSubmission->pProRunInfo.hProcess = NULL;
		CloseHandle(pstJudgeSubmission->pProRunInfo.hThread);
		pstJudgeSubmission->pProRunInfo.hThread = NULL;		
		
		CloseHandle(pstJudgeSubmission->hInputFile);  
		pstJudgeSubmission->hInputFile = INVALID_HANDLE_VALUE;
		
		CloseHandle(pstJudgeSubmission->hOutputFile);  
		pstJudgeSubmission->hOutputFile = INVALID_HANDLE_VALUE;		 
				
		return OS_OK;
	}	
	else
	{
		write_log(JUDGE_ERROR,"CreateProcess(%s) pok. %u", pstJudgeSubmission->runCmd, GetTickCount());

		pstJudgeSubmission->stSolution.verdictId = V_SE;
		
		CloseHandle(pstJudgeSubmission->hInputFile);  
		pstJudgeSubmission->hInputFile = INVALID_HANDLE_VALUE;
		
		CloseHandle(pstJudgeSubmission->hOutputFile);  
		pstJudgeSubmission->hOutputFile = INVALID_HANDLE_VALUE;				
	}
 	
	return OS_ERR;
}

int Judge_SpecialJudge(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	/* return 0 ====Error,return 1 ====Accepted */
	int judge ;
	char spj_path[MAX_PATH];

	sprintf(spj_path,"%s\\%d\\spj_%d.exe %s %s",
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

int Judge_GetTestcasesByProblemID(int problemId, char szTestcases[JUDGE_MAX_CASE][_MAX_FNAME])
{
	char szPath[_MAX_PATH] = {0};
	int i = 0;
	long Handle;
	struct _finddata_t FileInfo;
	int iCaseNum = 0;

	sprintf(szPath, "%s\\%d\\*.in", dataPath, problemId);

	if(-1L != (Handle=_findfirst(szPath, &FileInfo)))
	{
		char path_buffer[_MAX_PATH];  
		char drive[_MAX_DRIVE];  
		char dir[_MAX_DIR];  
		char fname[_MAX_FNAME];  
		char ext[_MAX_EXT]; 
		
		_splitpath(FileInfo.name, drive, dir, fname, ext); 
		sprintf(szTestcases[iCaseNum++], "%s", fname);
		
		while( _findnext(Handle,&FileInfo)==0)
		{
			_splitpath(FileInfo.name, drive, dir, fname, ext); 
			sprintf(szTestcases[iCaseNum++], "%s", fname);
		}
		
		_findclose(Handle);

		qsort(szTestcases, iCaseNum, sizeof(char)*_MAX_FNAME, string_cmp);
	}

#if 0
	for(i=0; i < iCaseNum; ++i)
	{
		printf("\r\n %s", szTestcases[i]);
	}	
#endif

	return iCaseNum;
}

int Judge_RunLocalSolution(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	long caseTime=0;
	int i;
	int icaseId = 0;
	char srcPath[MAX_PATH];
	char ansPath[MAX_PATH];
	char buf[40960];
	cJSON *json = cJSON_CreateObject();
	cJSON *array = cJSON_CreateArray();
	cJSON *testcase = NULL;
	char *pjsonBuf = NULL;

	if (NULL == pstJudgeSubmission
		|| NULL == json
		|| NULL == array)
	{
		write_log(JUDGE_ERROR,"Judge_RunLocalSolution ERROR, pstJudgeSubmission is NULL....");
		return 0;
	}

	cJSON_AddNumberToObject(json,"solutionId", pstJudgeSubmission->stSolution.solutionId);
	cJSON_AddNumberToObject(json,"problemId",pstJudgeSubmission->stSolution.problemId);
	cJSON_AddStringToObject(json,"username",pstJudgeSubmission->stSolution.username);
	cJSON_AddStringToObject(json,"language",pstJudgeSubmission->languageName);
	
	pstJudgeSubmission->dwProStatusCode = 0;
	pstJudgeSubmission->stSolution.time_used = 0;
	pstJudgeSubmission->stSolution.memory_used = 0;

	(VOID)util_freset(pstJudgeSubmission->judge_log_filename);

	int caseNum = 0;
	char szTestcases[JUDGE_MAX_CASE][_MAX_FNAME] = {0};
	caseNum = Judge_GetTestcasesByProblemID(pstJudgeSubmission->stProblem.problemId, szTestcases);
	
	for(i=0; i < caseNum; ++i)
	{
		sprintf(pstJudgeSubmission->inFileName, "%s\\%d\\%s.in",
				dataPath, pstJudgeSubmission->stProblem.problemId, szTestcases[i]);
		sprintf(pstJudgeSubmission->outFileName,"%s%s%s.out",
				workPath,pstJudgeSubmission->subPath, szTestcases[i]);
		sprintf(pstJudgeSubmission->stdOutFileName,"%s\\%d\\%s.out",
				dataPath, pstJudgeSubmission->stProblem.problemId, szTestcases[i]);

		sprintf(srcPath, "%s", pstJudgeSubmission->outFileName);
 		sprintf(ansPath, "%s", pstJudgeSubmission->stdOutFileName);

		write_log(JUDGE_INFO,"TEST(%d) verdictId:%s\r\n inFileName:%s\r\n outFileName:%s\r\n srcPath:%s\r\n ansPath:%s\r\n",
					i + 1, VERDICT_NAME[pstJudgeSubmission->stSolution.verdictId], pstJudgeSubmission->inFileName, pstJudgeSubmission->outFileName, srcPath, ansPath);

		if( (file_access(pstJudgeSubmission->inFileName, 0 )) == -1 )
		{
			write_log(JUDGE_INFO,"Test over..");
			break ;
		}

		icaseId = i + 1;
		pstJudgeSubmission->stSolution.testcase = icaseId;

		SQL_updateSolution(pstJudgeSubmission->stSolution.solutionId ,
							V_RUN,
							icaseId,
							pstJudgeSubmission->stSolution.failcase,
							pstJudgeSubmission->stSolution.time_used - pstJudgeSubmission->stSolution.time_used%10,
							pstJudgeSubmission->stSolution.memory_used);

		HANDLE hThread_run;
		hThread_run = (HANDLE)thread_create(Judge_RunProgramThread, (void*)pstJudgeSubmission);
		if(hThread_run == NULL)
		{
			write_log(JUDGE_ERROR,"Create thread error");
		}

		write_log(JUDGE_INFO,"Create Judge_RunProgramThread ok...");

		DWORD status_ = WaitForSingleObject(hThread_run, pstJudgeSubmission->stProblem.time_limit + 1000);   /* 放宽时限1000mS,返回值大于零说明超时. */
		if(status_>0)
		{
			write_log(JUDGE_INFO,"case %u WaitForSingleObject TIME LIMIT, verdictId:%s.", i, VERDICT_NAME[pstJudgeSubmission->stSolution.verdictId]);

			caseTime = pstJudgeSubmission->stProblem.time_limit;
			pstJudgeSubmission->stSolution.time_used = pstJudgeSubmission->stProblem.time_limit;

			if(pstJudgeSubmission->stSolution.verdictId == V_AC)
			{
				pstJudgeSubmission->stSolution.verdictId = V_TLE;
			}
		}

		if(pstJudgeSubmission->hInputFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(pstJudgeSubmission->hInputFile);
			pstJudgeSubmission->hInputFile = INVALID_HANDLE_VALUE;
		}
		
		if(pstJudgeSubmission->hOutputFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(pstJudgeSubmission->hOutputFile);
			pstJudgeSubmission->hOutputFile = INVALID_HANDLE_VALUE;
		}
		
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
			write_log(JUDGE_INFO,"case %u STILL_ACTIVE, to TIME LIMIT, verdictId:%s. ", icaseId, VERDICT_NAME[pstJudgeSubmission->stSolution.verdictId]);

			TerminateProcess(pstJudgeSubmission->pProRunInfo.hProcess, 0);
			
			if(pstJudgeSubmission->stSolution.verdictId == V_AC)
			{
				pstJudgeSubmission->stSolution.verdictId = V_TLE;
				caseTime = pstJudgeSubmission->stProblem.time_limit;
				pstJudgeSubmission->stSolution.time_used = pstJudgeSubmission->stProblem.time_limit;
				goto l;
			}
		}

		if (pstJudgeSubmission->stSolution.verdictId != V_TLE)
		{
			caseTime = pstJudgeSubmission->endt - pstJudgeSubmission->startt;
			write_log(JUDGE_INFO,"case %u , verdictId:%s, time(%u = %u - %u).",
							icaseId, VERDICT_NAME[pstJudgeSubmission->stSolution.verdictId], caseTime, pstJudgeSubmission->endt, pstJudgeSubmission->startt);

			if(caseTime < 0)
			{

				caseTime = pstJudgeSubmission->stProblem.time_limit;
			}
		}

		CloseHandle(pstJudgeSubmission->pProRunInfo.hProcess);
		pstJudgeSubmission->pProRunInfo.hProcess = NULL;
		
		pstJudgeSubmission->stSolution.time_used = (caseTime>pstJudgeSubmission->stSolution.time_used)?caseTime:pstJudgeSubmission->stSolution.time_used;

		if(caseTime >= pstJudgeSubmission->stProblem.time_limit)
		{
			pstJudgeSubmission->stSolution.verdictId = V_TLE;
			pstJudgeSubmission->stSolution.time_used = pstJudgeSubmission->stProblem.time_limit;
			goto l;
		}
		
		if(tmp_memory >= pstJudgeSubmission->stProblem.memory_limit)
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
			int verdict_ = compare(srcPath,ansPath);
			
			if (OS_NO == g_judge_ignore_extra_space_enable)
			{
				pstJudgeSubmission->stSolution.verdictId = verdict_;
			}
			else 
			{
				/* 使能忽略多余空格，切ac */
				if (V_PE == verdict_)
				{
					pstJudgeSubmission->stSolution.verdictId = V_AC;
				}
			}
			
		}

l:		write_log(JUDGE_INFO,"ID:%d Test%d ,%s ,%d(%d)ms %d(%d)kb ,Return code:%u",
					pstJudgeSubmission->stSolution.solutionId, icaseId,
					VERDICT_NAME[pstJudgeSubmission->stSolution.verdictId],
					caseTime, pstJudgeSubmission->stSolution.time_used,
					tmp_memory, pstJudgeSubmission->stSolution.memory_used,
					pstJudgeSubmission->dwProStatusCode);
       
		/* save jsonResult */	
		if (JUDGE_MODE_OI == g_judge_mode)
		{
			testcase = cJSON_CreateObject();
			if (NULL != testcase)
			{
				cJSON_AddItemToArray(array, testcase);
				cJSON_AddNumberToObject(testcase, "case", icaseId);
				cJSON_AddStringToObject(testcase, "verdict", VERDICT_NAME[pstJudgeSubmission->stSolution.verdictId]);
				cJSON_AddNumberToObject(testcase, "timeused", caseTime - caseTime%10);
				cJSON_AddNumberToObject(testcase, "memused", tmp_memory);
			}
		}

		/* write judge-log */
		(void)util_fwrite(pstJudgeSubmission->judge_log_filename,
					"Test: #%d, time: %d ms, memory: %d kb, exit code: %d,verdict: %s",
					icaseId, caseTime - caseTime%10, tmp_memory, pstJudgeSubmission->dwProStatusCode,
					VERDICT_NAME[pstJudgeSubmission->stSolution.verdictId]);

		memset(buf,0,sizeof(buf));
		(void)util_fread(pstJudgeSubmission->inFileName, buf, JUDGE_LOG_BUF_SIZE);
		(void)util_fwrite(pstJudgeSubmission->judge_log_filename,"\nInput\n");
		(void)util_fwrite(pstJudgeSubmission->judge_log_filename,buf);

		memset(buf,0,sizeof(buf));
		(void)util_fread(pstJudgeSubmission->outFileName, buf, JUDGE_LOG_BUF_SIZE);
		(void)util_fwrite(pstJudgeSubmission->judge_log_filename,"\nOutput\n");
		(void)util_fwrite(pstJudgeSubmission->judge_log_filename,buf);

		memset(buf,0,sizeof(buf));
		(void)util_fread(ansPath, buf, JUDGE_LOG_BUF_SIZE);
		(void)util_fwrite(pstJudgeSubmission->judge_log_filename,"\nAnswer\n");
		(void)util_fwrite(pstJudgeSubmission->judge_log_filename,buf);

		(void)util_fwrite(pstJudgeSubmission->judge_log_filename,"\n------------------------------------------------------------------\n");

		if(pstJudgeSubmission->stSolution.verdictId != V_AC)
		{			
			pstJudgeSubmission->stSolution.failcase++;
				
			if (JUDGE_MODE_OI == g_judge_mode)
			{
				#if 0
				if (JUDGE_ISJUDGE_STOP(pstJudgeSubmission->stSolution.verdictId))
				{
					break;
				}
				#endif
				
				pstJudgeSubmission->stSolution.verdictId = V_AC;
				continue;
			}
			else
			{
				break;
			}
		}
	}

	
	cJSON_AddNumberToObject(json,"testcases", pstJudgeSubmission->stSolution.testcase);
	cJSON_AddItemToObject(json,"cases", array);
		
	if (JUDGE_MODE_OI == g_judge_mode)
	{
		/* 存在错误用例转为WA */
		if (pstJudgeSubmission->stSolution.failcase > 0)
		{
			pstJudgeSubmission->stSolution.verdictId = V_WA;
		}

		pjsonBuf = cJSON_Print(json);

		write_log(JUDGE_ERROR, "jsonBuf-len=%u.", strlen(pjsonBuf));
		
		if (strlen(pjsonBuf) < JSONBUFSIZE)
		{
			memcpy(pstJudgeSubmission->pszjudgeResult_Json, pjsonBuf, strlen(pjsonBuf));
		}
		else
		{
			write_log(JUDGE_ERROR, "judgeResult_Json is not enough memory. (JSONBUFSIZE=%u, strlen=%u)", JSONBUFSIZE, strlen(pjsonBuf));
		}

		SQL_updateSolutionJsonResult(pstJudgeSubmission->stSolution.solutionId, pjsonBuf);
		
		free(pjsonBuf);
		
	}

	/* 迭代删除json数据 */
	cJSON_Delete(json);	
	
	return 0;

}

int Judge_Local(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	write_log(JUDGE_INFO,"Enter Judge_Local...");

	if(0 == Judge_CompileProc(pstJudgeSubmission))
	{
		pstJudgeSubmission->stSolution.verdictId=V_CE;
		SQL_updateCompileInfo(pstJudgeSubmission);

		(VOID)util_freset(pstJudgeSubmission->judge_log_filename);

		(void)util_fwrite(pstJudgeSubmission->judge_log_filename,
					"Test: #1, time: 0 ms, memory: 0 kb, exit code: 0,verdict: %s\n",
					VERDICT_NAME[pstJudgeSubmission->stSolution.verdictId]);
	}
	else
	{
		write_log(JUDGE_INFO,"Start Run solution %u...", pstJudgeSubmission->stSolution.solutionId);

        {
			/* 一分钟内没有提交，重复一遍，规避长时间没有judge导致的运行时间不准问题 */
            time_t s_t = 0;            
            time(&s_t);
            
            if (util_getdiftime(s_t,g_lastjudgetime) > 60)
            {
                Judge_PushQueue(pstJudgeSubmission->stSolution.solutionId);
            }
            g_lastjudgetime = s_t;
        }
        
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

	socket_t sClient_hdu;
	char buff[128] = {0};
	
    sClient_hdu = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sClient_hdu == INVALID_SOCKET)
	{
		Judge_Debug(DEBUG_TYPE_ERROR, "Judge_SendToJudger socket error");
		return OS_ERR;
	}

	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	servAddr.sin_addr.s_addr =inet_addr(ip);

	if(connect(sClient_hdu,(sockaddr*)&servAddr,sizeof(servAddr))==SOCKET_ERROR)
	{
		Judge_Debug(DEBUG_TYPE_ERROR, "Judge_SendToJudger connect error");
		closesocket(sClient_hdu);
		return OS_ERR;
	}

	sprintf(buff, "judge solution %u", solutionId);
	
	send(sClient_hdu,(const char*)buff, sizeof(buff),0);

	closesocket(sClient_hdu);

	return OS_OK;
}

#if (OS_YES == OSP_MODULE_JUDGE_VJUDGE)
int Judge_Remote(JUDGE_SUBMISSION_ST *pstJudgeSubmission)
{
	int ret = OS_OK;

	write_log(JUDGE_INFO, "Start vjudge remote. (solutionId=%u, szVirJudgerName=%s)",
				pstJudgeSubmission->stSolution.solutionId, pstJudgeSubmission->stProblem.szVirJudgerName);
	
	do
	{
		if (0 == strcmp(pstJudgeSubmission->stProblem.szVirJudgerName,"HDU"))
		{
			Judge_Debug(DEBUG_TYPE_FUNC, "virtual-judge HDU.(domain=%s, vjudge_enable=%u,remote_enable=%d)",
							hdu_domain, hdu_vjudge_enable, hdu_remote_enable);

			if (hdu_vjudge_enable == OS_NO)
			{
				Judge_Debug(DEBUG_TYPE_ERROR, "Error: hdu-judge is not enable.");
				return OS_ERR;
			}

			if (OS_YES == hdu_remote_enable)
			{
				Judge_Debug(DEBUG_TYPE_FUNC, "Send to remote judger(%s:%d).", hdu_judgerIP, hdu_sockport);

				ret = Judge_SendToJudger(pstJudgeSubmission->stSolution.solutionId, hdu_sockport, hdu_judgerIP);
				if (OS_OK == ret)
				{
					/* virdict置queue , 由远程judger继续执行 */
					pstJudgeSubmission->stSolution.verdictId = V_Q;
				}

				return ret;
			}

			/* local vjudge */
			#if (OS_YES == OSP_MODULE_JUDGE_VJUDGE)
			ret = HDU_VJudge(pstJudgeSubmission);
			#endif
			
			break;
		}

		if (0 == strcmp(pstJudgeSubmission->stProblem.szVirJudgerName,"GUET_DEPT3"))
		{
			Judge_Debug(DEBUG_TYPE_FUNC, "virtual-judge GUET_DEPT3.(vjudge_enable=%u,remote_anable=%d)",
							guet_vjudge_enable, guet_remote_enable);

			if (guet_vjudge_enable == OS_NO)
			{
				Judge_Debug(DEBUG_TYPE_ERROR, "Error: guet-judge is not enable.");
				return OS_ERR;
			}

			if (OS_YES == guet_remote_enable)
			{
				Judge_Debug(DEBUG_TYPE_FUNC, "Send to remote judger(%s:%d).", guet_judgerIP, guet_sockport);

				ret = Judge_SendToJudger(pstJudgeSubmission->stSolution.solutionId, guet_sockport, guet_judgerIP);
				if (OS_OK == ret)
				{
					/* virdict置quieue , 由远程judger继续执行 */
					pstJudgeSubmission->stSolution.verdictId = V_Q;
				}

				return ret;
			}

			/* local vjudge */
			#if(JUDGE_VIRTUAL == VOS_YES)
			ret = GUET_VJudge(pstJudgeSubmission);
			#endif
			
			break;
		}

		Judge_Debug(DEBUG_TYPE_ERROR, "virtua-judge is not support (%s).",
						pstJudgeSubmission->stProblem.szVirJudgerName);
		
		write_log(JUDGE_INFO, "virtua-judge is not support (%s).",
						pstJudgeSubmission->stProblem.szVirJudgerName);
		
		return OS_ERR;
	}while(0);

	return ret;
}
#endif

unsigned _stdcall  Judge_Proc(void *pData)
{
	int ret = OS_OK;
	int isExist = OS_NO;
	int solutionId = *(int *)pData;
	char *pJsonBuf = NULL;
	JUDGE_SUBMISSION_ST stJudgeSubmission;

	pJsonBuf = (char*)malloc(JSONBUFSIZE);
	if (NULL == pJsonBuf)
	{
		return OS_ERR;
	}

	memset(pJsonBuf, 0, JSONBUFSIZE);
	memset(&stJudgeSubmission, 0, sizeof(stJudgeSubmission));
	
	stJudgeSubmission.pszjudgeResult_Json = pJsonBuf;
		
	stJudgeSubmission.stSolution.solutionId = solutionId;
	Judge_InitSubmissionData(&stJudgeSubmission);

	write_log(JUDGE_INFO, "Start judge solution %d.", stJudgeSubmission.stSolution.solutionId);

	ret = SQL_getSolutionByID(stJudgeSubmission.stSolution.solutionId, &(stJudgeSubmission.stSolution), &isExist);
	if (OS_ERR == ret || OS_NO == isExist)
	{
		Judge_Debug(DEBUG_TYPE_ERROR, "No such solution %d.", stJudgeSubmission.stSolution.solutionId);
		free(pJsonBuf);
		return OS_ERR;
	}

	Judge_InitJudgePath(&stJudgeSubmission);
    
	ret = SQL_getSolutionSource(&stJudgeSubmission);
	if (OS_OK != ret)
	{
		free(pJsonBuf);
		Judge_Debug(DEBUG_TYPE_ERROR, "SQL_getSolutionSource failed.(solutionId=%d)", stJudgeSubmission.stSolution.solutionId);
		write_log(JUDGE_INFO,"SQL_getSolutionSource failed.(solutionId=%d)", stJudgeSubmission.stSolution.solutionId);
		return OS_ERR;
	}
    
	stJudgeSubmission.stProblem.problemId = stJudgeSubmission.stSolution.problemId;

	ret = SQL_getProblemInfo(&(stJudgeSubmission.stProblem));
	if (OS_OK != ret)
	{
		free(pJsonBuf);
		Judge_Debug(DEBUG_TYPE_ERROR, "SQL_getProblemInfo failed.(solutionId=%d)", stJudgeSubmission.stSolution.solutionId);
		write_log(JUDGE_INFO,"SQL_getProblemInfo failed.(solutionId=%d)", stJudgeSubmission.stSolution.solutionId);
		return OS_ERR;
	}

	write_log(JUDGE_INFO,"Prepare judge ok. (solutionId=%d, problemId=%u, isVirtualJudge=%u)",
		stJudgeSubmission.stSolution.solutionId, stJudgeSubmission.stSolution.problemId, stJudgeSubmission.stProblem.isVirtualJudge);

	if (OS_YES == stJudgeSubmission.stProblem.isVirtualJudge)
	{
		#if (OS_YES == OSP_MODULE_JUDGE_VJUDGE)
		if (OS_YES == Judge_IsVirtualJudgeEnable())
		{
			ret = Judge_Remote(&stJudgeSubmission);
			if (OS_OK != ret)
			{
				stJudgeSubmission.stSolution.verdictId = V_SK;
				Judge_Debug(DEBUG_TYPE_ERROR, "virtua-judge is fail...");
			}
		}
		else
		{
			Judge_Debug(DEBUG_TYPE_ERROR, "Error: virtual-judge is not enable.");
			stJudgeSubmission.stSolution.verdictId = V_SK;
		}

		#else
		stJudgeSubmission.stSolution.verdictId = V_SK;
		Judge_Debug(DEBUG_TYPE_ERROR, "virtua-judge is not support.");
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
					   stJudgeSubmission.stSolution.failcase,
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

	//DeleteFile(stJudgeSubmission.sourcePath);
	//DeleteFile(stJudgeSubmission.DebugFile);
	//DeleteFile(stJudgeSubmission.exePath);

	string time_string_;
	(VOID)util_time_to_string(time_string_, stJudgeSubmission.stSolution.submitDate);
	judge_outstring("\r\n -----------------------"
				"\r\n     *Judge verdict*"
				"\r\n -----------------------"
				"\r\n SolutionId   : %3d"
				"\r\n ProblemId    : %3d"
				"\r\n Lang.        : %3s"
				"\r\n Run cases    : %3d"
				"\r\n Failed cases : %3d"
				"\r\n Time-used    : %3d ms"
				"\r\n Memory-used  : %3d kb"
				"\r\n Return code  : 0x%x"
				"\r\n Verdict      : %3s"
				"\r\n Submit Date  : %3s"
				"\r\n Username     : %3s"
				"\r\n Json Result  : %s"
				"\r\n -----------------------\r\n",
					stJudgeSubmission.stSolution.solutionId,
					stJudgeSubmission.stProblem.problemId,
					stJudgeSubmission.languageName,
					stJudgeSubmission.stSolution.testcase,
					stJudgeSubmission.stSolution.failcase,
					stJudgeSubmission.stSolution.time_used - stJudgeSubmission.stSolution.time_used%10,
					stJudgeSubmission.stSolution.memory_used,
					stJudgeSubmission.dwProStatusCode,
					VERDICT_NAME[stJudgeSubmission.stSolution.verdictId],
					time_string_.c_str(), stJudgeSubmission.stSolution.username,
					stJudgeSubmission.pszjudgeResult_Json);

	free(pJsonBuf);
	
	return OS_OK;
}

void Judge_PushQueue(int solutionId)
{
	JUDGE_DATA_S jd = {0};
	
	jd.solutionId = solutionId;
	
	if (OS_YES == g_judge_enable)
	{
		Judge_Debug(DEBUG_TYPE_MSG, "Recieve judge request. (solution=%d).",solutionId);
		write_log(JUDGE_INFO,"Recieve judge request. (solution=%d).",solutionId);
		g_JudgeQueue.push(jd);
	}
	else
	{
		Judge_Debug(DEBUG_TYPE_MSG, "Recieve judge request, but judger is disable. (solution=%d).",solutionId);
		write_log(JUDGE_INFO,"Recieve judge request, but judger is disable. (solution=%d).",solutionId);
	
		SQL_updateSolution(solutionId,
					V_SK,
					0,
					0,
					0,
					0); 			
	}
	
}

/* virtual-judge & local-judge 应分两个队列 */
int Judge_DispatchThread(void *pEntry)
{
	JUDGE_DATA_S jd;

	for (;;)
	{
		if(!g_JudgeQueue.empty())
		{
				jd = g_JudgeQueue.front();

				/* 启动评判 */
				Judge_Proc((void*)&(jd.solutionId));
	
				g_JudgeQueue.pop();
		}

		Sleep(1000);
	}

	return 0;
}


void Judge_AutoJudgeTimer()
{
	int n = 0;
	JUDGE_DATA_S *pJudgeData = NULL;

	Judge_Debug(DEBUG_TYPE_FUNC, "Judge_AutoJudgeTimer.");
	
	pJudgeData = (JUDGE_DATA_S*)malloc(JUDGE_MAX_AUTOJUDGE * sizeof(JUDGE_DATA_S));
	if (NULL == pJudgeData)
	{
		return ;
	}

	SQL_getUnJudgeSolutions(pJudgeData, &n, JUDGE_MAX_AUTOJUDGE);

	for (int i = 0; i < n; i++)
	{
		Judge_PushQueue(pJudgeData->solutionId);
		pJudgeData++;
	}

	free(pJudgeData);

	return;
}

int Judge_TimerThread(void *pEntry)
{
	ULONG ulLoop = 0;

	for (;;)
	{		
        Sleep(1000);
        
		/* 60s */
		if (0 == ulLoop % 60)
		{   
            if( (file_access(dataPath, 0 )) == -1 )
            {
            	Judge_Debug(DEBUG_TYPE_ERROR, "Warning: Data path '%s' is not exist, "
                    "please check. Use the command judge data-path <path> to configure.", dataPath);
            	write_log(JUDGE_ERROR,"Warning: Data path '%s' is not exist, please check.", dataPath);
            }  
		}

		/* 60*10s */
		if (OS_YES == g_judge_timer_enable)
		{
			if (0 == ulLoop % (60 * g_judge_auto_detect_interval))
			{
				Judge_AutoJudgeTimer();
			}
		}

		ulLoop++;
	}

	return 0;
}

ULONG Judge_BuildRun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
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
	if (OS_YES == Judge_IsVirtualJudgeEnable())
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

	if (0 == strlen(*ppBuildrun))
	{
		free(*ppBuildrun);
		*ppBuildrun = NULL;
	}
	
	return OS_OK;
}

ULONG Judge_MGR_BuildRun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
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

	if (10 == g_judge_auto_detect_interval)
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
	

	if (OS_TRUE == g_ulNeedApiHookFlag)
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
	

	if (0 == strcmp(dataPath, "data"))
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"testcase-path %s", dataPath);
		}
	}
	else
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"testcase-path %s", dataPath);
	}
	
	return OS_OK;
}

#if (OS_YES == OSP_MODULE_JUDGE_VJUDGE)

ULONG Judge_VJUDGE_BuildRun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
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



	return OS_OK;
}
#endif
int Judge_Init()
{
	if(SQL_InitMySQL()==0)
	{
		write_log(JUDGE_ERROR,"Judge can not connect to MySQL(%s, %s, %s, %s, %d).",Mysql_url, Mysql_username, Mysql_password, Mysql_table, Mysql_port);
		printf("Error: Judge can not connect to MySQL(%s, %s, %s, %s, %d).\r\n",Mysql_url, Mysql_username, Mysql_password, Mysql_table, Mysql_port);
	}
	
	return OS_OK;
}

int Judge_TaskEntry(void *pEntry)
{
	write_log(JUDGE_INFO,"Running Judge Core...");

	Judge_RegCmd();
	
	(void)BDN_RegistBuildRun(MID_JUDGE, VIEW_SYSTEM, BDN_PRIORITY_NORMAL, Judge_BuildRun);
	(void)BDN_RegistBuildRun(MID_JUDGE, VIEW_JUDGE_MGR, BDN_PRIORITY_LOW, Judge_MGR_BuildRun);

	#if (OS_YES == OSP_MODULE_JUDGE_VJUDGE)
	(void)BDN_RegistBuildRun(MID_JUDGE, VIEW_VJUDGE_MGR, BDN_PRIORITY_LOW - 1, Judge_VJUDGE_BuildRun);
	#endif
	
	(void)thread_create(Judge_DispatchThread, NULL);
	
	(void)thread_create(Judge_TimerThread, NULL);

	write_log(JUDGE_INFO,"Judge Task init ok...");

	/* 循环读取消息队列 */
	for(;;)
	{
		/* 放权 */
		Sleep(10);
	}

	closesocket(g_sListen);

#ifdef _WIN32_	
	WSACleanup();
#endif

	return 0;
}

APP_INFO_S g_judgeAppInfo =
{
	NULL,
	"Judge",
	Judge_Init,
	Judge_TaskEntry
};

#if 0
int  Test1(int evtId, int cmdId, void *pData, void **ppInfo)
{
	Judge_Debug(DEBUG_TYPE_FUNC, "Do test1, evtId=%d, cmdId=%d..", evtId, cmdId);

	return OS_OK;
}

int  Test2(int evtId, int cmdId, void *pData, void **ppInfo)
{
	Judge_Debug(DEBUG_TYPE_FUNC, "Do test2, evtId=%d, cmdId=%d..", evtId, cmdId);
	return OS_OK;
}

int  Test3(int evtId, int cmdId, void *pData, void **ppInfo)
{
	Judge_Debug(DEBUG_TYPE_FUNC, "Do test3, evtId=%d, cmdId=%d..", evtId, cmdId);
	return OS_OK;
}
int  Test4(int evtId, int cmdId, void *pData, void **ppInfo)
{
	Judge_Debug(DEBUG_TYPE_FUNC, "Do test4, evtId=%d, cmdId=%d..", evtId, cmdId);
	return OS_OK;
}
#endif

void Judge_RegAppInfo()
{
	APP_RegistInfo(&g_judgeAppInfo);
}

#endif

