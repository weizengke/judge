#ifndef _JUDGE_SQL_H_
#define _JUDGE_SQL_H_

extern int SQL_getSolutionSource();
extern int SQL_getSolutionInfo();
extern int SQL_getProblemInfo();
extern int SQL_getProblemInfo_contest(int contestId,int problemId,char *num);
extern int SQL_getContestInfo(int contestId,time_t &start_time,time_t &end_time);
extern int SQL_getContestAttend(int contestId,char *username,char num,long &ac_time,int &wrongsubmits);
extern int SQL_countContestProblems(int contestId);
extern int SQL_getFirstACTime_contest(int contestId,int problemId,char *username,time_t &ac_time,time_t start_time,time_t end_time);
extern long SQL_countProblemVerdict(int contestId,int problemId,int verdictId,char *username);
extern void SQL_updateSolution(int solutionId,int verdictId,int testCase,int time,int memory);
extern void SQL_updateProblem(int problemId);
extern void SQL_updateProblem_contest(int contestId,int problemId);
extern int SQL_countACContestProblems(int contestId,char *username,time_t start_time,time_t end_time);
extern int SQL_getContestScore(int contestId,char *username,time_t start_time,time_t end_time);
extern void SQL_updateAttend_contest(int contestId,int verdictId,int problemId,
										char *num,char *username,time_t start_time,time_t end_time);
extern void SQL_updateUser(char *username);
extern void SQL_updateCompileInfo(int solutionId);

#endif
