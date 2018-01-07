#ifndef _COMMAND_IO_H_
#define _COMMAND_IO_H_

extern void vty_print2all(const char *format, ...);
extern void vty_printf(struct cmd_vty *vty, const char *format, ...);
extern int cmd_getch();
extern int vty_getch(struct cmd_vty *vty);
extern void cmd_back_one(struct cmd_vty *vty);
extern void cmd_put_one(struct cmd_vty *vty,char c);
extern void cmd_delete_one(struct cmd_vty *vty);
extern void cmd_outstring(const char *format, ...);


#endif _COMMAND_IO_H_

