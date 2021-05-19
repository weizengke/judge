#ifndef __CLI_VTY_IO__ 
#define __CLI_VTY_IO__

VOID cmd_clear_line(CMD_VTY_S *vty);
VOID cmd_delete_one(CMD_VTY_S *vty);
VOID cmd_back_one(CMD_VTY_S *vty);
VOID cmd_put_one(CMD_VTY_S *vty, CHAR c);
UCHAR vty_getchar(CMD_VTY_S *vty);
UCHAR cmd_getch();
UCHAR cmd_parse(CMD_VTY_S *vty);

#endif