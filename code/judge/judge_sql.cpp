/*
  适配OJ MySQL数据库操作
*/

#include "judge/include/judge_inc.h"



#if (OS_YES == OSP_MODULE_JUDGE)

MYSQL *mysql = NULL;
char query[40960];
char Mysql_url[255];
char Mysql_username[255];
char Mysql_password[255];
char Mysql_table[255];
int Mysql_port;
char Mysql_Character[255];
int no_database = 0;

/* BEGIN: Added by weizengke, 2014/7/10  for 多线程判题信号量保护数据库访问 */

mutex_t hSemaphore_SQL;

VOID SQL_CreateSem()
{
	hSemaphore_SQL = mutex_create("SQL_SEM");
}

VOID SQL_SemP()
{
	(void)mutex_lock(hSemaphore_SQL);
}

VOID SQL_SemV()
{
	(void)mutex_unlock(hSemaphore_SQL);
}

/* END:   Added by weizengke, 2014/7/10 */

ULONG SQL_BuildRun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{

	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR *)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);

	pBuildrun = *ppBuildrun;

	pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN "mysql url %s port %u username %s password %s table %s",
						 Mysql_url, Mysql_port, Mysql_username, Mysql_password, Mysql_table);

	return OS_OK;
}

/* 初始化mysql，并设置字符集 */
int SQL_InitMySQL()
{
	if (no_database == 1) {
		return 1;
	}

	(VOID)DEBUG_PUB_RegModuleDebugs(MID_SQL, "sql", "Mysql");

#if 0
	(void)BDN_RegistBuildRun(MID_SQL, VIEW_SYSTEM, BDN_PRIORITY_NORMAL, SQL_BuildRun);
#endif

	SQL_CreateSem();

	mysql = mysql_init((MYSQL *)0);
	if (mysql != 0 &&
		!mysql_real_connect(mysql, Mysql_url,
							Mysql_username,
							Mysql_password,
							Mysql_table,
							Mysql_port,
							NULL,
							CLIENT_MULTI_STATEMENTS))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	/* 设置自动重连 */
	my_bool my_true = TRUE;
	(void)mysql_options(mysql, MYSQL_OPT_RECONNECT, &my_true);

	strcpy(query, "SET CHARACTER SET gbk"); //设置编码 gbk
	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret)
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	write_log(JUDGE_INFO, "Connect MySQL(%s, %s, %s, %s, %d) ok...", Mysql_url, Mysql_username, Mysql_password, Mysql_table, Mysql_port);
	printf("Connect MySQL(%s, %s, %s, %s, %d) ok...\r\n", Mysql_url, Mysql_username, Mysql_password, Mysql_table, Mysql_port);

	return 1;
}

int SQL_Destroy()
{
	mysql_close(mysql);

	return 0;
}

#if M_DES("donot p sem again", 1)

int SQL_getFirstACTime_contest(int contestId, int problemId, char *username, time_t &ac_time, time_t start_time, time_t end_time)
{
	string s_t, e_t;

	(VOID) util_time_to_string(s_t, start_time);
	(VOID) util_time_to_string(e_t, end_time);

	sprintf(query, "select submit_date from solution where contest_id=%d and problem_id=%d and username='%s'and verdict=%d and submit_date between '%s' and '%s' order by solution_id ASC limit 1;", contestId, problemId, username, V_AC, s_t.c_str(), e_t.c_str());
	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret)
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL)
	{
		return 0;
	}

	MYSQL_ROW row;
	if (row = mysql_fetch_row(recordSet))
	{
		(void)util_string_to_time(row[0], ac_time);
	}
	else
	{
		mysql_free_result(recordSet);
		return 0;
	}

	mysql_free_result(recordSet);

	return 1;
}

