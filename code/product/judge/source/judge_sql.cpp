/*
  适配OJ MySQL数据库操作
*/

#include <windows.h>
#include <iostream>
#include <conio.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#include <queue>
#include <string>
#include <sstream>


#include "product\judge\include\judge_inc.h"


MYSQL *mysql;     //mysql连接
char query[1024]; //查询语句
char Mysql_url[255];
char Mysql_username[255];
char Mysql_password[255];
char Mysql_table[255];
int  Mysql_port;
char Mysql_Character[255];  //编码

/* 初始化mysql，并设置字符集 */
int SQL_InitMySQL()
{
	mysql=mysql_init((MYSQL*)0);
	if(mysql!=0 && !mysql_real_connect(mysql,Mysql_url, Mysql_username, Mysql_password, Mysql_table,Mysql_port,NULL,CLIENT_MULTI_STATEMENTS )){
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	strcpy(query,"SET CHARACTER SET gbk"); //设置编码 gbk
	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret){
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}
	return 1;
}

int SQL_Destroy()
{
	mysql_close(mysql);
}

int SQL_getSolutionSource()
{
	sprintf(query,"select source from solution_source where solution_id=%d",GL_solutionId);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return OS_ERR;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		write_log(JUDGE_ERROR,"SQL_getSolutionSource");
		return OS_ERR;
	}

	FILE *fp_source = fopen(sourcePath, "w");
	char code[MAX_CODE]={0};
	MYSQL_ROW row;

	if(row=mysql_fetch_row(recordSet))
	{
		sprintf(code, "%s", row[0]);
	}
	else
	{
		write_log(JUDGE_ERROR,"SQL_getSolutionSource Error");
	}

	if(isTranscoding==1)
	{
		int ii=0;
		//解决VS下字符问题
		while (code[ii]!='\0')
		{
			if (code[ii]=='\r')
			{
				code[ii] = '\n';
			}

			ii++;
		}
	}

	fprintf(fp_source, "%s", code);

	/* add for vjudge*/
	string code_ = code;
	GL_source = code_;

	mysql_free_result(recordSet);
	fclose(fp_source);
	return OS_OK;
}

int SQL_getSolutionInfo(int *pIsExist)
{
	*pIsExist = OS_NO;

	sprintf(query,"select problem_id,contest_id,language,username,submit_date from solution where solution_id=%d",GL_solutionId);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return OS_ERR;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		write_log(JUDGE_ERROR,"Error SQL_getSolutionData");
		return OS_ERR;
	}

	//获取数据
	MYSQL_ROW row; //一个行数据的类型安全(type-safe)的表示
	if (row = mysql_fetch_row(recordSet))  //获取下一条记录
	{
		GL_problemId=atoi(row[0]);
		GL_contestId=atoi(row[1]);
		GL_languageId=atoi(row[2]);
		strcpy(GL_username,row[3]);
		StringToTimeEX(row[4],GL_submitDate);
		*pIsExist = OS_YES;
		write_log(JUDGE_INFO,"Found record.");
	}
	else
	{
		write_log(JUDGE_ERROR,"No such record.");
	}

	/* 释放结果集 */
	mysql_free_result(recordSet);

	return OS_OK;
}

int SQL_getProblemInfo()
{
	sprintf(query,"select time_limit,memory_limit,spj,isvirtual,oj_pid,oj_name from problem where problem_id=%d",GL_problemId);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return OS_ERR;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		return OS_ERR;
	}

	//获取数据
	MYSQL_ROW row; //一个行数据的类型安全(type-safe)的表示
	if(row=mysql_fetch_row(recordSet))  //获取下一条记录
	{
		GL_time_limit=atoi(row[0]);
		GL_memory_limit=atoi(row[1]);
		GL_spj=atoi(row[2]);
		GL_vjudge=atoi(row[3]);
		GL_vpid=atoi(row[4]);
		GL_ojname=row[5];
	}

	mysql_free_result(recordSet);//释放结果集
	return OS_OK;
}

