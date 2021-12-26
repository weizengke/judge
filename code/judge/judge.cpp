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

#include "judge/include/judge_inc.h"


#if (OS_YES == OSP_MODULE_JUDGE)

using namespace std;

char g_judge_ini_cfg_path[] = STARTUP_CFG;
char g_judge_work_path[MAX_PATH];
char g_judge_log_path[MAX_PATH];
char g_judge_testcase_path[MAX_PATH];
int g_judge_mode = JUDGE_MODE_ACM;
int g_judge_timer_enable = OS_NO;
int g_judge_auto_detect_num = JUDGE_AUTO_NUM_DEFAULT;
int g_judge_auto_detect_interval = JUDGE_AUTO_INTERVAL_DEFAULT;
time_t g_judge_last_judgetime = 0;
int g_judge_ignore_extra_space_enable = OS_NO;
int g_judge_contest_colect_interval = 30;
int g_judge_api_hook_enable = OS_YES;
int g_judge_log_buffsize = 204800;
queue <JUDGE_REQ_MSG_S> g_judge_queue; /* 全局队列 */

mutex_t hJudgeSem;

void judge_sem_create()
{
	hJudgeSem = mutex_create("JUDGE_SEM");
}
void judge_sem_p()
{
	(void)mutex_lock(hJudgeSem);
}

void judge_sem_v()
{
	(void)mutex_unlock(hJudgeSem);
}

void judge_solution_init_path(JUDGE_SUBMISSION_S *psubmission)
{
	char keyname[100]={0};
	sprintf(keyname,"Language%d", psubmission->solution.languageId);

	util_ini_get_string("Language",keyname,"",psubmission->languageName, sizeof(psubmission->languageName), g_judge_ini_cfg_path);
	util_ini_get_string("LanguageExt",psubmission->languageName,"",psubmission->languageExt, sizeof(psubmission->languageExt),g_judge_ini_cfg_path);
	util_ini_get_string("LanguageExe",psubmission->languageName,"",psubmission->languageExe,sizeof(psubmission->languageExe),g_judge_ini_cfg_path);
	util_ini_get_string("CompileCmd",psubmission->languageName,"",psubmission->compileCmd,sizeof(psubmission->compileCmd),g_judge_ini_cfg_path);
	util_ini_get_string("RunCmd",psubmission->languageName,"",psubmission->runCmd,sizeof(psubmission->runCmd),g_judge_ini_cfg_path);
	util_ini_get_string("SourcePath",psubmission->languageName,"",psubmission->sourcePath,sizeof(psubmission->sourcePath),g_judge_ini_cfg_path);
	util_ini_get_string("ExePath",psubmission->languageName,"",psubmission->exePath,sizeof(psubmission->exePath),g_judge_ini_cfg_path);

	psubmission->isTranscoding=util_ini_get_int("Transcoding",psubmission->languageName,0,g_judge_ini_cfg_path);
	psubmission->limitIndex=util_ini_get_int("TimeLimit",psubmission->languageName,1,g_judge_ini_cfg_path);
	psubmission->nProcessLimit=util_ini_get_int("ProcessLimit",psubmission->languageName,1,g_judge_ini_cfg_path);

	char buf[128];
	sprintf(buf, "%s", psubmission->sessionId);
	string name = buf;
	string compile_string=psubmission->compileCmd;
	replace_all_distinct(compile_string,"%PATH%",g_judge_work_path);
	replace_all_distinct(compile_string,"%SUBPATH%",psubmission->subPath);
	replace_all_distinct(compile_string,"%NAME%",name);
	replace_all_distinct(compile_string,"%EXT%",psubmission->languageExt);
	replace_all_distinct(compile_string,"%EXE%",psubmission->languageExe);
	strcpy(psubmission->compileCmd,compile_string.c_str());

	string runcmd_string=psubmission->runCmd;
	replace_all_distinct(runcmd_string,"%PATH%",g_judge_work_path);
	replace_all_distinct(runcmd_string,"%SUBPATH%",psubmission->subPath);
	replace_all_distinct(runcmd_string,"%NAME%",name);
	replace_all_distinct(runcmd_string,"%EXT%",psubmission->languageExt);
	replace_all_distinct(runcmd_string,"%EXE%",psubmission->languageExe);
	strcpy(psubmission->runCmd,runcmd_string.c_str());

	string sourcepath_string=psubmission->sourcePath;
	replace_all_distinct(sourcepath_string,"%PATH%",g_judge_work_path);
	replace_all_distinct(sourcepath_string,"%SUBPATH%",psubmission->subPath);
	replace_all_distinct(sourcepath_string,"%NAME%",name);
	replace_all_distinct(sourcepath_string,"%EXT%",psubmission->languageExt);
	strcpy(psubmission->sourcePath,sourcepath_string.c_str());

	string exepath_string=psubmission->exePath;
	replace_all_distinct(exepath_string,"%PATH%",g_judge_work_path);
	replace_all_distinct(exepath_string,"%SUBPATH%",psubmission->subPath);
	replace_all_distinct(exepath_string,"%NAME%",name);
	replace_all_distinct(exepath_string,"%EXE%",psubmission->languageExe);
	strcpy(psubmission->exePath,exepath_string.c_str());	

	sprintf(psubmission->DebugFile,"%s%s%s.txt",g_judge_work_path,psubmission->subPath,name.c_str());  /* debug文件路径*/
	sprintf(psubmission->ErrorFile,"%s%s%s_re.txt",g_judge_work_path,psubmission->subPath,name.c_str());  /* re文件路径*/

	if ((file_access(g_judge_log_path, 0 )) == -1) {
		create_directory(g_judge_log_path);
	}

	sprintf(psubmission->judge_log_filename,"%sjudge-log-%s.log",g_judge_log_path,psubmission->sessionId);
	sprintf(psubmission->judge_log_test_json,"%sjudge-log-%s.json",g_judge_log_path,psubmission->sessionId);

	write_log(JUDGE_INFO, "Judge init path ok. "
		"(sessionId=%s, languageId=%d, sourcePath=%s, exePath=%s, compileCmd=%s, runCmd=%s)", 
		psubmission->sessionId,
		psubmission->solution.languageId,
		psubmission->sourcePath,
		psubmission->exePath,
		psubmission->compileCmd,
		psubmission->runCmd);
}

