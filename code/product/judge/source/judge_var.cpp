#include "..\include\judge_inc.h"

MYSQL *mysql;     //mysql连接
char query[1024]; //查询语句


char GL_username[MAX_NAME];
int GL_solutionId;
int GL_problemId;
int GL_languageId;
int GL_verdictId;
int GL_contestId;
int GL_time;
int GL_memory;
int GL_reJudge;
int GL_testcase;
time_t GL_submitDate;
char GL_languageName[100]={0};
char GL_languageExt[10]={0};
char GL_languageExe[10]={0};
int GL_time_limit;
int GL_memory_limit;
int GL_spj;

string GL_source;   /* no used */


int isTranscoding=0;   //针对VS的转码
int limitIndex=1;
int nProcessLimit=1;

char sourcePath[1024]={0};
char exePath[1024]={0};


char inFileName[MAX_PATH];
char outFileName[MAX_PATH];
char DebugFile[MAX_PATH];
char ErrorFile[MAX_PATH];


int g_oj_debug_switch = JUDGE_DEBUG_ON;
