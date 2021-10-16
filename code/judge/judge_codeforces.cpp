
#include "judge/include/judge_inc.h"

#if (OS_YES == OSP_MODULE_JUDGE)

using namespace std;

char codeforces_username[128]= "";
char codeforces_password[128]= "";
char codeforces_judgerIP[20]="127.0.0.1";
int codeforces_sockport = 0;
int codeforces_remote_enable = OS_NO;
int codeforces_vjudge_enable = OS_NO;

char g_codeforces_result[512]  = "codeforces_result.json";
char codeforces_jsonBuff[1000000] = {0};

extern int g_judge_log_buffsize ;

void codeforces_solution_log_testcase(char *judge_log_filename, int testcase, char *buff)
{
	(void)util_fwrite(judge_log_filename, "Test case %s", buff);

	(void)util_fwrite(judge_log_filename,"\n------------------------------------------------------------------\n");

}

int codeforces_python(JUDGE_SUBMISSION_S *submission)
{
	(void)util_freset(g_codeforces_result);
	(void)util_freset(submission->judge_log_filename);

	char cmd_string[MAX_PATH];
	sprintf(cmd_string,"python -O codeforces.py %s %s %s %s %s %s",
			codeforces_username,
			codeforces_password,
			submission->problem.virtualPID, 
			"c", 
			submission->sourcePath,
			g_codeforces_result);

	Judge_Debug(DEBUG_TYPE_FUNC, "cmd_string=%s", cmd_string);
	system(cmd_string);

	memset(codeforces_jsonBuff, 0, sizeof(codeforces_jsonBuff));
	(void)util_fread(g_codeforces_result, codeforces_jsonBuff, sizeof(codeforces_jsonBuff) - 1);

	write_log(JUDGE_INFO, "codeforces_result:len=%d, buff:%s", strlen(codeforces_jsonBuff), codeforces_jsonBuff);

	cJSON *json = cJSON_Parse(codeforces_jsonBuff);
	if (json == NULL) {
		write_log(JUDGE_INFO, "cJSON_Parse failed.");
		submission->solution.verdictId = V_SK;
		return 0;
	}

	cJSON *json_status_msg = cJSON_GetObjectItem(json, "verdict");
    if(json_status_msg != NULL && json_status_msg->type == cJSON_String) {
        write_log(JUDGE_INFO, "verdict=%s", json_status_msg->valuestring);
		if (0 != strstr(json_status_msg->valuestring, "Compilation error")) {
			submission->solution.verdictId = V_CE;
			cJSON *json_compile_err = cJSON_GetObjectItem(json, "compile_error");
			if (json_compile_err != NULL && json_compile_err->type == cJSON_String && json_compile_err->valuestring != NULL) {
				write_log(JUDGE_INFO, "json_compile_err:%s\n", json_compile_err->valuestring);

				FILE *fp;
				char buffer[4096]={0};
				if ((fp = fopen (submission->DebugFile, "w")) != NULL){
					fputs(json_compile_err->valuestring, fp);
					fclose(fp);
				}
			}
		} else if (0 != strstr(json_status_msg->valuestring, "Accepted")) {
			submission->solution.verdictId = V_AC;
		} else if (0 != strstr(json_status_msg->valuestring, "Wrong answer")) {
			submission->solution.verdictId = V_WA;
		} else if (0 != strstr(json_status_msg->valuestring, "Time limit exceeded")) {
			submission->solution.verdictId = V_TLE;
		} else if (0 != strstr(json_status_msg->valuestring, "Output limit exceeded")) {
			submission->solution.verdictId = V_OLE;
		} else if (0 != strstr(json_status_msg->valuestring, "Memory limit exceeded")) {
			submission->solution.verdictId = V_MLE;
		} else if (0 != strstr(json_status_msg->valuestring, "Runtime error")) {
			submission->solution.verdictId = V_RE;
		}
    }  else {
		submission->solution.verdictId = V_SK;
	}

    cJSON *json_time = cJSON_GetObjectItem(json, "time");
    if (json_time != NULL &&  json_time->type == cJSON_String && json_time->valuestring != NULL) {
        write_log(JUDGE_INFO, "time: %s", json_time->valuestring);
		sscanf(json_time->valuestring, "%d ms", &submission->solution.time_used);
    }

	cJSON *json_memory = cJSON_GetObjectItem(json, "memory");
    if(json_memory != NULL &&  json_memory->type == cJSON_String && json_memory->valuestring != NULL) {
        write_log(JUDGE_INFO, "memory=%s", json_memory->valuestring);
		sscanf(json_memory->valuestring, "%d KB", &submission->solution.memory_used);
		//submission->solution.memory_used = submission->solution.memory_used/1024;
    } 

	int total_testcases = 0;
	cJSON *json_testcases = cJSON_GetObjectItem(json, "testcases");
    if(json_testcases != NULL &&  json_testcases->type == cJSON_Array) {
		total_testcases = cJSON_GetArraySize(json_testcases);
		submission->solution.testcase = total_testcases;
        write_log(JUDGE_INFO, "total_testcases=%d", total_testcases);
    } 

	for (int testcase = 0; testcase < total_testcases; testcase++) {
		cJSON *json_testcase = cJSON_GetArrayItem(json_testcases, testcase);
		if (json_testcase != NULL &&  json_testcase->type == cJSON_String && json_testcase->valuestring != NULL) {
			//write_log(JUDGE_INFO, "valuestring=%s\n", json_testcase->valuestring);
			codeforces_solution_log_testcase(submission->judge_log_filename, testcase + 1, json_testcase->valuestring);			
		}
	}

	cJSON_Delete(json);

	return 0;
}

int codeforces_vjudge(JUDGE_SUBMISSION_S *submisssion)
{
	return codeforces_python(submisssion);
}

int codeforces_test()
{
	JUDGE_SUBMISSION_S submisssion = {0};
	strcpy(submisssion.problem.virtualPID, "1A");
	strcpy(submisssion.sourcePath, "D:\\code\\codeforces\\solution.c");
	strcpy(submisssion.DebugFile, "D:\\code\\codeforces\\compile.txt");

	codeforces_python(&submisssion);
	return 0;
}

#endif /* #if (OS_YES == OSP_MODULE_JUDGE) */