void judge_solution_init_submition(JUDGE_SUBMISSION_S *submission)
{
	submission->solution.verdictId = V_AC;
	submission->solution.contestId = 0;
	submission->solution.time_used = 0;
	submission->solution.memory_used = 0;
	submission->solution.time_cur = 0;
	submission->solution.memory_cur = 0;
	submission->solution.testcase = 0;
	submission->solution.reJudge = 0;
	submission->problem.time_limit = 5000;
	submission->problem.memory_limit = 65535;
	submission->problem.isVirtualJudge = OS_NO;
	submission->isTranscoding = 0;
	submission->limitIndex = 1;
	submission->nProcessLimit = 1;
	submission->dwProStatusCode = 0;
	submission->hInputFile = INVALID_HANDLE_VALUE;
	submission->hOutputFile = INVALID_HANDLE_VALUE;
	submission->judge_result_json = 0;

	if( (file_access(g_judge_work_path, 0 )) == -1 ) {
		create_directory(g_judge_work_path);
	}

	time_t timep;
	time(&timep);
	srand((int)time(0)*3);
	submission->ulSeed = timep + rand();
	sprintf(submission->subPath, "%s_%u\/", submission->sessionId, submission->ulSeed);

	char fullPath[1024] = {0};
	sprintf(fullPath, "%s%s", g_judge_work_path, submission->subPath);
	while ((file_access(fullPath, 0 )) != -1 ) {
		write_log(JUDGE_INFO,"Gernerate another Seed...(%u)", submission->ulSeed);
		Sleep(10);
		submission->ulSeed = timep + rand();
		sprintf(submission->subPath, "%s_%u\/", submission->sessionId, submission->ulSeed);
		sprintf(fullPath, "%s%s", g_judge_work_path, submission->subPath);
	}

	create_directory(fullPath);
	strncpy(submission->workPath, fullPath, sizeof(submission->workPath));
}

#if (OS_YES == OSP_MODULE_JUDGE_LOCAL)

int judge_special_judge(JUDGE_SUBMISSION_S *submission)
{
	char cmd[MAX_PATH] = {0};

	sprintf(cmd,"%s\/%d\/spj_%d.exe %s %s",
			g_judge_testcase_path,
			submission->problem.problemId,
			submission->problem.problemId,
			submission->inFileName, 
			submission->outFileName);

	int ret = system(cmd);
	if(ret == 1) {
		/* success */
		return 1;
	}

	return 0;
}

void judge_solution_answer_check(JUDGE_SUBMISSION_S *submission)
{
	if (submission->mode != JUDGE_SUBMIT_MODE) {
		return;
	}

	//spj
	if (submission->problem.isSpecialJudge == 1) {
		if (1 == judge_special_judge(submission)) {
			submission->solution.verdictId = V_AC;
		} else {
			submission->solution.verdictId = V_WA;
		}
	} else {
		int verdict_ = compare(submission->outFileName,submission->stdOutFileName);
		submission->solution.verdictId = verdict_;
		if (OS_YES == g_judge_ignore_extra_space_enable) {
			/* 使能忽略多余空格，切ac */
			if (V_PE == verdict_) {
				submission->solution.verdictId = V_AC;
			}
		}
	}
}

void judge_solution_result_submit_log_upload(JUDGE_SUBMISSION_S *submission)
{
	if (g_judge_upload_log_ftp_enable == OS_YES) {
		char judge_log_filename[MAX_PATH] = {0};
		sprintf(judge_log_filename, "judge-log-%s.log", submission->sessionId);
		FTPC_PUB_Upload(g_judge_upload_log_ftp_ip, g_judge_upload_log_ftp_port,
						g_judge_upload_log_ftp_user, g_judge_upload_log_ftp_pwd,
						submission->judge_log_filename, judge_log_filename);
	}
}

void judge_solution_testcase_retult_to_file(JUDGE_SUBMISSION_S *submission)
{	
	char buf[40960] = {0};
	
	write_log(JUDGE_INFO,"Run testcase %d ok. "
				"(sessionId=%s, verdictId=%s, timeused=%d(%d)ms, time_limit=%dms, "
				"memoryused=%d(%d)kb, memory_limit=%dkb, errorcode=%u, inFileName=%s, "
				"outFileName=%s, stdInFileName=%s, stdOutFileName=%s",
				submission->solution.testcase,
				submission->sessionId, 
				VERDICT_NAME[submission->solution.verdictId],
				submission->solution.time_cur, submission->solution.time_used,
				submission->problem.time_limit,
				submission->solution.memory_cur, submission->solution.memory_used,
				submission->problem.memory_limit,
				submission->dwProStatusCode,
				submission->inFileName, submission->outFileName,
				submission->stdInFileName, submission->stdOutFileName);

	(void)util_fwrite(submission->judge_log_filename,
				"Test: #%d, time: %d ms, memory: %d kb, exit code: %d,verdict: %s",
				submission->solution.testcase, 
				submission->solution.time_cur - submission->solution.time_cur%10,
				submission->solution.memory_cur, 
				submission->dwProStatusCode,
				VERDICT_NAME[submission->solution.verdictId]);

	memset(buf,0,sizeof(buf));
	(void)util_fread(submission->stdInFileName, buf, sizeof(buf));
	(void)util_fwrite(submission->judge_log_filename,"\nInput\n");
	(void)util_fwrite(submission->judge_log_filename,buf);

	memset(buf,0,sizeof(buf));
	(void)util_fread(submission->outFileName, buf, sizeof(buf));
	(void)util_fwrite(submission->judge_log_filename,"\nOutput\n");
	(void)util_fwrite(submission->judge_log_filename,buf);

	memset(buf,0,sizeof(buf));
	(void)util_fread(submission->stdOutFileName, buf, sizeof(buf));
	(void)util_fwrite(submission->judge_log_filename,"\nAnswer\n");
	(void)util_fwrite(submission->judge_log_filename,buf);

	if (submission->solution.verdictId == V_RE) {
		memset(buf,0,sizeof(buf));
		(void)util_fread(submission->ErrorFile, buf, sizeof(buf));
		(void)util_fwrite(submission->judge_log_filename,"\nRuntime Error\n");
		(void)util_fwrite(submission->judge_log_filename,buf);
	}

	(void)util_fwrite(submission->judge_log_filename,"\n------------------------------------------------------------------\n");
}