int SQL_getProblemInfo_contest(int contestId,int problemId,char *num)
{
	sprintf(query,"select num from contest_problem where contest_id=%d and problem_id=%d",contestId,problemId);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		write_log(JUDGE_INFO,"SQL_getProblemInfo_contest ,No record");
		return 0;
	}

	//获取数据
	MYSQL_ROW row; //一个行数据的类型安全(type-safe)的表示
	if(row=mysql_fetch_row(recordSet))  //获取下一条记录
	{
		strcpy(num,row[0]);
	}

	mysql_free_result(recordSet);//释放结果集
	return 1;
}

int SQL_getContestInfo(int contestId,time_t &start_time,time_t &end_time)
{
	//start_time,end_time
	sprintf(query,"select start_time,end_time from contest where contest_id=%d",contestId);
	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		return 0;
	}

	//获取数据
	MYSQL_ROW row; //一个行数据的类型安全(type-safe)的表示
	if(row=mysql_fetch_row(recordSet))  //获取下一条记录
	{
		StringToTimeEX(row[0],start_time);
		StringToTimeEX(row[1],end_time);
	}

	mysql_free_result(recordSet);//释放结果集
	return 1;
}
int SQL_getContestAttend(int contestId,char *username,char num,long &ac_time,int &wrongsubmits)
{
	sprintf(query,"select %c_time,%c_wrongsubmits from attend where contest_id=%d and username='%s';",num,num,contestId,username);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		return 0;
	}

	MYSQL_ROW row;
	if(row=mysql_fetch_row(recordSet))
	{
		ac_time=atol(row[0]);
		wrongsubmits=atoi(row[1]);
	}

	mysql_free_result(recordSet);

	return 1;
}

int SQL_countContestProblems(int contestId)
{
	sprintf(query,"select count(problem_id) from contest_problem where contest_id=%d",contestId);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		return 0;
	}

	int nCount=0;
	MYSQL_ROW row;
	if(row=mysql_fetch_row(recordSet))
	{
		nCount=atoi(row[0]);
	}

	mysql_free_result(recordSet);
	return nCount;
}

int SQL_getFirstACTime_contest(int contestId,int problemId,char *username,time_t &ac_time,time_t start_time,time_t end_time){
	//
	string s_t,e_t;
	API_TimeToString(s_t,start_time);
	API_TimeToString(e_t,end_time);

	sprintf(query,"select submit_date from solution where contest_id=%d and problem_id=%d and username='%s'and verdict=%d and submit_date between '%s' and '%s' order by solution_id ASC limit 1;",contestId,problemId,username,V_AC,s_t.c_str(),e_t.c_str());
	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{

		return 0;
	}

	MYSQL_ROW row;
	if(row=mysql_fetch_row(recordSet))
	{

		StringToTimeEX(row[0],ac_time);
	}
	else
	{
		return 0;
	}

	mysql_free_result(recordSet);

	return 1;
}

long SQL_countProblemVerdict(int contestId,int problemId,int verdictId,char *username)
{

	sprintf(query,"select count(solution_id) from solution where contest_id=%d and problem_id=%d and verdict=%d and username='%s'",contestId,problemId,verdictId,username);

	int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
	if(ret)
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		return 0;
	}

	long nCount=0;

	//获取数据
	MYSQL_ROW row; //一个行数据的类型安全(type-safe)的表示
	if(row=mysql_fetch_row(recordSet))  //获取下一条记录
	{
		nCount=atoi(row[0]);
	}

	mysql_free_result(recordSet);//释放结果集
	return nCount;
}

/* update Solution table*/
void SQL_updateSolution(int solutionId,int verdictId,int testCase,int time,int memory)
{
	sprintf(query,"update solution set verdict=%d,testcase=%d,time=%d,memory=%d where solution_id=%d;",verdictId,testCase,time,memory,solutionId);

	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}
}

