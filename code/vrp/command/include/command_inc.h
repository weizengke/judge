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



#include "..\..\debug\include\debug_center.h"


#include "..\include\command_def.h"
#include "..\include\command_type.h"
#include "..\include\command_var.h"
#include "..\include\command_func.h"
#include "..\include\command_io.h"
#include "..\include\command_adp.h"
#include "..\include\command_core.h"




#endif