void judge_solution_testcase_result_to_json(JUDGE_SUBMISSION_S *submission)
{
	char *json_buf = NULL;
	JUDGE_TESTCASE_S *judge_case = NULL;
	cJSON *json = cJSON_CreateObject();
	cJSON *array = cJSON_CreateArray();
	
	if (json == NULL || array == NULL) {
		return;
	}

	cJSON_AddNumberToObject(json,"solutionId", submission->solution.solutionId);
	cJSON_AddNumberToObject(json,"problemId",submission->solution.problemId);
	cJSON_AddStringToObject(json,"username",submission->solution.username);
	cJSON_AddStringToObject(json,"language",submission->languageName);
	
	LL_FOREACH (submission->testcases_result, judge_case) {
		cJSON *testcase = cJSON_CreateObject();
		if (NULL != testcase) {
			cJSON_AddItemToArray(array, testcase);
			cJSON_AddNumberToObject(testcase, "case", judge_case->case_id);
			cJSON_AddStringToObject(testcase, "verdict", VERDICT_NAME[judge_case->verdict]);
			cJSON_AddNumberToObject(testcase, "timeused", judge_case->time_used);
			cJSON_AddNumberToObject(testcase, "memused", judge_case->memory_used);
		}
	}

	cJSON_AddNumberToObject(json,"testcases", submission->solution.testcase);
	cJSON_AddItemToObject(json,"cases", array);

	json_buf = cJSON_Print(json);
	submission->judge_result_json = json_buf; /* json_buf free outside */
	SQL_updateSolutionJsonResult(submission->solution.solutionId, submission->judge_result_json);

	cJSON_Delete(json);

	return ;
}

void judge_solution_testcase_result_save(JUDGE_SUBMISSION_S *submission)
{
	JUDGE_TESTCASE_S *detail = (JUDGE_TESTCASE_S *)malloc(sizeof(JUDGE_TESTCASE_S));
	if (detail != NULL) {
		detail->case_id = submission->solution.testcase;
		detail->time_used = submission->solution.time_cur - submission->solution.time_cur%10;
		detail->memory_used = submission->solution.memory_cur;
		detail->verdict = submission->solution.verdictId;
		LL_APPEND(submission->testcases_result, detail);
	}
}

void judge_solution_testcase_result_free(JUDGE_SUBMISSION_S *submission)
{
	JUDGE_TESTCASE_S *judge_case = NULL;
	JUDGE_TESTCASE_S *judge_case_tmp = NULL;

    LL_FOREACH_SAFE(submission->testcases_result, judge_case, judge_case_tmp) {
      LL_DELETE(submission->testcases_result, judge_case);
      free(judge_case);
    }
}

int judge_request_json_parse(JUDGE_SUBMISSION_S *submission) {
	cJSON *json = submission->judge_request_json;
	if (json == NULL) {
		return OS_OK;
	}

	cJSON *json_session_id = cJSON_GetObjectItem(json, "session_id");
    if(json_session_id != NULL && json_session_id->type == cJSON_String) {
        write_log(JUDGE_INFO, "session_id=%s\n", json_session_id->valuestring);
        strncpy(submission->sessionId, json_session_id->valuestring, sizeof(submission->sessionId) - 1);
    } else {
		write_log(JUDGE_ERROR, "session_id parse failed\n");
		return OS_ERR;
	}

	cJSON *json_problem_id = cJSON_GetObjectItem(json, "problem_id");
    if(json_problem_id != NULL &&  json_problem_id->type == cJSON_String) {
        write_log(JUDGE_INFO, "problem_id=%s\n", json_problem_id->valuestring);
		submission->solution.problemId = atoi(json_problem_id->valuestring);
    } else {
		write_log(JUDGE_ERROR, "problem_id parse failed\n");
	}

	cJSON *json_language_id = cJSON_GetObjectItem(json, "language_id");
    if(json_language_id != NULL &&  json_language_id->type == cJSON_String) {
        write_log(JUDGE_INFO, "language_id=%s\n", json_language_id->valuestring);
		submission->solution.languageId = atoi(json_language_id->valuestring);
    } else {
		write_log(JUDGE_ERROR, "language_id parse failed\n");
	}

	submission->mode = JUDGE_TEST_MODE;/* 默认test，兼容老版本web */
	cJSON *json_mode = cJSON_GetObjectItem(json, "mode");
    if(json_mode != NULL && json_mode->type == cJSON_String) {
        write_log(JUDGE_INFO, "output_file=%s\n", json_mode->valuestring);
		if (strcmp(json_mode->valuestring, "submit") == 0) {
			submission->mode = JUDGE_SUBMIT_MODE;
			submission->solution.solutionId = atoi(submission->sessionId);
		} else if (strcmp(json_mode->valuestring, "test") == 0) {
			submission->mode = JUDGE_TEST_MODE;
		}
    } else {
		write_log(JUDGE_ERROR, "mode parse failed\n");
	}

	return OS_OK;
}

void judge_request_json_get_code(JUDGE_SUBMISSION_S *submission) {
	cJSON *json = submission->judge_request_json;
	if (json == NULL) {
		return;
	}

	cJSON *json_code = cJSON_GetObjectItem(json, "code");
    if(json_code != NULL && json_code->type == cJSON_String) {
        FILE *fp = fopen(submission->sourcePath, "w");
        if (fp != NULL) {
            fprintf(fp, "%s", json_code->valuestring);
            fclose(fp);
        }
    }
}

void judge_request_json_get_input(JUDGE_SUBMISSION_S *submission) {
	cJSON *json = submission->judge_request_json;
	if (json == NULL) {
		return;
	}

	cJSON *json_input_file = cJSON_GetObjectItem(json, "input_file");
    if(json_input_file != NULL && json_input_file->type == cJSON_String) {
        write_log(JUDGE_INFO, "input_file parse:%s", json_input_file->valuestring);
		sprintf(submission->inFileName, "%s%s", submission->workPath, json_input_file->valuestring);
		submission->inFile = OS_YES;
    } 
	
	if (submission->mode == JUDGE_TEST_MODE) {
		cJSON *json_input = cJSON_GetObjectItem(json, "input");
		if(json_input != NULL && json_input->type == cJSON_String) {
			if (strlen(submission->inFileName) == 0) {
				sprintf(submission->inFileName, "%s%s.in", submission->workPath, submission->sessionId);
			}
			write_log(JUDGE_INFO, "input parse:%s", json_input->valuestring);
			FILE *fp = fopen(submission->inFileName, "w");
			if (fp != NULL) {
				fprintf(fp, "%s", json_input->valuestring);
				fclose(fp);
			}
		}
	}
}

