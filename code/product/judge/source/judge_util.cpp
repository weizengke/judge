
#include "product/judge/include/judge_inc.h"

#if (OS_YES == OSP_MODULE_JUDGE)

using namespace std;

static int g_DEBUG = 0;

int isInFile(const char fname[]) {
	int l = strlen(fname);
	if (l <= 3 || strcmp(fname + l - 3, ".in") != 0)
		return 0;
	else
		return l - 3;
}

void find_next_nonspace(int & c1, int & c2, FILE *& f1, FILE *& f2, int & ret) {
	// Find the next non-space character or \n.
	while ((isspace(c1)) || (isspace(c2))) {
		if (c1 != c2) {
			if (c2 == EOF) {
				do {
					c1 = fgetc(f1);
				} while (isspace(c1));
				continue;
			} else if (c1 == EOF) {
				do {
					c2 = fgetc(f2);
				} while (isspace(c2));
				continue;
			} else if ((c1 == '\r' && c2 == '\n')) {
				c1 = fgetc(f1);
			} else {
				if (g_DEBUG)
					printf("\r\n%d=%c %d=%c", c1, c1, c2, c2);
				;
				ret = V_PE;
			}
		}
		if (isspace(c1)) {
			c1 = fgetc(f1);
		}
		if (isspace(c2)) {
			c2 = fgetc(f2);
		}
	}
}
/*
 * translated from ZOJ judger r367
 * http://code.google.com/p/zoj/source/browse/trunk/judge_client/client/text_checker.cc#25
 *
 */


int compare_zoj(const char *file1, const char *file2) {
	int ret = V_AC;
	FILE * f1, *f2;
	f1 = fopen(file1, "r");
	f2 = fopen(file2, "r");
	if (!f1 || !f2) {
		ret = V_RE;
	} else
		for (;;) {
			// Find the first non-space character at the beginning of line.
			// Blank lines are skipped.
			int c1 = fgetc(f1);
			int c2 = fgetc(f2);
			find_next_nonspace(c1, c2, f1, f2, ret);
			// Compare the current line.
			for (;;) {
				// Read until 2 files return a space or 0 together.
				while ((!isspace(c1) && c1) || (!isspace(c2) && c2)) {
					if (c1 == EOF && c2 == EOF) {
						goto end;
					}
					if (c1 == EOF || c2 == EOF) {
						break;
					}
					if (c1 != c2) {
						// Consecutive non-space characters should be all exactly the same
						ret = V_WA;
						goto end;
					}
					c1 = fgetc(f1);
					c2 = fgetc(f2);
				}
				find_next_nonspace(c1, c2, f1, f2, ret);
				if (c1 == EOF && c2 == EOF) {
					goto end;
				}
				if (c1 == EOF || c2 == EOF) {
					ret = V_WA;
					goto end;
				}

				if ((c1 == '\n' || !c1) && (c2 == '\n' || !c2)) {
					break;
				}
			}
		}
end: if (f1)
		 fclose(f1);
	 if (f2)
		 fclose(f2);
	 return ret;
}


void delnextline(char s[]) {
	int L;
	L = strlen(s);
	while (L > 0 && (s[L - 1] == '\n' || s[L - 1] == '\r'))
		s[--L] = 0;
}

int compare(const char *file1, const char *file2) {
	//compare ported and improved from zoj don't limit file size
	return compare_zoj(file1, file2);
}

int string_cmp(const void *a, const void*b) {
    char *s1 = (char *)a;
    char *s2 = (char *)b;
	int len1 = strlen(s1);
	int len2 = strlen(s2);
	
	if (len1 == len2)
	{
		return strcmp(s1, s2);
	}
	else 
	{
		if (len1 > len2)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}
}

string GetLocalTimeAsString(const char* format) {
	time_t t = time(NULL);
	struct tm *p;
	p = localtime(&t);
	char buf[1024];
	strftime(buf, sizeof(buf), format, p);
	return buf;
}

string getCurrentTime()
{
	time_t s_t;
	string time_string;

	time(&s_t);
	util_time_to_string(time_string,s_t);

	return time_string;
}


//×Ö·û´®Ìæ»»ËùÓÐold_value->new_value
string&  replace_all_distinct(string&   str,const   string&   old_value,const   string&   new_value)
{
	for(string::size_type   pos(0);   pos!=string::npos;   pos+=new_value.length())   {
		if(   (pos=str.find(old_value,pos))!=string::npos   )
			str.replace(pos,old_value.length(),new_value);
		else   break;
	}
	return   str;
}

#endif