/* Note: No need to P Sem!!!! */
int SQL_countACContestProblems(int contestId, char *username, time_t start_time, time_t end_time)
{

	string s_t, e_t;

	(VOID) util_time_to_string(s_t, start_time);
	(VOID) util_time_to_string(e_t, end_time);

	sprintf(query, "select count(distinct(problem_id)) from solution where  verdict=%d and contest_id=%d and username='%s' and submit_date between '%s' and '%s'", V_AC, contestId, username, s_t.c_str(), e_t.c_str());

	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL)
	{
		return 0;
	}

	int nCount = 0;
	MYSQL_ROW row;
	if (row = mysql_fetch_row(recordSet))
	{
		nCount = atoi(row[0]);
	}

	mysql_free_result(recordSet);

	return nCount;
}

int SQL_getContestScore(int contestId, char *username, time_t start_time, time_t end_time)
{

	if (SQL_countACContestProblems(contestId, username, start_time, end_time) == 0)
	{
		return 0;
	}

	string s_t, e_t;

	(VOID) util_time_to_string(s_t, start_time);
	(VOID) util_time_to_string(e_t, end_time);

	sprintf(query, "SELECT sum(point) from contest_problem where contest_id=%d and problem_id in (select distinct(problem_id) from solution where  verdict=%d and contest_id=%d and username='%s' and submit_date between '%s' and '%s')", contestId, V_AC, contestId, username, s_t.c_str(), e_t.c_str());

	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL)
	{
		write_log(JUDGE_ERROR, "SQL_getContestScore recordSet JUDGE_ERROR");
	}

	int point = 0;
	MYSQL_ROW row;

	if (row = mysql_fetch_row(recordSet))
	{
		point = atoi(row[0]);
	}

	mysql_free_result(recordSet);

	return point;
}
int SQL_getContestAttend(int contestId, char *username, char num, long &ac_time, int &wrongsubmits)
{
	sprintf(query, "select %c_time,%c_wrongsubmits from attend where contest_id=%d and username='%s';", num, num, contestId, username);

	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret)
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL)
	{
		return 0;
	}

	MYSQL_ROW row;
	if (row = mysql_fetch_row(recordSet))
	{
		ac_time = atol(row[0]);
		wrongsubmits = atoi(row[1]);
	}

	mysql_free_result(recordSet);

	return 1;
}

int SQL_countContestProblems(int contestId)
{
	sprintf(query, "select count(problem_id) from contest_problem where contest_id=%d", contestId);

	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret)
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL)
	{
		return 0;
	}

	int nCount = 0;
	MYSQL_ROW row;
	if (row = mysql_fetch_row(recordSet))
	{
		nCount = atoi(row[0]);
	}

	mysql_free_result(recordSet);

	return nCount;
}

long SQL_countProblemVerdict(int contestId, int problemId, int verdictId, char *username)
{

	sprintf(query, "select count(solution_id) from solution where contest_id=%d and problem_id=%d and verdict=%d and username='%s'", contestId, problemId, verdictId, username);

	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret)
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL)
	{
		return 0;
	}

	long nCount = 0;

	MYSQL_ROW row;
	if (row = mysql_fetch_row(recordSet))
	{
		nCount = atoi(row[0]);
	}

	mysql_free_result(recordSet);

	return nCount;
}

#endif

int SQL_getSolutionSource(JUDGE_SUBMISSION_S *submission)
{
	if (NULL == submission) {
		write_log(JUDGE_ERROR, "SQL_getSolutionSourceEx, Input param is null...");
		return OS_ERR;
	}

	SQL_SemP();

	sprintf(query, "select source from solution_source where solution_id=%d",
			submission->solution.solutionId);

	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret) {
		write_log(JUDGE_ERROR, mysql_error(mysql));
		SQL_SemV();
		return OS_ERR;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL) {
		write_log(JUDGE_ERROR, "SQL_getSolutionSource");
		SQL_SemV();
		return OS_ERR;
	}

	FILE *fp_source = fopen(submission->sourcePath, "w");
	char code[MAX_CODE] = {0};
	MYSQL_ROW row;

	if (row = mysql_fetch_row(recordSet)) {
		sprintf(code, "%s", row[0]);
	} else {
		write_log(JUDGE_ERROR, "SQL_getSolutionSource Error");
	}
    //util_fwrite(submission->sourcePath, code, strlen(code));
	fprintf(fp_source, "%s", code);

	mysql_free_result(recordSet);
	fclose(fp_source);

	SQL_SemV();

	write_log(JUDGE_INFO, "SQL get solution source ok. (solutionId=%u)", submission->solution.solutionId);

	return OS_OK;
}

