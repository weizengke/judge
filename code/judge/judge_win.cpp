#include "judge/include/judge_inc.h"

#if (OS_YES == OSP_MODULE_JUDGE)
using namespace std;

#ifdef WIN32
extern char g_judge_apihook_path[MAX_PATH];
int judge_privilege_disable(thread_id_t process)
{
	thread_id_t token;
	if (!OpenProcessToken(process, TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &token)) {
		write_log(JUDGE_SYSTEM_ERROR,"OpenProcessToken error\n");
		return 1;
	}

	LUID luid;
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
		write_log(JUDGE_SYSTEM_ERROR,"LookupPrivilege error!\n");
	}

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = luid;

	if (!AdjustTokenPrivileges(token, TRUE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
		write_log(JUDGE_SYSTEM_ERROR,"AdjustTokenPrivileges error!\n");
		CloseHandle(token);
		return 1;
	}

	CloseHandle(token);
	
	return 0;
}

int judge_privilege_enable(LPCSTR name)
{
	thread_id_t token;
	
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &token)) {
		write_log(JUDGE_ERROR,"OpenProcessToken error\n");
		return 1;
	}

	LUID luid;
	if (!LookupPrivilegeValue(NULL, name, &luid)) {
		write_log(JUDGE_ERROR,"LookupPrivilege error!\n");
	}

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes =SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = luid;

	if (!AdjustTokenPrivileges(token, 0, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
		write_log(JUDGE_ERROR,"AdjustTokenPrivileges error!\n");
		return 1;
	}

	return 0;
}

thread_id_t judge_create_sandbox(JUDGE_SUBMISSION_S *submission)
{
	thread_id_t job = CreateJobObject(NULL,NULL);
	if (job != NULL) {
		JOBOBJECT_BASIC_LIMIT_INFORMATION jobli;
		 memset(&jobli,0,sizeof(jobli));
		jobli.LimitFlags=JOB_OBJECT_LIMIT_PRIORITY_CLASS|JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
		jobli.PriorityClass=IDLE_PRIORITY_CLASS;
		jobli.ActiveProcessLimit=submission->nProcessLimit;
	//	jobli.MinimumWorkingSetSize= 1;
	//	jobli.MaximumWorkingSetSize= 1024*GL_memory_limit;|JOB_OBJECT_LIMIT_WORKINGSET|JOB_OBJECT_LIMIT_PROCESS_TIME
	//	jobli.PerProcessUserTimeLimit.QuadPart=10000*(GL_time_limit+2000);
		if (SetInformationJobObject(job, JobObjectBasicLimitInformation, &jobli, sizeof(jobli))) {
			JOBOBJECT_BASIC_UI_RESTRICTIONS jobuir;
			jobuir.UIRestrictionsClass= 0x00000000 /*JOB_OBJECT_UILIMIT_NONE*/;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_EXITWINDOWS;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_READCLIPBOARD ;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_WRITECLIPBOARD ;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS;
			jobuir.UIRestrictionsClass |=JOB_OBJECT_UILIMIT_HANDLES;

			if (SetInformationJobObject(job, JobObjectBasicUIRestrictions, &jobuir, sizeof(jobuir))) {
				return job;
			} else {
				write_log(JUDGE_SYSTEM_ERROR,"SetInformationJobObject JOBOBJECT_BASIC_UI_RESTRICTIONS [Error:%d]\n", GetLastError());
			}
		} else {
			write_log(JUDGE_SYSTEM_ERROR,"SetInformationJobObject JOBOBJECT_BASIC_LIMIT_INFORMATION [Error:%d]\n", GetLastError());
		}
	} else {
		write_log(JUDGE_SYSTEM_ERROR,"CreateJobObject [Error:%d]\n", GetLastError());
	}

	return NULL;
}

int judge_assign_sandbox(thread_id_t job, PROCESS_INFORMATION p)
{
	if (AssignProcessToJobObject(job, p.hProcess)) {
		/*
		thread_id_t   hPS   =   OpenProcess(PROCESS_ALL_ACCESS,   false,  p.dwProcessId);
		if(!SetPriorityClass(hPS,   HIGH_PRIORITY_CLASS))
		{
			write_log(JUDGE_SYSTEM_ERROR,"SetPriorityClass        [Error:%d]\n",GetLastError());
		}
		CloseHandle(hPS);
		*/

		judge_privilege_disable(p.hProcess);
		return OS_OK;
	} else {
		write_log(JUDGE_SYSTEM_ERROR,"AssignProcessToJobObject Error:%d", GetLastError());
	}

	return OS_ERR;
}