void judge_request_json_get_output(JUDGE_SUBMISSION_S *submission) {
	cJSON *json = submission->judge_request_json;
	if (json == NULL) {
		return;
	}

	cJSON *json_output_file = cJSON_GetObjectItem(json, "output_file");
    if(json_output_file != NULL && json_output_file->type == cJSON_String) {
        write_log(JUDGE_INFO, "output_file parse:%s\n", json_output_file->valuestring);
        sprintf(submission->outFileName, "%s%s", submission->workPath, json_output_file->valuestring);
		submission->outFile = OS_YES;
    }

	if (submission->mode == JUDGE_TEST_MODE) {
		if (strlen(submission->outFileName) == 0) {
			sprintf(submission->outFileName, "%s%s.out", submission->workPath, submission->sessionId);
		}
	}
}

void judge_solution_result_test_json_upload(JUDGE_SUBMISSION_S *submission)
{
	if (g_judge_upload_log_ftp_enable == OS_YES) {
		char judge_log_test_json[MAX_PATH] = {0};
		sprintf(judge_log_test_json, "judge-log-%s.json", submission->sessionId);
		FTPC_PUB_Upload(g_judge_upload_log_ftp_ip, g_judge_upload_log_ftp_port,
						g_judge_upload_log_ftp_user, g_judge_upload_log_ftp_pwd,
						submission->judge_log_test_json, judge_log_test_json);
	}
}
/* 
   {
       "session_id: "asas121dd2d2",
       "problem_id": "1000",
       "language_id", "1",
	   "verdict": "compile error",
	   "compile_error": "ssssssss",
	   "time"："11",
	   "memory"："11",
       "input": "sssss",
       "output": "aasasa"
   }
*/
int judge_solution_result_test_json_make(JUDGE_SUBMISSION_S *submission) {
	char buff[10240] = {0};	
	cJSON *json = cJSON_CreateObject();
	if (json == NULL) {
		return OS_ERR;
	}

	cJSON_AddStringToObject(json,"session_id", submission->sessionId);
	cJSON_AddNumberToObject(json,"problem_id",submission->solution.problemId);
	cJSON_AddNumberToObject(json,"language_id",submission->solution.languageId);
	cJSON_AddStringToObject(json,"verdict", VERDICT_NAME[submission->solution.verdictId]);
	cJSON_AddNumberToObject(json, "time", submission->solution.time_used);
	cJSON_AddNumberToObject(json, "memory", submission->solution.memory_used);

	memset(buff, 0, sizeof(buff));
	util_fread(submission->inFileName, buff, sizeof(buff) - 1);
	write_log(JUDGE_INFO, "inFileName=%s, input=%s, %d\n", submission->inFileName, buff, strlen(buff));
	cJSON_AddStringToObject(json, "input", buff);

	if (submission->solution.verdictId == V_CE) {
		memset(buff, 0, sizeof(buff));		
		util_fread(submission->DebugFile, buff, sizeof(buff) - 1);
		cJSON_AddStringToObject(json, "compile_error", buff);
	} else {
		memset(buff, 0, sizeof(buff));
		util_fread(submission->outFileName, buff, sizeof(buff) - 1);
		cJSON_AddStringToObject(json, "output", buff);
	}

	char *json_buf = cJSON_Print(json);
	if (json_buf != NULL) {
		FILE *fd = fopen(submission->judge_log_test_json, "w");
		if (fd) {
			fprintf(fd, "%s", json_buf);
			fclose(fd);
		}
		free(json_buf);
	}

	cJSON_Delete(json);

	return OS_OK;
}

int judge_solution_testcase_prepare(JUDGE_SUBMISSION_S *submission, char *testcase)
{
	memset(submission->inFileName, 0, sizeof(submission->inFileName));
	memset(submission->outFileName, 0, sizeof(submission->outFileName));

	judge_request_json_get_input(submission);
	judge_request_json_get_output(submission);

	if (strlen(submission->inFileName) == 0) {
		sprintf(submission->inFileName, "%s%s.in", submission->workPath, testcase);
	}
	if (strlen(submission->outFileName) == 0) {
		sprintf(submission->outFileName,"%s%s.out", submission->workPath, testcase);
	}
	(void)util_remove(submission->outFileName);

	if (submission->mode == JUDGE_SUBMIT_MODE) {
		sprintf(submission->stdOutFileName,"%s/%d/%s.out",
				g_judge_testcase_path, submission->problem.problemId, testcase);
		sprintf(submission->stdInFileName, "%s/%d/%s.in",
				g_judge_testcase_path, submission->problem.problemId, testcase);
		CopyFile(submission->stdInFileName, submission->inFileName, 0);
		if (submission->problem.isSpecialJudge != 1) {
			if ((file_access(submission->stdOutFileName, 0 )) == -1) {
				write_log(JUDGE_ERROR,"stdOutFileName(%s) is not found.", submission->stdOutFileName);
				return OS_ERR;
			}
		}
		if ((file_access(submission->inFileName, 0 )) == -1) {
			write_log(JUDGE_ERROR,"inFileName(%s) is not found.", submission->inFileName);
			return OS_ERR;
		}
	}

	return OS_OK;
}

void judge_solution_testcase_teardown(JUDGE_SUBMISSION_S *submission)
{
	/* save testcase result */
	if (JUDGE_MODE_OI == g_judge_mode) {
		judge_solution_testcase_result_save(submission);
		judge_solution_testcase_result_to_json(submission);
	}

	/* save testcase result to judge-log */
	judge_solution_testcase_retult_to_file(submission);
}

void judge_solution_suitecase_teardown(JUDGE_SUBMISSION_S *submission)
{
	if (JUDGE_MODE_OI == g_judge_mode) {
		if (submission->solution.failcase > 0 && submission->solution.verdictId != V_SE) {
			submission->solution.verdictId = V_WA;
		}

		/* result to sjon */
		judge_solution_testcase_result_to_json(submission);	
	}

	judge_solution_testcase_result_free(submission);	

	write_log(JUDGE_INFO,"judge solution %s run finish.", submission->sessionId);
}