void SQL_getSolutionsByVerdict(int verdict, JUDGE_REQ_MSG_S *pJudgeData, int *n, int iMax)
{
	int ret = OS_OK;
	int m = 0;
	MYSQL_RES *recordSet = NULL;
	MYSQL_ROW row;
	JUDGE_REQ_MSG_S *pJudgeData2 = NULL;

	if (NULL == pJudgeData || NULL == n)
	{
		return;
	}

	SQL_SemP();

	*n = 0;
	pJudgeData2 = pJudgeData;

	sprintf(query, "select solution_id from solution where verdict=%d order by solution_id asc limit 0,%d", verdict, iMax);

	ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret)
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
		SQL_SemV();
		return;
	}

	recordSet = mysql_store_result(mysql);
	if (recordSet == NULL)
	{
		write_log(JUDGE_ERROR, "Error SQL_getSolutionData");
		SQL_SemV();
		return;
	}

	while (row = mysql_fetch_row(recordSet))
	{
		pJudgeData2->solutionId = atoi(row[0]);
		SQL_Debug(DEBUG_TYPE_FUNC, "SQL getSolutionsByVerdict. (solutionId=%d)", pJudgeData2->solutionId);

		pJudgeData2++;
		m++;

		if (m == iMax)
		{
			break;
		}
	}

	*n = m;

	SQL_Debug(DEBUG_TYPE_FUNC, "SQL getSolutionsByVerdict. (verdict=%d, n=%d)", verdict, m);

	mysql_free_result(recordSet);

	SQL_SemV();

	return;
}

int SQL_getSolutionByID(int solutionID, JUDGE_SOLUTION_S *pstJudgeSolution, int *pIsExist)
{
	int ret = OS_OK;
	MYSQL_RES *recordSet = NULL;
	MYSQL_ROW row;

	if (NULL == pstJudgeSolution || NULL == pIsExist)
	{
		return OS_ERR;
	}

	*pIsExist = OS_NO;

	SQL_SemP();

	sprintf(query, "select problem_id,contest_id,language,username,submit_date from solution where solution_id=%d",
			solutionID);

	ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret)
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
		SQL_SemV();
		return OS_ERR;
	}

	recordSet = mysql_store_result(mysql);
	if (recordSet == NULL)
	{
		write_log(JUDGE_ERROR, "Error SQL_getSolutionData");
		SQL_SemV();
		return OS_ERR;
	}

	if (row = mysql_fetch_row(recordSet))
	{
		pstJudgeSolution->solutionId = solutionID;

		if (NULL == row[0])
		{
			mysql_free_result(recordSet);
			write_log(JUDGE_INFO, "solutionID %d data(problemId) is not valid in MySQL Server.", solutionID);
			SQL_SemV();
			return OS_ERR;
		}

		if (NULL == row[1])
		{
			mysql_free_result(recordSet);
			write_log(JUDGE_INFO, "solutionID %d data(contestId) is not valid in MySQL Server.", solutionID);
			SQL_SemV();
			return OS_ERR;
		}

		if (NULL == row[2])
		{
			mysql_free_result(recordSet);
			write_log(JUDGE_INFO, "solutionID %d data(languageId) is not valid in MySQL Server.", solutionID);
			SQL_SemV();
			return OS_ERR;
		}

		if (NULL == row[3])
		{
			mysql_free_result(recordSet);
			write_log(JUDGE_INFO, "solutionID %d data(username) is not valid in MySQL Server.", solutionID);
			SQL_SemV();
			return OS_ERR;
		}

		if (NULL == row[4])
		{
			mysql_free_result(recordSet);
			write_log(JUDGE_INFO, "solutionID %d data(submitDate) is not valid in MySQL Server.", solutionID);
			SQL_SemV();
			return OS_ERR;
		}

		pstJudgeSolution->problemId = atoi(row[0]);
		pstJudgeSolution->contestId = atoi(row[1]);
		pstJudgeSolution->languageId = atoi(row[2]);
		strcpy(pstJudgeSolution->username, row[3]);
		(void)util_string_to_time(row[4], pstJudgeSolution->submitDate);
		*pIsExist = OS_YES;

		//write_log(JUDGE_INFO,"Found record. (solutionID=%d)", solutionID);
	}
	else
	{
		write_log(JUDGE_ERROR, "No such record. (solutionID=%d)", solutionID);
	}

	mysql_free_result(recordSet);

	SQL_SemV();

	return OS_OK;
}

