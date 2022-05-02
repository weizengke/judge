#ifndef _JUDGE_INC_H
#define _JUDGE_INC_H

#include <iostream>
#include <sstream>
#include <list>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#ifdef _LINUX_
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#endif

#ifdef WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <io.h>
#endif

#ifdef WIN32
#include "common/psapi.h"
#include "dbghelp/DbgHelp.h"
#endif

#include "curl/curl.h"
#include "pcre/pcre.h"
#include "mysql/include/mysql.h"
#include "cjson/cJSON.h"

#include "kernel.h"
#include "securec.h"

/* BEGIN: Added by weizengke, 2013/11/15 for debug center*/
#include "include/pdt_common_inc.h"
/* END:   Added by weizengke, 2013/11/15 */

#include "osp_common_def.h"

#include "../../include/icli.h"

#include "util/util.h"

#include "root.h"

#include "ic/include/debug_center_inc.h"
#include "event/include/event_pub.h"
#include "sysmng/config.h"

#include "judge/include/judge_def.h"
#include "judge/include/judge_type.h"
#include "judge/include/judge_util.h"
#include "judge/include/judge_var.h"
#include "judge/include/judge_io.h"
#include "judge/include/judge_sql.h"
#include "judge/include/judge_hdu.h"
#include "judge/include/judge_guet.h"
#include "judge/include/judge_leetcode.h"
#include "judge/include/judge_codeforces.h"
#include "judge/include/judge_cmd.h"
#include "judge/include/judge.h"

#include "ftp/ftp_client.h"

#endif

