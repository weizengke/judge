#ifndef _COMMAND_INC_H_
#define _COMMAND_INC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#ifdef _LINUX_
#include <curses.h>
#include <termios.h>
#include <unistd.h>
#else
#include <windows.h>
#include <conio.h>
#include <io.h>
#endif


#include "osp_common_def.h"
#include "debug\include\debug_center.h"

#include "../../include/icli.h"
#include "cli_def.h"
#include "cli_type.h"
#include "cli_core.h"

#endif
