#ifndef _COMMAND_CORE_H_
#define _COMMAND_CORE_H_

extern ULONG cmd_ip_string_to_ulong(CHAR *ip);
extern VOID cmd_ip_ulong_to_string(ULONG ip, CHAR *buf);
extern ULONG cmd_string_is_ip(CHAR *str);

extern VOID vty_offline(ULONG vtyId);	
extern VOID cmd_clear_line(CMD_VTY_S *vty);
extern CMD_VTY_S *cmd_vty_getById(ULONG vtyId);
extern VOID cmd_vty_init(CMD_VTY_S *vty);
extern VOID cmd_vty_add_history(CMD_VTY_S *vty);
extern VOID cmd_vector_insert_cr(CMD_VECTOR_S *v);

extern CMD_VECTOR_S *cmd_vector_copy(CMD_VECTOR_S *v);
extern CMD_VECTOR_S *cmd_str2vec(CHAR *string);
extern CMD_VECTOR_S *cmd_vector_init();
extern VOID cmd_vector_deinit(CMD_VECTOR_S *v, ULONG freeall);

extern ULONG cmd_match_command(CMD_VECTOR_S *icmd_vec, CMD_VTY_S *vty,
		CMD_ELMT_S **ppstMatchCmdElem, ULONG *pulMatchNum);

extern ULONG cmd_complete_command(CMD_VECTOR_S *icmd_vec, CMD_VTY_S *vty,
											CMD_ELMT_S **ppstCmdElem, ULONG *pulMatchNum, ULONG *pulNoMatchPos);

extern ULONG cmd_execute_command(CMD_VECTOR_S *icmd_vec, CMD_VTY_S *vty,
											CMD_ELMT_S **ppstCmdElem, ULONG *pulMatchNum, ULONG *pulNoMatchPos);

extern VOID cmd_output_missmatch(CMD_VTY_S *vty, ULONG ulNoMatchPos);

extern VOID cmd_insert_word(CMD_VTY_S *vty, CHAR *str);
extern VOID cmd_delete_word(CMD_VTY_S *vty);
extern VOID cmd_delete_word_ctrl_W(CMD_VTY_S *vty);
extern VOID cmd_delete_word_ctrl_W_ex(CMD_VTY_S *vty);

extern ULONG cmd_get_idle_vty();
extern VOID cmd_vty_set_terminaldebug(VOID *vty, CHAR isEnable);

extern VOID cmd_read(CMD_VTY_S *vty);
extern ULONG cmd_run(CMD_VTY_S *vty);
extern VOID vty_view_set(ULONG vtyId, ULONG view_id);
extern VIEW_NODE_S * cmd_view_lookup(VIEW_NODE_S *view, ULONG view_id);
extern VOID vty_view_quit(CMD_VTY_S *vty);
extern ULONG cmd_view_getaislenth(CMD_VTY_S *vty);
extern VOID cmd_outprompt(CMD_VTY_S *vty);
extern CHAR* cmd_view_getAis(ULONG view_id);
extern CHAR* cmd_view_getViewName(ULONG view_id);
extern ULONG vty_view_getParentViewId(ULONG view_id);

extern ULONG cmd_resolve(CMD_VTY_S *vty);
extern ULONG cmd_resolve_vty(CMD_VTY_S *vty);
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
extern ULONG cmd_regcallback(ULONG ulMid, ULONG (*pfCallBackFunc)(VOID *pRcvMsg));


extern VOID cmd_resolve_filter(CMD_VTY_S *vty);
extern VOID cmd_resolve_tab(CMD_VTY_S *vty);
extern VOID cmd_resolve_enter(CMD_VTY_S *vty);
extern VOID cmd_resolve_quest(CMD_VTY_S *vty);
extern VOID cmd_resolve_up(CMD_VTY_S *vty);
extern VOID cmd_resolve_down(CMD_VTY_S *vty);
extern VOID cmd_resolve_left(CMD_VTY_S *vty);
extern VOID cmd_resolve_right(CMD_VTY_S *vty);
extern VOID cmd_resolve_delete(CMD_VTY_S *vty);
extern VOID cmd_resolve_backspace(CMD_VTY_S *vty);
extern VOID cmd_resolve_insert(CMD_VTY_S *vty);
extern VOID cmd_resolve_del_lastword(CMD_VTY_S *vty);

extern ULONG cmd_get_command_string(CMD_LINE_S *pstLine, CHAR *pszCmdString, int iSize);

#endif _COMMAND_CORE_H_