void judge_solution_check(JUDGE_SUBMISSION_S *submission)
{
	/* get memory info, continue to check exit-code&time */
	judge_solution_memory_check(submission);

	/* check exit code */
	judge_solution_exit_check(submission);
	if (submission->solution.verdictId == V_RE) {
		return;
	}

	/* get time info */
	judge_solution_time_check(submission);
	if (submission->solution.verdictId == V_TLE) {
		return;
	}

	/* check runtime err file */
	char buf[32] = {0};
	(void)util_fread(submission->ErrorFile, buf, sizeof(buf) - 1);
	if (strlen(buf) > 0) {
		submission->solution.verdictId = V_RE;
		return;
	}

	if (submission->solution.verdictId != V_AC) {
		return;
	}

	/* check answer */
	judge_solution_answer_check(submission);
}

int judge_solution(JUDGE_SUBMISSION_S *submission)
{
	int idx, caseNum = 0;
	char testcases[JUDGE_MAX_CASE][UTIL_MAX_FNAME] = {0};
	caseNum = judge_get_testcases(submission, testcases);

	(VOID)util_freset(submission->judge_log_filename);

	write_log(JUDGE_INFO, "run local solution. (problemId=%d, caseNum=%u).",
						   submission->problem.problemId, caseNum);
	
	for (idx = 0; idx < caseNum; idx++) {		
		submission->solution.testcase++;
		submission->solution.verdictId = V_AC;

		/* testcase init */
		if (judge_solution_testcase_prepare(submission, testcases[idx]) != OS_OK) {
			submission->solution.verdictId = V_SE;
			goto l;
		}

		if (judge_solution_run(submission) != OS_OK) {
			submission->solution.verdictId = V_SE;
		}

		judge_solution_check(submission);
l:
		judge_solution_testcase_teardown(submission);

		if (submission->solution.verdictId != V_AC) {
			submission->solution.failcase++;
			if (JUDGE_MODE_ACM == g_judge_mode) {
				break;
			}
		}
	}

	judge_solution_suitecase_teardown(submission);
	
	return OS_OK;
}

int judge_compile(JUDGE_SUBMISSION_S *submission)
{
	Judge_Debug(DEBUG_TYPE_INFO, "compile cmd=%s", submission->compileCmd);
	
    /* no need to compile */
    if (strcmp(submission->compileCmd, "NULL") == 0) {
		write_log(JUDGE_INFO, "no need to compile.");
        return OS_OK;
    }

	/* verdict to running in compiling */
	if (submission->mode == JUDGE_SUBMIT_MODE) {
		JUDGE_SUBMISSION_S submission_ = {0};
		memcpy(&submission_, submission, sizeof(JUDGE_SUBMISSION_S));
		submission_.solution.verdictId = V_C;
		SQL_updateSolution(&submission_);
	}

	(void)judge_compile_run(submission);

    if ((file_access(submission->exePath, 0)) == -1 ) {
		write_log(JUDGE_ERROR, "compile error.");
        submission->solution.verdictId = V_CE;
        return OS_ERR;
    }

	return OS_OK;
}

int judge_solution_get_problem(JUDGE_SUBMISSION_S *submission)
{
	submission->problem.problemId = submission->solution.problemId;

	int ret = SQL_getProblemInfo(&(submission->problem));	

	submission->problem.time_limit *= submission->limitIndex;
	submission->problem.memory_limit *= submission->limitIndex;
	
	if (submission->mode == JUDGE_TEST_MODE) {
		/* test mode force return ok, for vjudge problem test submission */
		submission->problem.isVirtualJudge = OS_NO;
		return OS_OK;
	}

	if (ret != OS_OK) {
		Judge_Debug(DEBUG_TYPE_ERROR, "getProblemInfo failed.(solutionId=%d)", submission->solution.solutionId);
		write_log(JUDGE_INFO,"getProblemInfo failed.(solutionId=%d)", submission->solution.solutionId);
		return OS_ERR;
	}

	return OS_OK;
}

void judge_solution_result_compile_err_to_file(JUDGE_SUBMISSION_S *submission)
{
	if (submission->solution.verdictId != V_CE) {
		return;
	}

	util_freset(submission->judge_log_filename);
	util_fwrite(submission->judge_log_filename, 
				"Test: #1, time: 0 ms, memory: 0 kb, exit code: 0,verdict: %s\n",
				VERDICT_NAME[submission->solution.verdictId]);
				
	char *buf = (char*)malloc(8193);
	if (buf != NULL) {
		memset(buf, 0, 8193);
		util_fread(submission->DebugFile, buf, 8192);
		util_fwrite(submission->judge_log_filename, "%s", buf);
	}					
	JUDGE_FREE(buf);
}

void judge_solution_result_upload(JUDGE_SUBMISSION_S *submission)
{
	/* upload judge-result to ftp server */
	if (submission->mode == JUDGE_TEST_MODE) {
		/* test mode result to json file */
		judge_solution_result_test_json_make(submission);
		judge_solution_result_test_json_upload(submission);
	} else {
		/* check if compile error, and save to judge-log */
		judge_solution_result_compile_err_to_file(submission);
		/* upload judge-log to server */
		judge_solution_result_submit_log_upload(submission);
	}
}
void judge_solution_result_to_db(JUDGE_SUBMISSION_S *submission)
{
	/* update sql data */
	if (submission->mode == JUDGE_SUBMIT_MODE) {
		write_log(JUDGE_INFO, "Judge solution result to db. (sessionId=%s)", submission->sessionId);
		SQL_updateCompileInfo(submission);
		SQL_updateSolution(submission);
		SQL_updateProblem(submission->solution.problemId);
		SQL_updateUser(submission->solution.username);
		SQL_updateContest(submission);
	}
}

void judge_solution_result_show(JUDGE_SUBMISSION_S *submission)
{
	if (CMD_VTY_CFM_ID == submission->vty){
		return;
	}

	string time_string_;
	(VOID)util_time_to_string(time_string_, submission->solution.submitDate);

	vty_printf(submission->vty,
		"\r\n -----------------------"
		"\r\n     *Judge verdict*"
		"\r\n -----------------------"
		"\r\n SolutionId   : %3s"
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
		submission->sessionId,
		submission->problem.problemId,
		submission->languageName,
		submission->solution.testcase,
		submission->solution.failcase,
		submission->solution.time_used - submission->solution.time_used%10,
		submission->solution.memory_used,
		submission->dwProStatusCode,
		VERDICT_NAME[submission->solution.verdictId],
		time_string_.c_str(), submission->solution.username,
		(submission->judge_result_json!=0)?submission->judge_result_json:"-");
}