int SQL_getProblemInfo(JUDGE_PROBLEM_S *pstProblem)
{
	if (NULL == pstProblem)
	{
		write_log(JUDGE_ERROR, "SQL_getProblemInfo, Input param is null...");
		return OS_ERR;
	}

	SQL_SemP();

	sprintf(query, "select time_limit,memory_limit,spj,isvirtual,oj_pid,oj_name from problem where problem_id=%d",
			pstProblem->problemId);

	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret)
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
		SQL_SemV();
		return OS_ERR;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL)
	{
		SQL_SemV();
		return OS_ERR;
	}

	MYSQL_ROW row;
	if (row = mysql_fetch_row(recordSet))
	{
		pstProblem->time_limit = atoi(row[0]);
		pstProblem->memory_limit = atoi(row[1]);
		pstProblem->isSpecialJudge = atoi(row[2]);
		pstProblem->isVirtualJudge = atoi(row[3]);
		strcpy(pstProblem->virtualPID, row[4]);
		strcpy(pstProblem->szVirJudgerName, row[5]);
	}

	mysql_free_result(recordSet);

	SQL_SemV();

	write_log(JUDGE_INFO, "SQL get problem info ok. (problemId=%u)", pstProblem->problemId);

	return OS_OK;
}

int SQL_getProblemInfo_contest(int contestId, int problemId, char *num)
{
	sprintf(query, "select num from contest_problem where contest_id=%d and problem_id=%d", contestId, problemId);

	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret)
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL)
	{
		write_log(JUDGE_INFO, "SQL_getProblemInfo_contest ,No record");
		return 0;
	}

	MYSQL_ROW row;
	if (row = mysql_fetch_row(recordSet))
	{
		strcpy(num, row[0]);
	}

	mysql_free_result(recordSet);

	return 1;
}

int SQL_getContestInfo(int contestId, time_t &start_time, time_t &end_time)
{
	//start_time,end_time
	sprintf(query, "select start_time,end_time from contest where contest_id=%d", contestId);
	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret)
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL)
	{
		return 0;
	}

	MYSQL_ROW row;
	if (row = mysql_fetch_row(recordSet))
	{
		(void)util_string_to_time(row[0], start_time);
		(void)util_string_to_time(row[1], end_time);
	}

	mysql_free_result(recordSet);

	return 1;
}