int judge_compile_thread(void *data)
{
	JUDGE_SUBMISSION_S *submission = (JUDGE_SUBMISSION_S *)data;

	if (NULL == submission) {
		write_log(JUDGE_ERROR,"compile thread submission is NULL.");
		return OS_ERR;
	}

	write_log(JUDGE_INFO,"compile thread start. (sessionId=%s)", submission->sessionId);
	system(submission->compileCmd);
	write_log(JUDGE_INFO,"compile thread end. (sessionId=%s)", submission->sessionId);

	return OS_OK;
}

int judge_compile_run(JUDGE_SUBMISSION_S *submission)
{
    thread_id_t th  = thread_create(judge_compile_thread, (void*)submission);
    if (th == NULL) {
        write_log(JUDGE_ERROR,"compile thread create error.");
        thread_close(th);
        return OS_ERR;
    }
    
    if (WaitForSingleObject(th, 30000) > 0) {
        write_log(JUDGE_WARNING,"compile over time_limit.");  
		thread_close(th);		      
    }
	
	return OS_OK;
}
BOOL judge_exception_check(DWORD dw)
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

BOOL judge_api_hook_injeck(DWORD processId, char* dll)
{
    FARPROC FuncAddr = NULL;

	if ((file_access(dll, 0 )) == -1) {
		write_log(JUDGE_ERROR,"InjectAPIHookDll failed, the file %s is not exist.", dll);
		return FALSE;
	}

	if (judge_privilege_enable(SE_DEBUG_NAME)) {
		write_log(JUDGE_ERROR,"add privilege error %u", GetLastError());
		return FALSE;
	}

    HMODULE hdll = LoadLibrary(TEXT("Kernel32.dll"));
    if (hdll == NULL) {
		write_log(JUDGE_ERROR,"LoadLibrary Kernel32.dll error %u", GetLastError());
		return FALSE;
    }

	FuncAddr = GetProcAddress(hdll, "LoadLibraryA");
	if (FuncAddr == NULL) {
		write_log(JUDGE_ERROR,"GetProcAddress error %u", GetLastError());
		return FALSE;
	}

    thread_id_t process = OpenProcess(PROCESS_ALL_ACCESS 
										| PROCESS_CREATE_THREAD 
										| PROCESS_VM_OPERATION 
										| PROCESS_VM_WRITE, 
										FALSE, 
										processId);
    if (process == NULL) {
		write_log(JUDGE_ERROR,"OpenProcess error %u", GetLastError());
		return FALSE;
    }

	char current_path[MAX_PATH] = {0};
	char dllpath[MAX_PATH] = {0};
	get_current_directory(sizeof(current_path), current_path);
	sprintf(dllpath, "%s//%s",current_path, dll);

    DWORD dwSize = strlen(dllpath) + 1;
    LPVOID RemoteBuf = VirtualAllocEx(process, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
    if (NULL == RemoteBuf) {
		write_log(JUDGE_ERROR,"VirtualAllocEx error %u", GetLastError());
		return FALSE;
	}

    DWORD dwRealSize;
    if (WriteProcessMemory(process, RemoteBuf, dllpath, dwSize, &dwRealSize)) {
        thread_id_t hRemoteThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)FuncAddr, RemoteBuf, 0, NULL);
        if (hRemoteThread == NULL) {
			write_log(JUDGE_ERROR,"CreateRemoteThread error %u", GetLastError());
            if (!VirtualFreeEx(process, RemoteBuf, dwSize, MEM_COMMIT)) {
				write_log(JUDGE_ERROR,"VirtualFreeEx error %u", GetLastError());
			}
			
            CloseHandle(process);
            return FALSE;
        }

        /* 释放资源 */
        WaitForSingleObject(hRemoteThread, INFINITE);
        CloseHandle(hRemoteThread);
        VirtualFreeEx(process, RemoteBuf, dwSize, MEM_COMMIT);
        CloseHandle(process);

		write_log(JUDGE_INFO,"Api hook ok. dllpath=%s", dllpath);
        return TRUE;
    } else {
		write_log(JUDGE_ERROR,"WriteProcessMemory error %u", GetLastError());
        VirtualFreeEx(process, RemoteBuf, dwSize, MEM_COMMIT);
        CloseHandle(process);
        return FALSE;
    }
}

