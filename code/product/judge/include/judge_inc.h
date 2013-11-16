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


/* BEGIN: Added by weizengke, 2013/11/15 for debug center*/
#include "..\..\include\pdt_common_inc.h"
/* END:   Added by weizengke, 2013/11/15 */


#include "..\include\judge_def.h"
#include "..\include\judge_util.h"
#include "..\include\judge_var.h"
#include "..\include\judge_io.h"
#include "..\include\judge_sql.h"
#include "..\include\judge_hdu.h"



#include "..\..\thirdpart\common\psapi.h"
#include "..\..\thirdpart\curl\curl.h"
#include "..\..\thirdpart\pcre\pcre.h"

#include "..\..\thirdpart\mysql\include\mysql.h"

#pragma comment(lib,"ws2_32")
#pragma comment(lib,"WSOCK32.lib")
#pragma comment(lib, "..\..\..\..\..\build\lib\pcre.lib")
#pragma comment(lib,"..\..\..\..\..\build\lib\psapi.lib")
#pragma comment(lib, "..\..\..\..\..\build\lib\libmysql.lib")


#endif