/* update Solution table*/
void SQL_updateSolution(JUDGE_SUBMISSION_S *submission)
{
	SQL_SemP();

	sprintf(query, "update solution set verdict=%d,testcase=%d,failcase=%d,time=%d,memory=%d where solution_id=%d;",
					submission->solution.verdictId, 
					submission->solution.testcase, 
					submission->solution.failcase, 
					submission->solution.time_used - submission->solution.time_used%10, 
					submission->solution.memory_used, 
					submission->solution.solutionId);
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));

		/* 兼容老版本不支持failcase字段 */
		memset(query, 0, sizeof(query));
		sprintf(query, "update solution set verdict=%d,testcase=%d,time=%d,memory=%d where solution_id=%d;",
		 				submission->solution.verdictId,
						submission->solution.testcase, 
						submission->solution.time_used - submission->solution.time_used%10, 
						submission->solution.memory_used, 
						submission->solution.solutionId);
		mysql_real_query(mysql, query, (unsigned int)strlen(query));
	}

	SQL_SemV();
}

void SQL_updateSolutionJsonResult(int solutionId, char *pJsonResult)
{
	SQL_SemP();

	sprintf(query, "update solution set json_result='%s' where solution_id=%d;", pJsonResult, solutionId);

	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
	}

	SQL_SemV();
}

/* update problem table*/
void SQL_updateProblem(int problemId)
{
	SQL_SemP();

	sprintf(query, "update problem set accepted=(SELECT count(*) FROM solution WHERE problem_id=%d and verdict=%d) where problem_id=%d;", problemId, V_AC, problemId);
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
	}

	sprintf(query, "update problem set submit=(SELECT count(*) FROM solution WHERE problem_id=%d)  where problem_id=%d;", problemId, problemId);
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
	}

	sprintf(query, "update problem set solved=(SELECT count(DISTINCT username) FROM solution WHERE problem_id=%d and verdict=%d) where problem_id=%d;", problemId, V_AC, problemId);
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
	}

	sprintf(query, "update problem set submit_user=(SELECT count(DISTINCT username) FROM solution WHERE problem_id=%d) where problem_id=%d;", problemId, problemId);
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
	}

	SQL_SemV();
}

void SQL_updateProblem_contest(int contestId, int problemId)
{
	SQL_SemP();

	sprintf(query, "update contest_problem set accepted=(SELECT count(*) FROM solution WHERE contest_id=%d and problem_id=%d and verdict=%d) where contest_id=%d and problem_id=%d;", contestId, V_AC, problemId, contestId, problemId);
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
	}

	sprintf(query, "update contest_problem set submit=(SELECT count(*) FROM solution WHERE contest_id=%d and problem_id=%d)  where contest_id=%d and problem_id=%d;", contestId, problemId, contestId, problemId);
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
	}

	sprintf(query, "update contest_problem set solved=(SELECT count(DISTINCT username) FROM solution WHERE contest_id=%d and problem_id=%d and verdict=%d) where contest_id=%d and problem_id=%d;", contestId, problemId, V_AC, contestId, problemId);
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
	}

	sprintf(query, "update contest_problem set submit_user=(SELECT count(DISTINCT username) FROM solution WHERE contest_id=%d and problem_id=%d) where contest_id=%d and problem_id=%d;", contestId, problemId, contestId, problemId);
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
	}

	SQL_SemV();
}

