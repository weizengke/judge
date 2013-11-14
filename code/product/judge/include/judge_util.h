#ifndef __UTIL_H__
#define __UTIL_H__

#include <iostream>
#include <string>
#include <time.h>
#include  <io.h>
using namespace std;



int isInFile(const char fname[]) ;
void find_next_nonspace(int & c1, int & c2, FILE *& f1, FILE *& f2, int & ret);
int compare_zoj(const char *file1, const char *file2);
void delnextline(char s[]);
int compare(const char *file1, const char *file2);

int StringToTimeEX(const string &strDateStr,time_t &timeData);
int API_TimeToString(string &strDateStr,const time_t &timeData);
long getdiftime(time_t maxt,time_t mint);

string GetLocalTimeAsString(const char* format);
string getCurrentTime();

string&  replace_all_distinct(string&   str,const   string&   old_value,const   string&   new_value);

#endif /* __UTIL_H__ */