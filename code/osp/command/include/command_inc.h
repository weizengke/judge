#ifndef _COMMAND_INC_H_
#define _COMMAND_INC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <stdarg.h>
#include <assert.h>

#ifdef _LINUX_
#include <curses.h>
#include <termios.h>
#include <unistd.h>
#else
#include <conio.h>
#include <io.h>
#include <windows.h>

#endif


#include "osp\common\include\osp_common_def.h"
#include "osp\debug\include\debug_center.h"


#include "osp\command\include\command_def.h"
#include "osp\command\include\command_type.h"
#include "osp\command\include\command_var.h"
#include "osp\command\include\command_func.h"
#include "osp\command\include\command_io.h"
#include "osp\command\include\command_adp.h"
#include "osp\command\include\command_core.h"




#endif
