#ifndef _COMMAND_FUNC_H_
#define _COMMAND_FUNC_H_

extern void debug_print(const char *format, ...);
extern void debug_print_ex(CMD_DEBUG_TYPE_EM type, const char *format, ...);

extern void cmd_outstring(const char *format, ...);
extern void cmd_debug(int level, const char *fname, const char *fmt, ...);

extern void cmd_init();
extern cmd_vector_t *cmd_vector_init(int size);
extern int cmd_reg_newcmdelement(int cmd_elem_id, CMD_ELEM_TYPE_EM cmd_elem_type, const char *cmd_name, const char *cmd_help);
extern void install_element(struct cmd_elem_st *elem);


#endif
