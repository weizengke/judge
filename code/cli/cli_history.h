#ifndef _CLI_HISTORY_H_
#define _CLI_HISTORY_H_

#include "icli.h"

VOID cmd_vty_add_history(CMD_VTY_S *vty);
void cmd_vty_free_histories(CMD_VTY_S *vty);
CHAR *cmd_vty_get_prev_history(CMD_VTY_S *vty);
CHAR *cmd_vty_get_next_history(CMD_VTY_S *vty);
VOID cmd_vty_show_history(CMD_VTY_S *vty, ULONG num);
	
#endif