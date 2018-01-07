#ifndef _COMMAND_VAR_H_
#define _COMMAND_VAR_H_


extern char g_sysname[];
extern const char *key_name[];
extern view_node_st *view_list;
extern struct cmd_vty g_vty[];
extern struct cmd_vty *g_con_vty; /* ´®¿ÚÓÃ»§ */

extern cmd_vector_t *cmd_vec;
extern struct para_desc g_cmd_elem[];

extern key_handler_t key_resolver[];


#endif
