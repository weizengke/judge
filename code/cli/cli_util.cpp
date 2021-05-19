#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef WIN32
#if CYGWIN
#include <ncurses.h>
#else
#include <conio.h>
#endif
#include <io.h>
#include <winsock2.h>
#endif

#ifdef _LINUX_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <assert.h>
#endif

#include "icli.h"
#include "cli_def.h"
#include "cli_type.h"

CHAR *cmd_util_strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    void *str = malloc(len);

    if (str == NULL){
        return NULL;
    }

    return (CHAR *)memcpy(str, s, len);
}

BOOL cmd_string_isdigit(CHAR *string)
{
	ULONG i = 0;
	if (string == NULL) {
		return FALSE;
	}

	for (i = 0; i < (int)strlen(string); i++) {
		if (!isdigit(*(string + i))) {
			return FALSE;
		}
	}

	return TRUE;
}

ULONG cmd_ip_string_to_ulong(CHAR *ip)
{
	ULONG re = 0;
	UCHAR tmp = 0;

	while (1) {
		if (*ip != '\0' && *ip != '.') {
			tmp = tmp * 10 + *ip - '0';
		} else {
			re = (re << 8) + tmp;
			if (*ip == '\0') {
				break;
			}
			tmp = 0;
		}
		ip++;
	}

	return re;
}

VOID cmd_ip_ulong_to_string(ULONG ip, CHAR *buf)
{
	sprintf(buf, "%u.%u.%u.%u",
			(UCHAR) * ((CHAR *)&ip + 3),
			(UCHAR) * ((CHAR *)&ip + 2),
			(UCHAR) * ((CHAR *)&ip + 1),
			(UCHAR) * ((CHAR *)&ip + 0));

	return;
}

BOOL cmd_string_is_ip(CHAR *str)
{
	ULONG ulLen = 0;
	ULONG aulArr[4] = {0};
	ULONG ulLoop = 0;
	ULONG ulIndex = 0;
	ULONG ulValue = 0;
	ULONG ulIsDotOrEnd = 0;

	if (NULL == str) {
		return FALSE;
	}

	ulLen = strlen(str);
	if (ulLen < 7 || ulLen > 15) {
		return FALSE;
    }

	for (ulLoop = 0; ulLoop < ulLen; ulLoop++) {
		if ('.' == str[ulLoop]) {
			/* 连续 '.' */
			if (1 == ulIsDotOrEnd) {
				return FALSE;
			}

			ulIsDotOrEnd = 1;
			aulArr[ulIndex++] = ulValue;
			ulValue = 0;
		} else {
			ulIsDotOrEnd = 0;

			/* not digit */
			if (!isdigit(str[ulLoop])) {
				return FALSE;
			}

			ulValue = ulValue * 10 + (str[ulLoop] - '0');

			if (ulValue > 255) {
				return FALSE;
			}
		}
	}

	if (0 == ulIsDotOrEnd) {
		aulArr[ulIndex++] = ulValue;
		ulValue = 0;
	}

	if (4 != ulIndex) {
		return FALSE;
	}

	return TRUE;
}
