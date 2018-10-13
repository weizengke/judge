#ifndef __JUDGE_UTIL_H__
#define __JUDGE_UTIL_H__

#include <iostream>
#include <string>
#include <time.h>
using namespace std;



int isInFile(const char fname[]) ;
void find_next_nonspace(int & c1, int & c2, FILE *& f1, FILE *& f2, int & ret);
int compare_zoj(const char *file1, const char *file2);
void delnextline(char s[]);
int compare(const char *file1, const char *file2);
int string_cmp(const void *a, const void*b);

string GetLocalTimeAsString(const char* format);
string getCurrentTime();

string&  replace_all_distinct(string&   str,const   string&   old_value,const   string&   new_value);

#endif /* __JUDGE_UTIL_H__ */