int judge_local(JUDGE_SUBMISSION_S *submission)
{
    write_log(JUDGE_INFO,"Judge local start. (sessionId=%s)", submission->sessionId);

    if(judge_compile(submission) != OS_OK) {
		return OS_ERR;
    } 

	(void)judge_solution(submission);

    write_log(JUDGE_INFO,"Judge local end. (sessionId=%s)", submission->sessionId);

    return OS_OK;
}
#endif

#if (OS_YES == OSP_MODULE_JUDGE_VJUDGE)
int judge_is_vjudge_enable()
{
	return g_vjudge_enable;
}

int judge_sendto_remote_test(char *jsonMsg, int port, char *ip)
{

	socket_t sock;
	
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET)
	{
		Judge_Debug(DEBUG_TYPE_ERROR, "judge_sendto_remote socket error");
		return OS_ERR;
	}

	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	servAddr.sin_addr.s_addr =inet_addr(ip);

	if(connect(sock,(sockaddr*)&servAddr,sizeof(servAddr))==SOCKET_ERROR)
	{
		Judge_Debug(DEBUG_TYPE_ERROR, "judge_sendto_remote connect error");
		closesocket(sock);
		return OS_ERR;
	}

	char *buff = (char*)malloc(100 + strlen(jsonMsg));
	memset(buff, 0, 100 + strlen(jsonMsg));
	sprintf(buff, "abcddcba0002%04x%s", strlen(jsonMsg), jsonMsg);

	Judge_Debug(DEBUG_TYPE_INFO, "judge_sendto_remote_test '%s'", buff);

	send(sock,(const char*)buff, strlen(buff) + 1, 0);

	closesocket(sock);
	
	return OS_OK;
}

int judge_sendto_remote(int solutionId, int port,char *ip)
{

	socket_t sClient_hdu;
	char buff[1024] = {0};
	char buffCmd[128] = {0};

    sClient_hdu = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sClient_hdu == INVALID_SOCKET)
	{
		write_log(JUDGE_ERROR, "judge_sendto_remote socket error");
		return OS_ERR;
	}

	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	servAddr.sin_addr.s_addr =inet_addr(ip);

	if(connect(sClient_hdu,(sockaddr*)&servAddr,sizeof(servAddr))==SOCKET_ERROR)
	{
		write_log(JUDGE_ERROR, "judge_sendto_remote connect error");
		closesocket(sClient_hdu);
		return OS_ERR;
	}

	sprintf(buffCmd, "judge solution %u", solutionId);
	sprintf(buff, "abcddcba0001%04x%s", strlen(buffCmd), buffCmd);

	Judge_Debug(DEBUG_TYPE_INFO, "judge_sendto_remote '%s'", buff);

	send(sClient_hdu,(const char*)buff, sizeof(buff),0);

	closesocket(sClient_hdu);

	return OS_OK;
}

int judge_vjudge_hdu(JUDGE_SUBMISSION_S *submission)
{
	int ret = OS_OK;

	Judge_Debug(DEBUG_TYPE_FUNC, "virtual-judge HDU.(domain=%s, vjudge_enable=%u,remote_enable=%d)",
								 hdu_domain, hdu_vjudge_enable, hdu_remote_enable);

	if (hdu_vjudge_enable == OS_NO) {
		write_log(JUDGE_ERROR, "Error: hdu-judge is not enable.");
		return OS_ERR;
	}

	if (OS_YES == hdu_remote_enable) {
		write_log(JUDGE_INFO, "Send to remote judger(%s:%d).", hdu_judgerIP, hdu_sockport);
		ret = judge_sendto_remote(submission->solution.solutionId, hdu_sockport, hdu_judgerIP);
		if (OS_OK == ret) {
			/* virdict置queue , 由远程judger继续执行 */
			submission->solution.verdictId = V_Q;
		}

		return ret;
	}

	/* local vjudge */
	#if (OS_YES == OSP_MODULE_JUDGE_VJUDGE)
	ret = HDU_VJudge(submission);
	#endif

	return ret;
}

int judge_vjudge_guet3(JUDGE_SUBMISSION_S *submission)
{
	int ret = OS_OK;

	Judge_Debug(DEBUG_TYPE_FUNC, "virtual-judge GUET_DEPT3.(vjudge_enable=%u,remote_anable=%d)",
								  guet_vjudge_enable, guet_remote_enable);

	if (guet_vjudge_enable == OS_NO) {
		write_log(JUDGE_ERROR, "Error: guet-judge is not enable.");
		return OS_ERR;
	}

	if (OS_YES == guet_remote_enable) {
		write_log(JUDGE_INFO, "Send to remote judger(%s:%d).", guet_judgerIP, guet_sockport);
		ret = judge_sendto_remote(submission->solution.solutionId, guet_sockport, guet_judgerIP);
		if (OS_OK == ret) {
			/* virdict置quieue , 由远程judger继续执行 */
			submission->solution.verdictId = V_Q;
		}

		return ret;
	}

	/* local vjudge */
	#if(JUDGE_VIRTUAL == VOS_YES)
	ret = GUET_VJudge(submission);
	#endif

	return ret;
}

int judge_vjudge_leetcode(JUDGE_SUBMISSION_S *submission)
{
	int ret = OS_OK;

	Judge_Debug(DEBUG_TYPE_FUNC, "virtual-judge LEETCODE. (vjudge_enable=%u, remote_anable=%d)",
							leetcode_vjudge_enable, leetcode_remote_enable);

	if (leetcode_vjudge_enable == OS_NO) {
		write_log(JUDGE_ERROR, "Error: leetcode-judge is not enable.");
		return OS_ERR;
	}

	if (OS_YES == leetcode_remote_enable) {
		extern char leetcode_judgerIP[20];
		extern int leetcode_sockport;
		write_log(JUDGE_INFO, "Send to remote judger(%s:%d).", leetcode_judgerIP, leetcode_sockport);
		ret = judge_sendto_remote(submission->solution.solutionId, leetcode_sockport, leetcode_judgerIP);
		if (OS_OK == ret) {
			/* virdict置quieue , 由远程judger继续执行 */
			submission->solution.verdictId = V_Q;
		}

		return ret;
	}

	/* local vjudge */
	#if(JUDGE_VIRTUAL == VOS_YES)
	ret = leetcode_vjudge(submission);
	#endif

	return ret;
}

