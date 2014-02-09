#ifndef _COMMAND_CORE_H_
#define _COMMAND_CORE_H_

extern void cmd_clear_line(struct cmd_vty *vty);
extern struct cmd_vty *cmd_vty_init();
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


extern void cmd_clear_line(struct cmd_vty *vty);
extern void cmd_read(struct cmd_vty *vty);


#endif _COMMAND_CORE_H_