void SQL_updateAttend_contest(int contestId, int verdictId, int problemId, char *num, char *username, time_t start_time, time_t end_time)
{

	//已经AC的不需要修改attend
	//update ac_time
	long AC_time = 0;
	time_t first_ac_t;

	SQL_SemP();

	if (SQL_getFirstACTime_contest(contestId, problemId, username, first_ac_t, start_time, end_time))
	{
		AC_time = util_getdiftime(first_ac_t, start_time);
	}
	else
	{
		AC_time = 0;
		first_ac_t = end_time;
	}

	sprintf(query, "update attend set %s_time=%ld where contest_id=%d and username='%s';", num, AC_time, contestId, username);
	//cout<<query<<endl;
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
	}

	long ac_nCount = SQL_countProblemVerdict(contestId, problemId, V_AC, username);
	int score_ = SQL_getContestScore(contestId, username, start_time, end_time);
	string s_t, e_t, fAC_t;
	(VOID) util_time_to_string(s_t, start_time);
	(VOID) util_time_to_string(e_t, end_time);
	(VOID) util_time_to_string(fAC_t, first_ac_t);

	//update score solved ,wrongsubmits
	sprintf(query, "update attend set solved=(SELECT count(DISTINCT problem_id) FROM solution WHERE contest_id=%d and username='%s' and verdict=%d and submit_date between '%s' and '%s'),%s_wrongsubmits=(SELECT count(solution_id) FROM solution WHERE contest_id=%d and problem_id=%d and username='%s' and verdict>%d and submit_date between '%s' and '%s'),score=%d  where contest_id=%d and username='%s';", contestId, username, V_AC, s_t.c_str(), e_t.c_str(), num, contestId, problemId, username, V_AC, s_t.c_str(), fAC_t.c_str(), score_, contestId, username);
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
	}

	//penalty
	int nCountProblems = SQL_countContestProblems(contestId);
	char index = 'A';
	long penalty = 0;
	for (int i = 0; i < nCountProblems; i++)
	{
		long a_time_ = 0;
		int wrongsubmits_ = 0;
		SQL_getContestAttend(contestId, username, index + i, a_time_, wrongsubmits_);

		if (a_time_ > 0)
		{
			penalty = penalty + a_time_ + wrongsubmits_ * 60 * 20;
		}
	}

	sprintf(query, "update attend set penalty=%ld where contest_id=%d and username='%s';", penalty, contestId, username);
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
	}

	SQL_SemV();
}

/* update user table*/
void SQL_updateUser(char *username)
{
	SQL_SemP();

	sprintf(query, "update users set submit=(SELECT count(*) FROM solution WHERE username='%s') where username='%s';", username, username);
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
	}

	sprintf(query, "update users set solved=(SELECT count(DISTINCT problem_id) FROM solution WHERE username='%s' and verdict=%d) where username='%s';", username, V_AC, username);
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR, mysql_error(mysql));
	}

	SQL_SemV();
}
void SQL_updateContest(JUDGE_SUBMISSION_S *submission)
{
	/* contest or not */
	if(submission->solution.contestId == 0) {
		return;
	}

	/* contest judge */
	time_t contest_s_time,contest_e_time;
	char num[10]={0};

	/* 获取contest problem题目标号 */
	SQL_getProblemInfo_contest(submission->solution.contestId, submission->solution.problemId,num);
	SQL_getContestInfo(submission->solution.contestId,contest_s_time,contest_e_time);

	if(contest_e_time>submission->solution.submitDate)
	{
		/* 比赛running ，修改Attend */
		SQL_updateAttend_contest(submission->solution.contestId, submission->solution.verdictId,
								submission->solution.problemId, num, submission->solution.username,
								contest_s_time,contest_e_time);
	}

	SQL_updateProblem_contest(submission->solution.contestId, submission->solution.problemId);
}

void SQL_updateCompileInfo(JUDGE_SUBMISSION_S *submission)
{
	char buffer[4096] = {0};

	if (submission->solution.verdictId != V_CE) {
		return;
	}

	SQL_SemP();

	snprintf(query, sizeof(buffer) - 1, "delete from compile_info where solution_id=%d;", submission->solution.solutionId);
	if (mysql_real_query(mysql, query, (unsigned int)strlen(query))) {
		write_log(JUDGE_ERROR, "%s\r\n%s", mysql_error(mysql), query);
	}

	util_fread(submission->DebugFile, buffer, 4000);
	string str = buffer;
	string::iterator it;
	for (it = str.begin(); it != str.end(); ++it) {
		if (*it == '\"') {
			str.erase(it);
		}
	}

	snprintf(query, sizeof(buffer) - 1, "insert into compile_info values(%d,\"%s\");", submission->solution.solutionId, str.c_str());
	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret) {
		write_log(JUDGE_ERROR, "%s\r\n%s", mysql_error(mysql), query);
		/* 解决插入失败，导致的数据库访问异常 */
		while (!mysql_next_result(mysql)) {
			MYSQL_RES *recordSet = mysql_store_result(mysql);
			mysql_free_result(recordSet);
		}		
		SQL_SemV();
		return;
	}

	SQL_SemV();
}

