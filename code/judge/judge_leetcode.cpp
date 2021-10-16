
#include "judge/include/judge_inc.h"

#if (OS_YES == OSP_MODULE_JUDGE)

using namespace std;

char leetcode_username[128]={0};
char leetcode_password[128]={0};
char leetcode_judgerIP[20]="127.0.0.1";
int leetcode_sockport = 0;
int leetcode_remote_enable = OS_NO;
int leetcode_vjudge_enable = OS_NO;

char g_leetcode_result[512]  = "leetcode_result.json";
char jsonBuff[1000000] = {0};

extern int g_judge_log_buffsize ;
void leetcode_solution_log(char *judge_log_filename, int verdictId, int memory, int time,
						   int pass_case, int total_case,
					       char *input, char *output, char *answer)
{	
	char *buf = (char*)malloc(g_judge_log_buffsize + 1);
	if (buf == NULL) {
		return;
	}

	util_freset(judge_log_filename);

	(void)util_fwrite(judge_log_filename,
				"passed_case: %d, total_case: %d, time: %d ms, memory: %d kb, verdict: %s",
				pass_case, total_case, time, memory, VERDICT_NAME[verdictId]);

	(void)util_fwrite(judge_log_filename,"\nInput\n");
	if (input) {
		(void)util_fwrite(judge_log_filename,input);
	}
	
	(void)util_fwrite(judge_log_filename,"\nOutput\n");
	if (output) {
		(void)util_fwrite(judge_log_filename, output);
	}
	
	(void)util_fwrite(judge_log_filename,"\nAnswer\n");
	if (answer) {
		(void)util_fwrite(judge_log_filename, answer);
	}

	(void)util_fwrite(judge_log_filename,"\n------------------------------------------------------------------\n");

	free(buf);
}