int judge_vjudge_codeforces(JUDGE_SUBMISSION_S *submission)
{
	int ret = OS_OK;

	Judge_Debug(DEBUG_TYPE_FUNC, "virtual-judge codeforces. (codeforces_vjudge_enable=%u, codeforces_remote_enable=%d)",
							codeforces_vjudge_enable, codeforces_remote_enable);

	if (codeforces_vjudge_enable == OS_NO) {
		write_log(JUDGE_ERROR, "Error: codeforces-judge is not enable.");
		return OS_ERR;
	}

	if (OS_YES == codeforces_remote_enable) {
		extern char codeforces_judgerIP[20];
		extern int codeforces_sockport;
		write_log(JUDGE_INFO, "Send to remote judger(%s:%d).", codeforces_judgerIP, codeforces_sockport);
		ret = judge_sendto_remote(submission->solution.solutionId, codeforces_sockport, codeforces_judgerIP);
		if (OS_OK == ret) {
			/* virdict置quieue , 由远程judger继续执行 */
			submission->solution.verdictId = V_Q;
		}

		return ret;
	}

	/* local vjudge */
	#if(JUDGE_VIRTUAL == VOS_YES)
	ret = codeforces_vjudge(submission);
	#endif

	return ret;
}
int judge_vjudge(JUDGE_SUBMISSION_S *submission)
{
	write_log(JUDGE_INFO, "Start virtua-judge. (solutionId=%u, szVirJudgerName=%s)",
				submission->solution.solutionId, submission->problem.szVirJudgerName);

	if (0 == strcmp(submission->problem.szVirJudgerName,"HDU")) {
		return judge_vjudge_hdu(submission);
	}

	if (0 == strcmp(submission->problem.szVirJudgerName,"GUET_DEPT3")) {
		return judge_vjudge_guet3(submission);
	}

	if (0 == strcmp(submission->problem.szVirJudgerName,"LEETCODE")) {
		return judge_vjudge_leetcode(submission);
	}
	if (0 == strcmp(submission->problem.szVirJudgerName,"CF")) {
		return judge_vjudge_codeforces(submission);
	}
	write_log(JUDGE_INFO, "virtua-judge is not support (%s).", submission->problem.szVirJudgerName);

	return OS_ERR;
}
#endif

int judge_run(JUDGE_SUBMISSION_S *submission)
{
	int ret = OS_OK;

	if (OS_YES == submission->problem.isVirtualJudge) {
		#if (OS_YES == OSP_MODULE_JUDGE_VJUDGE)
		if (OS_YES == judge_is_vjudge_enable()) {
			ret = judge_vjudge(submission);
			if (OS_OK != ret) {
				submission->solution.verdictId = V_SK;
				write_log(JUDGE_ERROR,  "virtual-judge is fail...");
			}
		} else {
			write_log(JUDGE_ERROR, "Error: virtual-judge is not enable.");
			submission->solution.verdictId = V_SK;
		}
		#else
		submission->solution.verdictId = V_SK;
		write_log(JUDGE_ERROR, "virtua-judge is not support.");
		#endif
	} else {
		#if (OS_YES == OSP_MODULE_JUDGE_LOCAL)
		ret = judge_local(submission);
		#else
		submission->solution.verdictId = V_SK;
		write_log(JUDGE_ERROR, "local-judge is not support.");
		#endif
	}

	return ret;
}

int judge_request_parse(JUDGE_REQ_MSG_S *requst, JUDGE_SUBMISSION_S *submission)
{
	submission->vty = requst->vtyId;

	if (requst->jsonMsg != NULL) {
		submission->judge_request_json = cJSON_Parse(requst->jsonMsg);
		if (submission->judge_request_json == NULL) {
			write_log(JUDGE_INFO, "judge_request_parse, cJSON_Parse failed.");
			return OS_ERR;
		}
		
		if (OS_OK != judge_request_json_parse(submission)) {	
			cJSON_Delete(submission->judge_request_json);	
			submission->judge_request_json = NULL;
			return OS_ERR;
		}	
	} else {
		submission->solution.solutionId = requst->solutionId;
		sprintf(submission->sessionId, "%d", submission->solution.solutionId);
		submission->mode = JUDGE_SUBMIT_MODE;
	}

	return OS_OK;
}

int judge_request_init(JUDGE_SUBMISSION_S *submission)
{
	int ret = OS_OK;	

	judge_solution_init_submition(submission);
	
	if (submission->mode == JUDGE_SUBMIT_MODE) {
		int isExist = OS_NO;
		ret = SQL_getSolutionByID(submission->solution.solutionId, &(submission->solution), &isExist);
		if (OS_ERR == ret || OS_NO == isExist) {
			Judge_Debug(DEBUG_TYPE_ERROR, "No solution %d.", submission->solution.solutionId);	
			return OS_ERR;
		}

		judge_solution_init_path(submission);

		ret = SQL_getSolutionSource(submission);
		if (OS_OK != ret) {
			return OS_ERR;
		}
	} else {
		judge_solution_init_path(submission);
		judge_request_json_get_code(submission);
	}

	ret = judge_solution_get_problem(submission);
	if (OS_OK != ret) {
		return OS_ERR;
	}

	return OS_OK;
}

int judge_request_proc(JUDGE_REQ_MSG_S *request)
{
	int ret = OS_OK;	
	JUDGE_SUBMISSION_S submission = {0};

	ret = judge_request_parse(request, &submission);
	if (ret != OS_OK) {
		return ret;
	}

	write_log(JUDGE_INFO, "Start judge solution %s", submission.sessionId);

	ret = judge_request_init(&submission);
	if (ret != OS_OK) {
		submission.solution.verdictId = V_SK;
		goto l;
	}

	ret = judge_run(&submission);
	if (ret != OS_OK) {
		goto l;
	}

l:
	judge_solution_result_upload(&submission);
	judge_solution_result_to_db(&submission);
	judge_solution_result_show(&submission);

	/* free memory*/
	if (submission.judge_request_json != NULL) {
		cJSON_Delete(submission.judge_request_json);	
	}
	if (submission.judge_result_json != NULL) {
		JUDGE_FREE(submission.judge_result_json);	
	}

    //util_remove(submission.exePath);
    //util_remove(submission.DebugFile);
    //util_remove(submission.ErrorFile);
    //util_remove(submission.sourcePath);

	write_log(JUDGE_INFO, "End judge sessionId %s.", submission.sessionId);

	return OS_OK;
}

