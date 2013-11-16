#ifndef _COMMAND_VAR_H_
#define _COMMAND_VAR_H_

extern char *szDebugName[];

extern unsigned long g_aulDebugMask[];
extern int g_debug_switch;

extern char g_sysname[];
extern const char *key_name[];

extern struct cmd_vty *vty;
extern cmd_vector_t *cmd_vec;
extern struct para_desc g_cmd_elem[];

#endif
