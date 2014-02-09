#ifndef _COMMAND_IO_H_
#define _COMMAND_IO_H_

extern void debug_print_ex(CMD_DEBUG_TYPE_EM type, const char *format, ...);
extern void debug_print(const char *format, ...);
extern int cmd_getch();



extern void cmd_outprompt(char *prompt);
extern void cmd_debug(int level, const char *fname, const char *fmt, ...);

extern void cmd_back_one();
extern void cmd_put_one(char c);

extern void cmd_outstring(const char *format, ...);



#endif _COMMAND_IO_H_

