#ifndef _COMMAND_CORE_H_
#define _COMMAND_CORE_H_

extern VOID vty_offline(ULONG vtyId);	
extern VOID cmd_clear_line(struct cmd_vty *vty);
extern struct cmd_vty *cmd_vty_getById(ULONG vtyId);
extern VOID cmd_vty_init(struct cmd_vty *vty);
extern VOID cmd_vty_deinit(struct cmd_vty *vty);
extern VOID cmd_vty_add_history(struct cmd_vty *vty);
extern VOID cmd_vector_insert_cr(CMD_VECTOR_S *v);

extern CMD_VECTOR_S *cmd_vector_copy(CMD_VECTOR_S *v);
extern CMD_VECTOR_S *cmd_str2vec(CHAR *string);
extern ULONG cmd_vector_fetch(CMD_VECTOR_S *v);
extern CMD_VECTOR_S *cmd_vector_init(ULONG size);
extern VOID cmd_vector_deinit(CMD_VECTOR_S *v, ULONG freeall);

extern ULONG cmd_match_command(CMD_VECTOR_S *icmd_vec, struct cmd_vty *vty,
		struct para_desc **match, ULONG *match_size, CHAR *lcd_str);

extern ULONG cmd_complete_command(CMD_VECTOR_S *icmd_vec, struct cmd_vty *vty,
											struct para_desc **match, ULONG *match_size, ULONG *nomatch_pos);

extern ULONG cmd_execute_command(CMD_VECTOR_S *icmd_vec, struct cmd_vty *vty,
											struct para_desc **match, ULONG *match_size, ULONG *nomatch_pos);

extern VOID cmd_output_missmatch(cmd_vty *vty, ULONG nomath_pos);

extern VOID cmd_insert_word(struct cmd_vty *vty, CHAR *str);
extern VOID cmd_delete_word(struct cmd_vty *vty);
extern VOID cmd_delete_word_ctrl_W(struct cmd_vty *vty);
extern VOID cmd_delete_word_ctrl_W_ex(struct cmd_vty *vty);

extern ULONG cmd_get_idle_vty();
extern VOID cmd_vty_set_terminaldebug(VOID *vty, CHAR isEnable);

extern VOID cmd_read(struct cmd_vty *vty);
extern ULONG cmd_run(struct cmd_vty *vty);
extern VOID vty_view_set(ULONG vtyId, ULONG view_id);
extern view_node_st * cmd_view_lookup(view_node_st *view, ULONG view_id);
extern VOID vty_view_quit(struct cmd_vty *vty);
extern ULONG cmd_view_getaislenth(struct cmd_vty *vty);
extern VOID cmd_outprompt(struct cmd_vty *vty);
extern CHAR* cmd_view_getAis(ULONG view_id);
extern CHAR* cmd_view_getViewName(ULONG view_id);
extern ULONG vty_view_getParentViewId(ULONG view_id);

extern ULONG cmd_resolve(struct cmd_vty *vty);
extern ULONG cmd_resolve_vty(struct cmd_vty *vty);
extern CMD_VECTOR_S *cmd_vector_new();
extern VOID cmd_vector_free(CMD_VECTOR_S **ppVec);
extern ULONG cmd_regelement_new(ULONG cmd_elem_id, CMD_ELEM_TYPE_E cmd_elem_type, CHAR *cmd_name, CHAR *cmd_help, CMD_VECTOR_S *pVec);
extern VOID cmd_install_command(ULONG mid, ULONG cmd_view, CHAR *cmd_string, CMD_VECTOR_S *pVec);
extern ULONG cmd_view_regist(CHAR *view_name, CHAR *view_ais, ULONG view_id, ULONG parent_view_id);

extern ULONG cmd_get_vty_id(VOID *pRunMsg);
extern VOID *cmd_get_elem_by_index(VOID *pRunMsg, ULONG index);
extern ULONG cmd_get_elem_num(VOID *pRunMsg);
extern ULONG cmd_get_elemid(VOID *pElemMsg);
extern CHAR* cmd_get_elem_param(VOID *pElemMsg);
extern ULONG cmd_get_ulong_param(VOID *pElemMsg);
extern VOID cmd_copy_string_param(VOID *pElemMsg, CHAR *param);	
extern ULONG cmd_get_first_elem_tblid(VOID *pRunMsg);
extern ULONG cmd_regcallback(ULONG mId, ULONG (*pfCallBackFunc)(VOID *pRcvMsg));


extern VOID cmd_resolve_filter(struct cmd_vty *vty);
extern VOID cmd_resolve_tab(struct cmd_vty *vty);
extern VOID cmd_resolve_enter(struct cmd_vty *vty);
extern VOID cmd_resolve_quest(struct cmd_vty *vty);
extern VOID cmd_resolve_up(struct cmd_vty *vty);
extern VOID cmd_resolve_down(struct cmd_vty *vty);
extern VOID cmd_resolve_left(struct cmd_vty *vty);
extern VOID cmd_resolve_right(struct cmd_vty *vty);
extern VOID cmd_resolve_delete(struct cmd_vty *vty);
extern VOID cmd_resolve_backspace(struct cmd_vty *vty);
extern VOID cmd_resolve_insert(struct cmd_vty *vty);
extern VOID cmd_resolve_del_lastword(struct cmd_vty *vty);


#endif _COMMAND_CORE_H_
