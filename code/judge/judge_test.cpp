#include "judge/include/judge_inc.h"

typedef struct tag_Judge_test_ST
{
	char testId[128];
	int problemId;
	int languageId;
	int verdictId;
	int time_used;
	int memory_used;
	char languageName[100];
	char languageExt[10];
	char languageExe[10];
    char sourcePath[MAX_PATH];
    char inputFile[MAX_PATH];
    char outputFile[MAX_PATH];
    char compileCmd[512];
    char runCmd[MAX_PATH];    
}JUDGE_TEST_S;

/* 
1: judge test xxx_id.json
   json:
   {
       "test_id: asas121dd2d2",
       "problem_id: 1000",
       "language_id", "1",
       "code": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
       "input": "sssss"
   },
   result:
   {
       "test_id: asas121dd2d2",
       "problem_id: 1000",
       "language_id", "1",
       "input": "sssss",
       "output": "aasasa"
   }
*/
char g_jsonBuff[1000000] = {0};

extern char g_judge_ini_cfg_path[];

int judge_test_init(JUDGE_TEST_S *test) {

    write_log(JUDGE_INFO, "testId=%s, languageId=%d", test->testId, test->languageId);

    sprintf(test->sourcePath, "%s\/%s\/data.in", g_judge_work_path, test->testId);
    sprintf(test->inputFile, "%s\/%s\/data.in", g_judge_work_path, test->testId);
    sprintf(test->outputFile, "%s\/%s\/data.out", g_judge_work_path, test->testId);

	char keyname[100]={0};
	sprintf(keyname,"Language%d", test->languageId);
	util_ini_get_string("Language", keyname, "", test->languageName, sizeof(test->languageName), g_judge_ini_cfg_path);
	util_ini_get_string("LanguageExt", test->languageName,"",test->languageExt, sizeof(test->languageExt),g_judge_ini_cfg_path);
	util_ini_get_string("LanguageExe", test->languageName,"",test->languageExe,sizeof(test->languageExe),g_judge_ini_cfg_path);
	util_ini_get_string("CompileCmd",test->languageName,"",test->compileCmd,sizeof(test->compileCmd),g_judge_ini_cfg_path);
	util_ini_get_string("RunCmd", test->languageName,"",test->runCmd,sizeof(test->runCmd),g_judge_ini_cfg_path);

    write_log(JUDGE_INFO, "sourcePath=%s", test->sourcePath);
    write_log(JUDGE_INFO, "inputFile=%s", test->inputFile);
    write_log(JUDGE_INFO, "outputFile=%s", test->outputFile);
    write_log(JUDGE_INFO, "languageName=%s", test->languageName);
    write_log(JUDGE_INFO, "languageExt=%s", test->languageExt);
    write_log(JUDGE_INFO, "languageExe=%s", test->languageExe);
    write_log(JUDGE_INFO, "compileCmd=%s", test->compileCmd);
    write_log(JUDGE_INFO, "runCmd=%s", test->runCmd);

    return 0;
}

void judge_test_parse(char *jsonBuff) {
    JUDGE_TEST_S test = {0};
    
    write_log(JUDGE_INFO, "%s", jsonBuff);

	cJSON *json = cJSON_Parse(jsonBuff);
	if (json == NULL) {
		write_log(JUDGE_INFO, "cJSON_Parse failed.");
		return ;
	}

	cJSON *json_test_id = cJSON_GetObjectItem(json, "test_id");
    if(json_test_id != NULL && json_test_id->type == cJSON_String) {
        write_log(JUDGE_INFO, "test_id=%s\n", json_test_id->valuestring);
        strncpy(test.testId, json_test_id->valuestring, sizeof(test.testId) - 1);
    }

	cJSON *json_problem_id = cJSON_GetObjectItem(json, "problem_id");
    if(json_problem_id != NULL &&  json_problem_id->type == cJSON_Number) {
        write_log(JUDGE_INFO, "problem_id=%d\n", json_problem_id->valueint);
		test.problemId = json_problem_id->valueint;
    } 

	cJSON *json_language_id = cJSON_GetObjectItem(json, "language_id");
    if(json_language_id != NULL &&  json_language_id->type == cJSON_Number) {
        write_log(JUDGE_INFO, "language_id=%d\n", json_language_id->valueint);
		test.languageId = json_language_id->valueint;
    } 

    judge_test_init(&test);
    
	cJSON *json_code = cJSON_GetObjectItem(json, "code");
    if(json_code != NULL && json_code->type == cJSON_String) {
        write_log(JUDGE_INFO, "code=%s\n", json_code->valuestring);
        FILE *fp = fopen(test.sourcePath, "w");
        if (fp != NULL) {
            fprintf(fp, "%s", json_code->valuestring);
            fclose(fp);
        }
    }

	cJSON *json_input = cJSON_GetObjectItem(json, "input");
    if(json_input != NULL && json_input->type == cJSON_String) {
        write_log(JUDGE_INFO, "code=%s\n", json_input->valuestring);
        FILE *fp = fopen(test.inputFile, "w");
        if (fp != NULL) {
            fprintf(fp, "%s", json_input->valuestring);
            fclose(fp);
        }
    }

}

void judge_test_run(char *json_file) {
    if (json_file == NULL) {
        return;
    }

    if( (file_access(json_file, 0 )) == -1 ) {
        write_log(JUDGE_INFO, "json_file %s is not exist.", json_file);
        return;
    }

    char *jsonBuff = (char*)malloc(1000000);
    memset(jsonBuff, 0, sizeof(1000000));
	(void)util_fread(json_file, jsonBuff, 1000000 - 1);

    int judge_sendto_remote_test(char *jsonMsg, int port, char *ip);
    judge_sendto_remote_test(jsonBuff, 5000, "127.0.0.1");
    
    free(jsonBuff);

    //judge_test_parse(g_jsonBuff);
}

void judge_test() {
    judge_test_run("1.json");
}