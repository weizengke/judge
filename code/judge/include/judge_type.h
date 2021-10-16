#ifndef _JUDGE_TYPE_H_
#define _JUDGE_TYPE_H_

#include "util/utlist.h"

#ifdef _LINUX_
#include <csignal>
#include <wait.h>
#include <sys/resource.h>
#endif

#define JUDGE_OJNAME_SIZE_MAX 32

typedef struct tagJudge_Req_Msg_S
{
	int solutionId;
	int vtyId;
	char *jsonMsg;
}JUDGE_REQ_MSG_S;

typedef struct tag_Judge_ProblemInfo_S
{
	int problemId;
	int time_limit;
	int memory_limit;
	int isSpecialJudge;

	int isVirtualJudge;
	char virtualPID[32];
	char szVirJudgerName[JUDGE_OJNAME_SIZE_MAX]; /* oj name */

}JUDGE_PROBLEM_S;

typedef struct tagJUDGE_TESTCASE_S {
	int case_id;
	int verdict;
	int time_used;
	int memory_used;
	struct tagJUDGE_TESTCASE_S *next;
}JUDGE_TESTCASE_S;

typedef struct tag_Judge_Solution_S
{
	int solutionId;
	int problemId;
	int languageId;
	int verdictId;
	int contestId;
	time_t submitDate;
	char username[MAX_NAME];

	int time_cur;
	int memory_cur;
	int time_used;
	int memory_used;
	int testcase;
	int failcase; 
	
	int reJudge;
}JUDGE_SOLUTION_S;

typedef struct tag_Judge_Submission_S
{
	JUDGE_SOLUTION_S solution;
	JUDGE_PROBLEM_S  problem;

	int vty;
	int mode; /* 0:submit, 1:test */
	int inFile; 
	int outFile; 
    int isTranscoding;
    int limitIndex;
    int nProcessLimit;

	char languageName[100];
	char languageExt[10];
	char languageExe[10];

	unsigned long ulSeed;
	char workPath[MAX_PATH];
	char subPath[MAX_PATH];
	char sourcePath[MAX_PATH];
	char exePath[MAX_PATH];
    char inFileName[MAX_PATH];
    char outFileName[MAX_PATH];	
	char stdInFileName[MAX_PATH];
	char stdOutFileName[MAX_PATH];
    char DebugFile[MAX_PATH];
    char ErrorFile[MAX_PATH];
    char judge_log_filename[MAX_PATH];
	char judge_log_test_json[MAX_PATH];

	char sessionId[128];
	cJSON *judge_request_json;
	char *judge_result_json; /* ��ʽ: json */
	JUDGE_TESTCASE_S *testcases_result;

    char compileCmd[512];
    char runCmd[MAX_PATH];

#if (OS_YES == OSP_MODULE_JUDGE_LOCAL)
	#ifdef WIN32
	PROCESS_INFORMATION stProRunInfo; /* ���н�����Ϣ */
	PROCESS_INFORMATION stProComInfo; /* ���������Ϣ */
	#endif

	#ifdef _LINUX_
	rusage rused;
	pid_t exec_pid;
	int status;
	#endif
#endif
	thread_id_t hJob;         /* ��ҵ��� */
    thread_id_t hInputFile ;  /* �����������ļ���� */
    thread_id_t hOutputFile;  /* �ӽ��̱�׼������ */

    DWORD dwProStatusCode;     /* �������״̬ */

	clock_t startt; /* ÿ��run��ʱ��� */
	clock_t endt ;  /* ÿ��run��ʱ��� */

}JUDGE_SUBMISSION_S;

#endif