bool judge_process_kill(PROCESS_INFORMATION processInfo)
{    
	DWORD processId = processInfo.dwProcessId;
    PROCESSENTRY32 processEntry = {0};

	write_log(JUDGE_INFO, "killProcess, processId=%u", processId);
	
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    thread_id_t handleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if( Process32First(handleSnap, &processEntry)) {        
		BOOL isContinue = TRUE;
		do {            
			if (processEntry.th32ParentProcessID == processId) {     
				thread_id_t hChildProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processEntry.th32ProcessID);  
				if (hChildProcess) {             
					TerminateProcess(hChildProcess, 0);   
					CloseHandle(hChildProcess);          
				} else {
					write_log(JUDGE_ERROR, "killProcess, OpenProcess processId=%u failed.", processId);
				}
			}          

			isContinue = Process32Next(handleSnap, &processEntry); 
		} while (isContinue); 
		
		thread_id_t hBaseProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId); 
		if (hBaseProcess) {   
			TerminateProcess(hBaseProcess, 0);   
			CloseHandle(hBaseProcess);      
		} else {
			write_log(JUDGE_ERROR, "killProcess, OpenProcess2 processId=%u failed.", processId);
		}		
	}	
	
	DWORD exitCode = 0;	
	GetExitCodeProcess(processInfo.hProcess, &exitCode);
	if (exitCode == STILL_ACTIVE) {      
		write_log(JUDGE_ERROR, "killProcess failed. processId=%u", processId);
		return false;
	}   
	
	return true;
}