int leetcode_python(JUDGE_SUBMISSION_S *submission)
{
	(void)util_freset(g_leetcode_result);

	char cmd_string[MAX_PATH];
	sprintf(cmd_string,"python -O leetcode.py %s %s %s %s %s %s",
			leetcode_username,
			leetcode_password,
			submission->problem.virtualPID, 
			"c", 
			submission->sourcePath,
			g_leetcode_result);

	system(cmd_string);

	memset(jsonBuff, 0, sizeof(jsonBuff));
	(void)util_fread(g_leetcode_result, jsonBuff, sizeof(jsonBuff) - 1);

	write_log(JUDGE_INFO, "leetcode_result:len=%d, buff:%s", strlen(jsonBuff), jsonBuff);

	cJSON *json = cJSON_Parse(jsonBuff);
	if (json == NULL) {
		write_log(JUDGE_INFO, "cJSON_Parse failed.");
		submission->solution.verdictId = V_SK;
		return 0;
	}

	cJSON *json_status_msg = cJSON_GetObjectItem(json, "status_msg");
    if(json_status_msg != NULL && json_status_msg->type == cJSON_String) {
        write_log(JUDGE_INFO, "status_msg=%s\n", json_status_msg->valuestring);
		if (0 == strcmp(json_status_msg->valuestring, "Compile Error")) {
			submission->solution.verdictId = V_CE;
			cJSON *json_compile_err = cJSON_GetObjectItem(json, "full_compile_error");
			if (json_compile_err != NULL && json_compile_err->type == cJSON_String && json_compile_err->valuestring != NULL) {
				write_log(JUDGE_INFO, "json_compile_err:%s\n", json_compile_err->valuestring);

				FILE *fp;
				char buffer[4096]={0};
				if ((fp = fopen (submission->DebugFile, "w")) != NULL){
					fputs(json_compile_err->valuestring, fp);
					fclose(fp);
				}
			}
		} else if (0 == strcmp(json_status_msg->valuestring, "Accepted")) {
			submission->solution.verdictId = V_AC;
		} else if (0 == strcmp(json_status_msg->valuestring, "Wrong Answer")) {
			submission->solution.verdictId = V_WA;
		} else if (0 == strcmp(json_status_msg->valuestring, "Time Limit Exceeded")) {
			submission->solution.verdictId = V_TLE;
		} else if (0 == strcmp(json_status_msg->valuestring, "Output Limit Exceeded")) {
			submission->solution.verdictId = V_OLE;
		} else if (0 == strcmp(json_status_msg->valuestring, "Memory Limit Exceeded")) {
			submission->solution.verdictId = V_MLE;
		} else if (0 == strcmp(json_status_msg->valuestring, "Runtime Error")) {
			submission->solution.verdictId = V_RE;
		}
    }  else {
		submission->solution.verdictId = V_SK;
	}

    cJSON *json_time = cJSON_GetObjectItem(json, "status_runtime");
    if (json_time != NULL &&  json_time->type == cJSON_String && json_time->valuestring != NULL) {
        write_log(JUDGE_INFO, "time: %s\n", json_time->valuestring);
		sscanf(json_time->valuestring, "%d ms", &submission->solution.time_used);
    }

	cJSON *json_memory = cJSON_GetObjectItem(json, "memory");
    if(json_memory != NULL &&  json_memory->type == cJSON_Number) {
        write_log(JUDGE_INFO, "memory=%d\n", json_memory->valueint);
		submission->solution.memory_used = json_memory->valueint/1024;
    } 

	int total_testcases = 0;
	cJSON *json_total_testcases = cJSON_GetObjectItem(json, "total_testcases");
    if(json_total_testcases != NULL &&  json_total_testcases->type == cJSON_Number) {
        write_log(JUDGE_INFO, "total_testcases=%d\n", json_total_testcases->valueint);
		total_testcases = json_total_testcases->valueint;
		submission->solution.testcase = total_testcases;
    } 

	int correct = 0;
	cJSON *json_correct = cJSON_GetObjectItem(json, "total_correct");
    if(json_correct != NULL &&  json_correct->type == cJSON_Number) {
        write_log(JUDGE_INFO, "total_correct=%d\n", json_correct->valueint);
		correct = json_correct->valueint;
		submission->solution.failcase = total_testcases - correct;
    } 

	char *input = NULL;
	char *code_output = NULL;
	char *expected_output = NULL;

	if (submission->solution.verdictId != V_AC && submission->solution.verdictId != V_CE) {		
		cJSON *json_input = cJSON_GetObjectItem(json, "last_testcase");
		if (json_input != NULL &&  json_input->type == cJSON_String && json_input->valuestring != NULL) {
			write_log(JUDGE_INFO, "last_testcase: %s\n", json_input->valuestring);
			input = json_input->valuestring;
		}

		cJSON *json_code_output = cJSON_GetObjectItem(json, "code_output");
		if (json_code_output != NULL &&  json_code_output->type == cJSON_String && json_code_output->valuestring != NULL) {
			write_log(JUDGE_INFO, "code_output: %s\n", json_code_output->valuestring);
			code_output = json_code_output->valuestring;
		}
		
		cJSON *json_expected_output = cJSON_GetObjectItem(json, "expected_output");
		if (json_expected_output != NULL &&  json_expected_output->type == cJSON_String && json_expected_output->valuestring != NULL) {
			write_log(JUDGE_INFO, "expected_output: %s\n", json_expected_output->valuestring);
			expected_output = json_expected_output->valuestring;
		}
	}

	leetcode_solution_log(submission->judge_log_filename, 
						  submission->solution.verdictId,
						  submission->solution.memory_used, 
						  submission->solution.time_used,
						  correct, total_testcases, input, code_output, expected_output);

	cJSON_Delete(json);

	return 0;
}

int leetcode_vjudge(JUDGE_SUBMISSION_S *submisssion)
{
	return leetcode_python(submisssion);
}

int leetcode_test()
{
	JUDGE_SUBMISSION_S submisssion = {0};
	strcpy(submisssion.problem.virtualPID, "3");
	strcpy(submisssion.sourcePath, "D:\\code\\leetcode\\solution.c");
	strcpy(submisssion.DebugFile, "D:\\code\\leetcode\\compile.txt");

	leetcode_python(&submisssion);
	return 0;
}

#endif /* #if (OS_YES == OSP_MODULE_JUDGE) */