void judge_request_enqueue_test(char *jsonMsg) {
	JUDGE_REQ_MSG_S jd = {0};

	if (OS_YES == g_judge_enable) {
		char *json = (char*)malloc(strlen(jsonMsg) + 1);
		memset(json, 0, strlen(jsonMsg) + 1);
		memcpy(json, jsonMsg, strlen(jsonMsg));

		jd.solutionId = 0;
		jd.vtyId = CMD_VTY_CFM_ID;
		jd.jsonMsg = json;		
		g_judge_queue.push(jd);
	} else {
		Judge_Debug(DEBUG_TYPE_MSG, "Recieve judge test request, but judger is disable.");
	}
}

void judge_request_enqueue(int vtyId, int solutionId)
{
	JUDGE_REQ_MSG_S jd = {0};

	jd.solutionId = solutionId;
	jd.vtyId = vtyId;
	
	if (OS_YES == g_judge_enable) {
		Judge_Debug(DEBUG_TYPE_MSG, "Recieve judge request. (solution=%d, vtyId=%u).",solutionId, vtyId);
		write_log(JUDGE_INFO,"Recieve judge request. (solution=%d, vtyId=%u).",solutionId, vtyId);
		g_judge_queue.push(jd);
	} else {
		Judge_Debug(DEBUG_TYPE_MSG, "Recieve judge request, but judger is disable. (solution=%d, vtyId=%u).",solutionId, vtyId);
		write_log(JUDGE_INFO,"Recieve judge request, but judger is disable. (solution=%d, vtyId=%u).",solutionId, vtyId);

		JUDGE_SUBMISSION_S submission = {0};
		submission.solution.solutionId = solutionId;
		submission.solution.verdictId = V_SK;
		SQL_updateSolution(&submission);
	}
}

int judge_thread_msg(void *pEntry)
{
	JUDGE_REQ_MSG_S req;
	extern int judgeThreadPing;

	for (;;) {
		judge_sem_p();
		judgeThreadPing++;

		if(!g_judge_queue.empty()) {
			req = g_judge_queue.front();
			time(&g_judge_last_judgetime);
			judge_request_proc(&req);
			JUDGE_FREE(req.jsonMsg);
			g_judge_queue.pop();
		}

		judge_sem_v();
		Sleep(1000);
	}

	return 0;
}

void judge_timer_auto_detect()
{
	int i = 0;
	int n = 0;
	JUDGE_REQ_MSG_S *req = NULL;
	JUDGE_REQ_MSG_S *reqHead = NULL;
	static int tick = 0;

	if (OS_YES != g_judge_timer_enable) {
		tick = 0;
		return;
	}	

	if (0 != tick % g_judge_auto_detect_interval) {
		tick++;	
		return;
	}
	tick++;	

	/* queue not empty, rollback tick */
	if (g_judge_queue.size() != 0) {
		tick--;
		return;
	}

	req = (JUDGE_REQ_MSG_S*)malloc((g_judge_auto_detect_num + 1) * sizeof(JUDGE_REQ_MSG_S));
	if (NULL == req) {
		return ;
	}
	reqHead = req;
	SQL_getSolutionsByVerdict(V_Q, req, &n, g_judge_auto_detect_num);
	for (i = 0; i < n; i++) {
		write_log(JUDGE_INFO,"Auto detect solution. (solution=%d, n=%d).", req->solutionId, n);
		judge_request_enqueue(CMD_VTY_INVALID_ID, req->solutionId);
		req++;
	}

#if 0
	req = reqHead;
	SQL_getSolutionsByVerdict(V_SK, req, &n, g_judge_auto_detect_num);
	for (i = 0; i < n; i++) {
		judge_request_enqueue(CMD_VTY_INVALID_ID, req->solutionId);
		req++;
	}
#endif

	free(reqHead);

	return;
}

void judge_timer_check_testcase_path()
{
	static int loop = 0;

	/* 60s */
	if (0 == loop % 60) {	
		loop = 0;
		if ((file_access(g_judge_testcase_path, 0 )) == -1) {
			Judge_Debug(DEBUG_TYPE_ERROR, "Warning: Data path '%s' is not exist, "
				"please check. Use the command judge data-path <path> to configure.", g_judge_testcase_path);
			write_log(JUDGE_ERROR,"Warning: Data path '%s' is not exist, please check.", g_judge_testcase_path);
		}
	}

	loop++;
}

void judge_timer_recent_contests_collect()
{	
	static int loop = 0;
	extern int g_judge_recent_enable;
	if (OS_YES == g_judge_recent_enable){
		if (0 == loop % (60 * g_judge_contest_colect_interval)) {
			loop = 0;
			Judge_Debug(DEBUG_TYPE_INFO, "start colect contests info...");
			extern int judge_recent_generate(void *p);
			thread_create(judge_recent_generate, NULL);
			Judge_Debug(DEBUG_TYPE_INFO, "End colect contests info...");
		}

		loop++;
	}
}

int judge_thread_timer(void *pEntry)
{

	for (;;){
        Sleep(1000);

		judge_sem_p();
		judge_timer_check_testcase_path();
		judge_timer_auto_detect();
		judge_timer_recent_contests_collect();

		judge_sem_v();
	}

	return 0;
}

int judge_init()
{
	if(SQL_InitMySQL()==0) {
		write_log(JUDGE_ERROR,"Judge can not connect to MySQL(%s, %s, %s, %s, %d).",Mysql_url, Mysql_username, Mysql_password, Mysql_table, Mysql_port);
		printf("Error: Judge can not connect to MySQL(%s, %s, %s, %s, %d).\r\n",Mysql_url, Mysql_username, Mysql_password, Mysql_table, Mysql_port);
	}

	return OS_OK;
}

int judge_task_entry(void *pEntry)
{
	write_log(JUDGE_INFO,"Running Judge Core...");

	Judge_RegCmd();

	thread_create(judge_thread_msg, NULL);
	thread_create(judge_thread_timer, NULL);

	write_log(JUDGE_INFO,"Judge Task init ok...");

	/* 循环读取消息队列 */
	for(;;) {
		/* 放权 */
		Sleep(10);
	}

	closesocket(g_sListen);

#ifdef WIN32
	WSACleanup();
#endif

	return 0;
}

APP_INFO_S g_judgeAppInfo =
{
	NULL,
	"Judge",
	judge_init,
	judge_task_entry
};

void Judge_RegAppInfo()
{
	APP_RegistInfo(&g_judgeAppInfo);
}

#endif

