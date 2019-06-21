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

#ifdef _WIN32_
#include <windows.h>
#include <tlhelp32.h>
#include <conio.h>
#include <io.h>
#endif

#ifdef _WIN32_
#include "product/thirdpart32/common/psapi.h"
#include "product/thirdpart32/dbghelp/DbgHelp.h"
#endif

#include "product/thirdpart32/curl/curl.h"
#include "product/thirdpart32/pcre/pcre.h"
#include "product/thirdpart32/mysql/include/mysql.h"
#include "product/thirdpart32/cjson/cJSON.h"

#include "kernel.h"

/* BEGIN: Added by weizengke, 2013/11/15 for debug center*/
#include "product/include/pdt_common_inc.h"
/* END:   Added by weizengke, 2013/11/15 */

#include "osp/common/include/osp_common_def.h"

#include "osp/command/include/icli.h"

#include "osp/util/util.h"

#include "product/main/root.h"

#include "osp/debug/include/debug_center_inc.h"
#include "osp/event/include/event_pub.h"

#include "product/judge/include/judge_def.h"
#include "product/judge/include/judge_type.h"
#include "product/judge/include/judge_util.h"
#include "product/judge/include/judge_var.h"
#include "product/judge/include/judge_io.h"
#include "product/judge/include/judge_sql.h"
#include "product/judge/include/judge_hdu.h"
#include "product/judge/include/judge_guet.h"
#include "product/judge/include/judge_cmd.h"


#endif