int SQL_getContestType(int contestId)
{
	int type = 0;

	sprintf(query, "select type from contest where contest_id=%d", contestId);
	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret) {
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return type;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL) {
		return type;
	}

	MYSQL_ROW row;
	if (row = mysql_fetch_row(recordSet)) {
		type = atoi(row[0]);
	}

	mysql_free_result(recordSet);

	return type;
}


int SQL_countContestActiveUser(int contestId)
{
	int type = SQL_getContestType(contestId);
	if (type == 0) {
		sprintf(query, "select count(username) from attend where contest_id=%d order by solved DESC ,penalty ASC;", contestId);
	} else {
		sprintf(query, "select count(username) from attend where contest_id=%d order by score DESC ,penalty ASC;", contestId);
	}
	
	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret) {
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL) {
		return 0;
	}

	int num = 0;
	MYSQL_ROW row;
	if (row = mysql_fetch_row(recordSet)) {
		num = atoi(row[0]);
	}

	mysql_free_result(recordSet);

	return num;
}

int SQL_IsUserHasAttend(int contestId, char *username) {
	int nCountProblems = SQL_countContestProblems(contestId);
	char index = 'A';
	long penalty = 0;
	for (int i = 0; i < nCountProblems; i++) {
		long a_time_ = 0;
		int wrongsubmits_ = 0;
		SQL_getContestAttend(contestId, username, index + i, a_time_, wrongsubmits_);
		if (a_time_ > 0 || wrongsubmits_ > 0) {
			return 1;
		}
	}

	return 0;
}

char **SQL_getContestActiveUser(int contestId, int *num)
{
	*num = 0;
	int n = SQL_countContestActiveUser(contestId);
	if (n == 0) {
		return NULL;
	}
	
	int type = SQL_getContestType(contestId);
	if (type == 0) {
		sprintf(query, "select username from attend where contest_id=%d order by solved DESC ,penalty ASC;", contestId);
	} else {
		sprintf(query, "select username from attend where contest_id=%d order by score DESC ,penalty ASC;", contestId);
	}
	
	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret) {
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return NULL;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL) {
		return NULL;
	}

	char **users = (char **)malloc(n * sizeof(char*));
	if (users == NULL) {
		mysql_free_result(recordSet);
		return NULL;
	}

	int i = 0;
	MYSQL_ROW row;
	while (row = mysql_fetch_row(recordSet)) {
		if (0 == SQL_IsUserHasAttend(contestId, row[0])) {
			continue;
		}
		int len = strlen(row[0]) + 1;
		users[i] = (char *)malloc(len);
		if (users[i] == NULL) {
			continue;
		}
		memset(users[i], 0, len);
		strcpy(users[i], row[0]);
		i++;

		if (i >= n) {
			break;
		}
	}

	mysql_free_result(recordSet);
	*num = i;

	return users;
}

int SQL_getRatingFromUser(const char *username)
{
	int ret = OS_OK;
	if (strlen(username) >= MAX_NAME || mysql == NULL) {
		return 0;
	}

	sprintf(query, "select rating from users where username='%s'", username);

	ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret) {
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL) {
		return 0;
	}

	int rating = 0;
	MYSQL_ROW row;
	if (row = mysql_fetch_row(recordSet)) {
		rating = atoi(row[0]);
	}

	mysql_free_result(recordSet);

	return rating;
}

/*
10 [3000, inf) Legendary Grandmaster 大红 
9 [2600, 3000) International Grandmaster 鲜红  
8 [2200, 2600) Grandmaster 黄 
7 [2050, 2200) International Master 橙 
6 [1900, 2050) Master 粉 
5 [1750, 1900) Candidate Master 紫 
4 [1600, 1750) Expert 蓝
3 [1400, 1600) Specialist 绿  
2 [1200, 1400) Pupil 青
1 (-inf, 1200) Newbie 灰 
0 -> black
*/

