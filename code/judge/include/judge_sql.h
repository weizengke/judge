#ifndef _JUDGE_SQL_H_
#define _JUDGE_SQL_H_


extern MYSQL *mysql; 
extern char query[]; 
extern char Mysql_url[255];
extern char Mysql_username[255];
extern char Mysql_password[255];
extern char Mysql_table[255];
extern int  Mysql_port;
extern char Mysql_Character[255];

#define SQL_Debug(x, format, ...) debugcenter_print(MID_SQL, x, format, ##__VA_ARGS__)

extern int SQL_InitMySQL();
extern void SQL_getSolutionsByVerdict(int verdict, JUDGE_REQ_MSG_S *pJudgeData, int *n, int iMax);
extern int SQL_getSolutionSource(JUDGE_SUBMISSION_S *submission);
extern int SQL_getSolutionByID(int solutionID, JUDGE_SOLUTION_S *pstJudgeSolution, int *pIsExist);
extern int SQL_getProblemInfo(JUDGE_PROBLEM_S *pstProblem);
extern int SQL_getProblemInfo_contest(int contestId,int problemId,char *num);
extern int SQL_getContestInfo(int contestId,time_t &start_time,time_t &end_time);
extern int SQL_getContestAttend(int contestId,char *username,char num,long &ac_time,int &wrongsubmits);
extern int SQL_countContestProblems(int contestId);
extern int SQL_getFirstACTime_contest(int contestId,int problemId,char *username,time_t &ac_time,time_t start_time,time_t end_time);
extern long SQL_countProblemVerdict(int contestId,int problemId,int verdictId,char *username);
extern void SQL_updateSolution(JUDGE_SUBMISSION_S *submission);
extern void SQL_updateSolutionJsonResult(int solutionId, char *pJsonResult);
extern void SQL_updateProblem(int problemId);
extern void SQL_updateProblem_contest(int contestId,int problemId);
extern int SQL_countACContestProblems(int contestId,char *username,time_t start_time,time_t end_time);
extern int SQL_getContestScore(int contestId,char *username,time_t start_time,time_t end_time);
extern void SQL_updateAttend_contest(int contestId,int verdictId,int problemId,
										char *num,char *username,time_t start_time,time_t end_time);
extern void SQL_updateUser(char *username);
extern void SQL_updateContest(JUDGE_SUBMISSION_S *submission);
extern void SQL_updateCompileInfo(JUDGE_SUBMISSION_S *submission);
extern char **SQL_getContestActiveUser(int contestId, int *num);
extern int SQL_getRatingFromUser(const char *username);
extern void SQL_updateRatingToUser(char *username, int rating);
extern void SQL_updateRating(char *username, int contestId, int rank, int rating, int delta);
extern int SQL_getRatingBeforeContest(char *username, int contestId);

#endif
