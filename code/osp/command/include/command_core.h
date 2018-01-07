#ifndef _COMMAND_CORE_H_
#define _COMMAND_CORE_H_

extern void cmd_vty_offline(struct cmd_vty *vty);
extern void cmd_clear_line(struct cmd_vty *vty);
extern void cmd_vty_init(struct cmd_vty *vty);
extern void cmd_vty_deinit(struct cmd_vty *vty);
extern void cmd_vty_add_history(struct cmd_vty *vty);
extern void cmd_vector_insert_cr(cmd_vector_t *v);

extern cmd_vector_t *cmd_vector_copy(cmd_vector_t *v);

extern cmd_vector_t *str2vec(char *string);
extern cmd_vector_t *cmd2vec(char *string, char *doc);
extern int cmd_vector_fetch(cmd_vector_t *v);
extern cmd_vector_t *cmd_vector_init(int size);
extern void cmd_vector_deinit(cmd_vector_t *v, int freeall);

extern int cmd_match_command(cmd_vector_t *icmd_vec, struct cmd_vty *vty,
		struct para_desc **match, int *match_size, char *lcd_str);

extern int cmd_complete_command(cmd_vector_t *icmd_vec, struct cmd_vty *vty,
											struct para_desc **match, int *match_size, int *match_pos);

extern int cmd_execute_command(cmd_vector_t *icmd_vec, struct cmd_vty *vty,
											struct para_desc **match, int *match_size, int *match_pos);

extern void cmd_output_missmatch(cmd_vty *vty, int nomath_pos);

extern void cmd_insert_word(struct cmd_vty *vty, const char *str);
extern void cmd_delete_word(struct cmd_vty *vty);
extern void cmd_delete_word_ctrl_W(struct cmd_vty *vty);
extern void cmd_delete_word_ctrl_W_ex(struct cmd_vty *vty);

extern struct cmd_vty* cmd_get_idlevty();

extern void cmd_clear_line(struct cmd_vty *vty);
extern void cmd_read(struct cmd_vty *vty);
extern int cmd_run(struct cmd_vty *vty);
extern void vty_view_set(struct cmd_vty *vty, VIEW_ID_EN view_id);
extern view_node_st * cmd_view_lookup(view_node_st *view, VIEW_ID_EN view_id);
extern void vty_view_quit(struct cmd_vty *vty);
extern int cmd_view_getaislenth(struct cmd_vty *vty);
extern void cmd_outprompt(struct cmd_vty *vty);
extern char* cmd_view_getAis(VIEW_ID_EN view_id);
extern char* cmd_view_getViewName(VIEW_ID_EN view_id);
extern VIEW_ID_EN vty_view_getParentViewId(VIEW_ID_EN view_id);

#endif _COMMAND_CORE_H_
