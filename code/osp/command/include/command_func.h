#ifndef _COMMAND_FUNC_H_
#define _COMMAND_FUNC_H_

extern void cmd_outstring(const char *format, ...);

extern void cmd_install();
extern cmd_vector_t *cmd_vector_init(int size);
extern int cmd_reg_newcmdelement(int cmd_elem_id, CMD_ELEM_TYPE_EM cmd_elem_type, const char *cmd_name, const char *cmd_help);
extern void install_command(VIEW_ID_EN view, struct cmd_elem_st *elem);

extern char INI_filename[];

#define CMD_debug(x, args...) debugcenter_print(MID_CMD, x, args)


#endif