/* update problem table*/
void SQL_updateProblem(int problemId)
{
	sprintf(query,"update problem set accepted=(SELECT count(*) FROM solution WHERE problem_id=%d and verdict=%d) where problem_id=%d;",problemId,V_AC,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	sprintf(query,"update problem set submit=(SELECT count(*) FROM solution WHERE problem_id=%d)  where problem_id=%d;",problemId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	sprintf(query,"update problem set solved=(SELECT count(DISTINCT username) FROM solution WHERE problem_id=%d and verdict=%d) where problem_id=%d;",problemId,V_AC,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	sprintf(query,"update problem set submit_user=(SELECT count(DISTINCT username) FROM solution WHERE problem_id=%d) where problem_id=%d;",problemId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

}
void SQL_updateProblem_contest(int contestId,int problemId)
{
	sprintf(query,"update contest_problem set accepted=(SELECT count(*) FROM solution WHERE contest_id=%d and problem_id=%d and verdict=%d) where contest_id=%d and problem_id=%d;",contestId,V_AC,contestId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	sprintf(query,"update contest_problem set submit=(SELECT count(*) FROM solution WHERE contest_id=%d and problem_id=%d)  where contest_id=%d and problem_id=%d;",contestId,problemId,contestId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	sprintf(query,"update contest_problem set solved=(SELECT count(DISTINCT username) FROM solution WHERE contest_id=%d and problem_id=%d and verdict=%d) where contest_id=%d and problem_id=%d;",contestId,problemId,V_AC,contestId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	sprintf(query,"update contest_problem set submit_user=(SELECT count(DISTINCT username) FROM solution WHERE contest_id=%d and problem_id=%d) where contest_id=%d and problem_id=%d;",contestId,problemId,contestId,problemId);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

}

int SQL_countACContestProblems(int contestId,char *username,time_t start_time,time_t end_time){

	string s_t,e_t;
	API_TimeToString(s_t,start_time);
	API_TimeToString(e_t,end_time);

	sprintf(query,"select count(distinct(problem_id)) from solution where  verdict=%d and contest_id=%d and username='%s' and submit_date between '%s' and '%s'",V_AC,contestId,username,s_t.c_str(),e_t.c_str());

	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		return 0;
	}

	int nCount=0;
	MYSQL_ROW row;
	if(row=mysql_fetch_row(recordSet))
	{
		nCount=atoi(row[0]);
	}

	mysql_free_result(recordSet);
	return nCount;
}


int SQL_getContestScore(int contestId,char *username,time_t start_time,time_t end_time){

	if(SQL_countACContestProblems(contestId,username,start_time,end_time)==0)
	{
		return 0;
	}

	string s_t,e_t;
	API_TimeToString(s_t,start_time);
	API_TimeToString(e_t,end_time);

	sprintf(query,"SELECT sum(point) from contest_problem where contest_id=%d and problem_id in (select distinct(problem_id) from solution where  verdict=%d and contest_id=%d and username='%s' and submit_date between '%s' and '%s')",contestId,V_AC,contestId,username,s_t.c_str(),e_t.c_str());

	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
		return 0;
	}

	MYSQL_RES *recordSet = mysql_store_result(mysql);
	if (recordSet==NULL)
	{
		write_log(JUDGE_ERROR,"SQL_getContestScore recordSet JUDGE_ERROR");
	}

	int point=0;
	MYSQL_ROW row;

	if(row=mysql_fetch_row(recordSet))
	{
		point=atoi(row[0]);
	}

	mysql_free_result(recordSet);

	return point;
}

void SQL_updateAttend_contest(int contestId,int verdictId,int problemId,char *num,char *username,time_t start_time,time_t end_time){

	//已经AC的不需要修改attend
	//update ac_time
	long AC_time=0;
	time_t first_ac_t;

	if(SQL_getFirstACTime_contest(contestId,problemId,username,first_ac_t,start_time,end_time))
	{
		AC_time=getdiftime(first_ac_t,start_time);
	}
	else
	{
		AC_time=0;
		first_ac_t = end_time;
	}

	sprintf(query,"update attend set %s_time=%ld where contest_id=%d and username='%s';",num,AC_time,contestId,username);
	//cout<<query<<endl;
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	long ac_nCount=SQL_countProblemVerdict(GL_contestId,problemId,V_AC,username);
	int score_ = SQL_getContestScore(contestId,username,start_time,end_time);
	string s_t,e_t,fAC_t;
	API_TimeToString(s_t,start_time);
	API_TimeToString(e_t,end_time);
	API_TimeToString(fAC_t,first_ac_t);

	//update score solved ,wrongsubmits
	sprintf(query,"update attend set solved=(SELECT count(DISTINCT problem_id) FROM solution WHERE contest_id=%d and username='%s' and verdict=%d and submit_date between '%s' and '%s'),%s_wrongsubmits=(SELECT count(solution_id) FROM solution WHERE contest_id=%d and problem_id=%d and username='%s' and verdict>%d and submit_date between '%s' and '%s'),score=%d  where contest_id=%d and username='%s';",contestId,username,V_AC,s_t.c_str(),e_t.c_str(),   num,contestId,problemId,username,V_AC,s_t.c_str(),fAC_t.c_str(),  score_,  contestId,username);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	//penalty
	int nCountProblems=SQL_countContestProblems(contestId);
	char index='A';
	long penalty=0;
	for(int i=0;i<nCountProblems;i++)
	{
		long a_time_=0;
		int wrongsubmits_=0;
		SQL_getContestAttend(contestId,username,index+i,a_time_,wrongsubmits_);

		if(a_time_>0)
		{
			penalty=penalty+a_time_+wrongsubmits_*60*20;
		}
	}

	sprintf(query,"update attend set penalty=%ld where contest_id=%d and username='%s';",penalty,contestId,username);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}


}

/* update user table*/
void SQL_updateUser(char *username)
{
	sprintf(query,"update users set submit=(SELECT count(*) FROM solution WHERE username='%s') where username='%s';",username,username);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}

	sprintf(query,"update users set solved=(SELECT count(DISTINCT problem_id) FROM solution WHERE username='%s' and verdict=%d) where username='%s';",username,V_AC,username);
	if(mysql_real_query(mysql,query,(unsigned int)strlen(query)))
	{
		write_log(JUDGE_ERROR,mysql_error(mysql));
	}
}


void SQL_updateCompileInfo(int solutionId)
{
	FILE *fp;
	char buffer[4096]={0};

	if ((fp = fopen (DebugFile, "r")) == NULL)
	{
		write_log(JUDGE_ERROR,"DebugFile open error");
		return ;
	}

	//先删除
	sprintf(query,"delete from compile_info  where solution_id=%d;",solutionId);
	mysql_real_query(mysql,query,(unsigned int)strlen(query));

	//先插入
	if((fgets(buffer, 4095, fp))!= NULL)
	{
		sprintf(query,"insert into compile_info values(%d,\"%s\");",solutionId,buffer);
		int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
		if(ret)
		{
			write_log(JUDGE_ERROR,mysql_error(mysql));
			fclose(fp);
			return ;
		}
	}

	//后连接
	while ((fgets (buffer, 4095, fp))!= NULL)
	{
		buffer[strlen(buffer)];
		sprintf(query,"update compile_info set error=CONCAT(error,\"%s\") where solution_id=%d;",buffer,solutionId);

		int ret=mysql_real_query(mysql,query,(unsigned int)strlen(query));
		if(ret)
		{
			write_log(JUDGE_ERROR,mysql_error(mysql));
			fclose(fp);
			return ;
		}
 	}

	fclose(fp);

}

