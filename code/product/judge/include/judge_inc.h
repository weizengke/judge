#ifndef _JUDGE_INC_H
#define _JUDGE_INC_H

#include <windows.h>
#include <iostream>
#include <conio.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#include <queue>
#include <string>
#include <sstream>

#include "osp\command\include\icli.h"

/* BEGIN: Added by weizengke, 2013/11/15 for debug center*/
#include "product\include\pdt_common_inc.h"
/* END:   Added by weizengke, 2013/11/15 */

#include "osp\common\include\osp_common_def.h"
#include "osp\debug\include\debug_center_inc.h"
#include "osp\event\include\event_pub.h"

#include "product\judge\include\judge_def.h"
#include "product\judge\include\judge_type.h"
#include "product\judge\include\judge_util.h"
#include "product\judge\include\judge_var.h"
#include "product\judge\include\judge_io.h"
#include "product\judge\include\judge_sql.h"
#include "product\judge\include\judge_hdu.h"
#include "product\judge\include\judge_guet.h"
#include "product\judge\include\judge_cmd.h"


#ifdef _WIN32_
#include "product\thirdpart32\common\psapi.h"
#include "product\thirdpart32\curl\curl.h"
#include "product\thirdpart32\pcre\pcre.h"
#include "product\thirdpart32\mysql\include\mysql.h"
#else
#include "product\thirdpart\common\psapi.h"
#include "product\thirdpart\curl\curl.h"
#include "product\thirdpart\pcre\pcre.h"
#include "product\thirdpart\mysql\include\mysql.h"

#endif

//#pragma comment(lib,"ws2_32")
//#pragma comment(lib,"WSOCK32.lib")
//#pragma comment(lib, "..\..\..\..\..\build\lib\pcre.lib")
//#pragma comment(lib,"..\..\..\..\..\build\lib\psapi.lib")
//#pragma comment(lib, "..\..\..\..\..\build\lib\libmysql.lib")


#endif

