
#include "..\..\command\include\command_type.h"
#include "..\..\debug\include\debug_center.h"

#define DEFUN(cmdname, cmdstr, helpstr, funcname)  DEFUN_CMD(cmdname, cmdstr, helpstr, funcname)


extern void RunDelay(int t);
extern void pdt_debug_print(const char *format, ...);
extern void pdt_debug_print_ex(int level, const char *format, ...);

extern void MSG_StartDot();
extern void MSG_StopDot();

