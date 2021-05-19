#ifndef _CLI_VTY_H_
#define _CLI_VTY_H_

CMD_VTY_S *cmd_vty_new(ULONG vtyId, CHAR *name, ULONG type);
VOID cmd_vty_init(CMD_VTY_S *vty);

#endif