int SQL_getRate(int  rating){
	if (rating == 0) {
		return 0;
	}
	
	if (rating < 1200 && rating > 0) {			
		return 1;
	}
	if (rating < 1400) {
		return 2;			
	}
	if (rating < 1600) {
		return 3;			
	}
	if (rating < 1750) {
		return 4;			
	}
	if (rating < 1900) {
		return 5;			
	}
	if (rating < 2050) {
		return 6;			
	}
	if (rating < 2200) {
		return 7;			
	}
	if (rating < 2600) {
		return 8;			
	}
	if (rating < 3000) {
		return 9;			
	}
	return 10;				
}

void SQL_updateRatingToUser(char *username, int rating)
{
	sprintf(query, "update users set rating=%d, rate=%d where username='%s';", rating, SQL_getRate(rating), username);

	write_log(JUDGE_INFO, "%s", query);

	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret) {
		write_log(JUDGE_ERROR, "%s\r\n%s", mysql_error(mysql), query);
		return;
	}
}

int SQL_getRatingBeforeContest(char *username, int contestId) {
	int ret = OS_OK;
	if (strlen(username) >= MAX_NAME || mysql == NULL) {
		return 0;
	}

	time_t st_date;
	time_t end_date;
	if (0 == SQL_getContestInfo(contestId, st_date, end_date)) {
		printf("SQL_getContestInfo failed.");
		return 0;
	}

	string time_string;
	(VOID)util_time_to_string(time_string, end_date);
	sprintf(query, "select rating from rating where username='%s' and rating_date < '%s' order by rating_date desc", username, time_string.c_str());

	ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret) {
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL) {
		return 0;
	}

	int rating = 0;
	MYSQL_ROW row;
	if (row = mysql_fetch_row(recordSet)) {
		rating = atoi(row[0]);
	}

	mysql_free_result(recordSet);

	return rating;
}

int SQL_isUserHasRating(char *username, int contestId) {
	int ret = OS_OK;
	if (strlen(username) >= MAX_NAME || mysql == NULL) {
		return 0;
	}

	sprintf(query, "select rating from rating where username='%s' and contest_id=%d", username, contestId);

	ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret) {
		write_log(JUDGE_ERROR, mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet == NULL) {
		return 0;
	}

	int found = 0;
	MYSQL_ROW row;
	if (row = mysql_fetch_row(recordSet)) {
		found = 1;
	}

	mysql_free_result(recordSet);

	return found;
}

void SQL_updateRating(char *username, int contestId, int rank, int rating, int delta)
{
	time_t st_date;
	time_t end_date;
	if (0 == SQL_getContestInfo(contestId, st_date, end_date)) {
		printf("get contest info failed.\n");
		return;
	}

	string time_string;
	(VOID)util_time_to_string(time_string, end_date);

	if (1 == SQL_isUserHasRating(username, contestId)) {
		sprintf(query, "update rating set rank=%d, delta=%d, rating=%d, rating_date='%s' where username='%s' and contest_id=%d", 
				rank, delta, rating, time_string.c_str(), username, contestId);
	} else {
		sprintf(query, "insert into rating (username, contest_id, rank, delta, rating, rating_date) "
					"values(\"%s\", %d, %d, %d, %d, \"%s\");", 
				username, contestId, rank, delta, rating, time_string.c_str());
	}

	write_log(JUDGE_INFO, "%s", query);

	int ret = mysql_real_query(mysql, query, (unsigned int)strlen(query));
	if (ret) {
		write_log(JUDGE_ERROR, "%s\r\n%s", mysql_error(mysql), query);
		return;
	}

	SQL_updateRatingToUser(username, rating);
}

#endif
