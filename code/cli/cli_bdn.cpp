#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
#include <io.h>
#include <winsock2.h>
#endif

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#ifdef _LINUX_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <assert.h>
#endif

#include "kernel.h"

#include "osp_common_def.h"
#include "icli.h"

#if (OS_YES == OSP_MODULE_CLI)

typedef struct cli_bdn_ntf_node_s
{
	ULONG moduleId;
	ULONG view_id;
	ULONG priority;
	ULONG (*pfCallBackFunc)(CHAR **ppBuildrun, ULONG includeDefault);
	struct cli_bdn_ntf_node_s *pNext;
};
struct cli_bdn_ntf_node_s *g_cli_bdn_ntf_lists;

extern VOID vty_printf(VOID *vty, CHAR *format, ...);

ULONG cli_bdn_regist(ULONG moduleId, ULONG view_id, ULONG  priority,
					 ULONG  (*pfcallback)(CHAR **ppBuildrun, ULONG includeDefault))
{
	struct cli_bdn_ntf_node_s *node_now = NULL;
	struct cli_bdn_ntf_node_s *node_pre = NULL;
	struct cli_bdn_ntf_node_s *node_new = NULL;

	node_new = (struct cli_bdn_ntf_node_s *)malloc(sizeof(struct cli_bdn_ntf_node_s));
	if (NULL == node_new) {
		return OS_ERR;
	}

	node_new->moduleId = moduleId;
	node_new->view_id = view_id;
	node_new->pfCallBackFunc = pfcallback;
	node_new->priority = priority;

	if (NULL == g_cli_bdn_ntf_lists) {
		g_cli_bdn_ntf_lists = node_new;
		node_new->pNext = NULL;
		return OS_OK;
	}

	node_now = g_cli_bdn_ntf_lists;
	node_pre = node_now;

	while (NULL != node_now) {
		if (pfcallback == node_now->pfCallBackFunc) {
			free(node_new);
			return OS_OK;
		}
		
		if (priority > node_now->priority) {
			break;
		}

		node_pre = node_now;
		node_now = node_now->pNext;
	}

	if (NULL == node_now) {
		/* INSERT TAIL*/
		node_pre->pNext = node_new;
		node_new->pNext = NULL;
	} else {
		/* INSERT HEAD */
		node_new->pNext = node_now;
		if (node_now == node_pre) {
			g_cli_bdn_ntf_lists = node_new;
		} else {
			node_pre->pNext = node_new;
		}
	}

	return OS_OK;
}

ULONG cli_bdn_system_buildrun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{
	int index = 0;
	int ret = OS_OK;
	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR *)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun) {
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);

	struct cli_bdn_ntf_node_s *pstHead = g_cli_bdn_ntf_lists;
	while (NULL != pstHead){
		
		pBuildrun = NULL;
		ret = pstHead->pfCallBackFunc(&pBuildrun, ulIncludeDefault);
		if (OS_OK != ret) {
		}

		if (NULL != pBuildrun) {
			if (0 != strlen(pBuildrun)) {
				strcat(*ppBuildrun, pBuildrun);
				strcat(*ppBuildrun, "\r\n#");
			}

			free(pBuildrun);
			pBuildrun = NULL;
		}
		pstHead = pstHead->pNext;
	}

	strcat(*ppBuildrun, "\r\nreturn");
	strcat(*ppBuildrun, "\r\n#");
	return OS_OK;
}

VOID cli_bdn_show(ULONG vtyId, ULONG ulIncludeDefault)
{
	ULONG ulRet = OS_OK;
	CHAR *pBuildrun = NULL;

	ulRet = cli_bdn_system_buildrun(&pBuildrun, ulIncludeDefault);
	if (OS_OK != ulRet) {
		return;
	}

	vty_printf(vtyId, "%s\r\n", pBuildrun);
	free(pBuildrun);
}

VOID cli_bdn_show_by_current_view(ULONG vtyId, ULONG ulIncludeDefault)
{
	int index  = 0;
	int ret = OS_OK;	
	CHAR *pBuildrun = NULL;
	CHAR *pBuildrunTmp = NULL;
	ULONG view_id = vty_get_current_viewid(vtyId);

	pBuildrun = (CHAR *)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == pBuildrun){
		return;
	}
	memset(pBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);
	strcat(pBuildrun, "#");

	struct cli_bdn_ntf_node_s *pstHead = g_cli_bdn_ntf_lists;
	while (NULL != pstHead) {
		if (pstHead->view_id != view_id) {
			pstHead = pstHead->pNext;
			continue;
		}
		
		pBuildrunTmp = NULL;
		ret = pstHead->pfCallBackFunc(&pBuildrunTmp, ulIncludeDefault);
		if (OS_OK != ret) {
		}

		if (NULL != pBuildrunTmp) {
			if (0 != strlen(pBuildrunTmp)) {
				strcat(pBuildrun, pBuildrunTmp);
				strcat(pBuildrun, "\r\n#");
			}

			free(pBuildrunTmp);
			pBuildrunTmp = NULL;
		}

		pstHead = pstHead->pNext;
	}

	strcat(pBuildrun, "\r\nreturn");
	strcat(pBuildrun, "\r\n#");
	vty_printf(vtyId, "%s\r\n", pBuildrun);
	free(pBuildrun);
}

#endif