int judge_solution_run(JUDGE_SUBMISSION_S *submission)
{
	if (submission->mode == JUDGE_SUBMIT_MODE) {
		/* verdict to running testcase xxx to db */
		JUDGE_SUBMISSION_S submission_ = {0};
		memcpy(&submission_, submission, sizeof(JUDGE_SUBMISSION_S));
		submission_.solution.verdictId = V_RUN;
		SQL_updateSolution(&submission_);
	}

	/* set no error-box mode */
	SetErrorMode(SEM_NOGPFAULTERRORBOX);

	if (submission->inFile == 0) {
		SECURITY_ATTRIBUTES sa = {0};
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		submission->hInputFile = CreateFile(submission->inFileName,
						GENERIC_READ,
						0,
						&sa,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
		if (submission->hInputFile == INVALID_HANDLE_VALUE) {
			write_log(JUDGE_ERROR, "solution run ERROR, CreateFile hInputFile failed. [Error:%d]\n",GetLastError());
			return OS_OK;
		}
	}

	if (submission->outFile == 0) {
		SECURITY_ATTRIBUTES sa = {0};
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		submission->hOutputFile  = CreateFile(submission->outFileName,
						GENERIC_WRITE,
						0,
						&sa,
						CREATE_ALWAYS,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
		if (submission->hOutputFile == INVALID_HANDLE_VALUE) {
			write_log(JUDGE_ERROR, "solution run ERROR, CreateFile hOutputFile failed. [Error:%d]\n",GetLastError());
			CloseHandle(submission->hInputFile);
			submission->hInputFile = INVALID_HANDLE_VALUE;
			return OS_OK;
		}
	}

	STARTUPINFO si = {sizeof(STARTUPINFO)};
	si.hStdInput = submission->hInputFile;
	si.hStdOutput= submission->hOutputFile;
	si.hStdError = submission->hOutputFile;
	si.wShowWindow = SW_HIDE;
	si.dwFlags =  STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	if (CreateProcess(NULL, submission->runCmd, NULL, NULL, TRUE, CREATE_SUSPENDED,
				      NULL, submission->workPath, &si, &submission->stProRunInfo)) {
		judge_privilege_disable(submission->stProRunInfo.hProcess);

		write_log(JUDGE_INFO,"judge create process ok. "
							"(dwProcessId=%u, workPath=%s, runCmd=%s, ApiHookEnable=%u)",
							submission->stProRunInfo.dwProcessId, submission->workPath,
							submission->runCmd, g_judge_api_hook_enable);

		/* 安全防护api hook */
		/* Begin add for apihook 2013/05/18 */
		if (OS_YES == g_judge_api_hook_enable) {
			if (FALSE == judge_api_hook_injeck(submission->stProRunInfo.dwProcessId, g_judge_apihook_path)) {
				write_log(JUDGE_ERROR,"InjectAPIHookDll Error, dwProcessId[%u]", submission->stProRunInfo.dwProcessId);
			}

			submission->hJob = judge_create_sandbox(submission);
			if (submission->hJob != NULL) {
				if (OS_OK != judge_assign_sandbox(submission->hJob, submission->stProRunInfo)) {
				}
			}
		}
		/* End add for apihook 2013/05/18 */

		/* 唤醒进程 */
		ResumeThread(submission->stProRunInfo.hThread);

		/* 计时开始 */
		submission->startt = clock();

		/* 放宽时限1000mS,返回值大于零说明超时. */
		WaitForSingleObject(submission->stProRunInfo.hProcess, submission->problem.time_limit + 1000);  

		/* 计时结束 */
		submission->endt = clock();

		/* 释放文件句柄 */
		CloseHandle(submission->hInputFile);
		submission->hInputFile = INVALID_HANDLE_VALUE;
		CloseHandle(submission->hOutputFile);
		submission->hOutputFile = INVALID_HANDLE_VALUE;

		return OS_OK;
	} else {
		write_log(JUDGE_ERROR,"Judge create process failed. (runCmd=%s, tick=%u)",
								submission->runCmd, GetTickCount());

		CloseHandle(submission->hInputFile);
		submission->hInputFile = INVALID_HANDLE_VALUE;
		CloseHandle(submission->hOutputFile);
		submission->hOutputFile = INVALID_HANDLE_VALUE;
	}

	return OS_ERR;
}

int judge_get_testcases(JUDGE_SUBMISSION_S *submission, char szTestcases[JUDGE_MAX_CASE][UTIL_MAX_FNAME])
{
	char szPath[UTIL_MAX_PATH] = {0};
	int i = 0;
	long Handle;
	struct _finddata_t FileInfo;
	int caseNum = 0;

	if (submission->mode == JUDGE_TEST_MODE) {
		sprintf(szTestcases[caseNum++], "data");
		return caseNum;
	}

	sprintf(szPath, "%s\/%d\/*.in", g_judge_testcase_path, submission->problem.problemId);

	if (-1L != (Handle=_findfirst(szPath, &FileInfo))) {
		char path_buffer[UTIL_MAX_PATH];
		char drive[UTIL_MAX_DRIVE];
		char dir[UTIL_MAX_DIR];
		char fname[UTIL_MAX_FNAME];
		char ext[UTIL_MAX_EXT];


		util_splitpath(FileInfo.name, drive, dir, fname, ext);
		sprintf(szTestcases[caseNum++], "%s", fname);

		while (_findnext(Handle,&FileInfo) == 0) {
			util_splitpath(FileInfo.name, drive, dir, fname, ext);
			sprintf(szTestcases[caseNum++], "%s", fname);
			if (caseNum >= JUDGE_MAX_CASE) {
				break;
			}
		}

		_findclose(Handle);

		qsort(szTestcases, caseNum, sizeof(char)*UTIL_MAX_FNAME, string_cmp);
	}
	return caseNum;
}

void judge_solution_memory_check(JUDGE_SUBMISSION_S *submission)
{
	PROCESS_MEMORY_COUNTERS pmc;

	if (GetProcessMemoryInfo(submission->stProRunInfo.hProcess, &pmc, sizeof(pmc))) {
		submission->solution.memory_cur = pmc.PeakWorkingSetSize/1024;
		if (submission->solution.memory_cur > submission->solution.memory_used) {
			submission->solution.memory_used = submission->solution.memory_cur;
		}
	}

	if (submission->solution.memory_cur >= submission->problem.memory_limit) {
		submission->solution.verdictId = V_MLE;
		submission->solution.memory_used = submission->problem.memory_limit;
	}	
}

void judge_solution_time_check(JUDGE_SUBMISSION_S *submission)
{
	if(submission->dwProStatusCode == STILL_ACTIVE) {
		write_log(JUDGE_INFO, "case %u STILL_ACTIVE, to TIME LIMIT, verdictId:%s",
							  submission->solution.testcase, 
							  VERDICT_NAME[submission->solution.verdictId]);

		if (submission->solution.verdictId == V_AC) {
			submission->solution.verdictId = V_TLE;
			submission->solution.time_cur = submission->problem.time_limit;
			submission->solution.time_used = submission->problem.time_limit;
			return;
		}
	}

	submission->solution.time_cur = submission->endt - submission->startt;
	if (submission->solution.time_cur < 0) {
		submission->solution.time_cur = submission->problem.time_limit;
	}

	/* more than time_limit */
	if (submission->solution.time_cur >= submission->problem.time_limit) {
		submission->solution.verdictId = V_TLE;
		submission->solution.time_used = submission->problem.time_limit;
		return;
	}

	/* more than time_used */
	if (submission->solution.time_cur > submission->solution.time_used) {
		submission->solution.time_used = submission->solution.time_cur;
	}
}

void judge_solution_exit_check(JUDGE_SUBMISSION_S *submission)
{
	//get process state
	GetExitCodeProcess(submission->stProRunInfo.hProcess, &(submission->dwProStatusCode));
	
	write_log(JUDGE_INFO, "dwProStatusCode=%u, lastError=%u", submission->dwProStatusCode, GetLastError());
	
	if (STILL_ACTIVE == submission->dwProStatusCode) {
		(VOID)judge_process_kill(submission->stProRunInfo);
	}

	CloseHandle(submission->stProRunInfo.hProcess);
	submission->stProRunInfo.hProcess = NULL;
	CloseHandle(submission->stProRunInfo.hThread);
	submission->stProRunInfo.hThread = NULL;
	TerminateJobObject(submission->hJob,0);
	CloseHandle(submission->hJob);submission->hJob = NULL;
	
	/* check exception */
	if(judge_exception_check(submission->dwProStatusCode)) {
		submission->solution.verdictId = V_RE;
		return;
	} 

	return;
}
#endif

#endif