#ifndef _PDT_DEBUG_CENTER_H_
#define _PDT_DEBUG_CENTER_H_

#define DEBUG_DISABLE 0
#define DEBUG_ENABLE  1

#ifdef UCHAR
#undef UCHAR
#define UCHAR unsigned char
#endif

#ifndef ULONG
#undef ULONG
#define ULONG unsigned long
#endif

#ifndef CHAR
#undef CHAR
#define CHAR char
#endif

#ifndef VOID
#undef VOID
#define VOID void
#endif
enum DEBUG_TYPE_EM
{
	DEBUG_TYPE_NONE,

	DEBUG_TYPE_ERROR,
	DEBUG_TYPE_FUNC,
	DEBUG_TYPE_INFO,
	DEBUG_TYPE_MSG,

	DEBUG_TYPE_MAX,
};

/* BEGIN:   Added by weizengke, 2013/10/27 */
#define DEBUG_MID_ISVALID(x) (x>MID_NULL && x<MID_ID_END)
#define DEBUG_TYPE_ISVALID(x) (x>DEBUG_TYPE_NONE && x<DEBUG_TYPE_MAX)
#define DEBUG_MASKLENTG 32
#define DEBUG_MASK_GET(mid, x) (( g_aulDebugMask[mid][(x)/DEBUG_MASKLENTG] >> ((x)%DEBUG_MASKLENTG)) & 1)
#define DEBUG_MASK_SET(mid, x) ( g_aulDebugMask[mid][(x)/DEBUG_MASKLENTG] |= ( 1 << (x)%DEBUG_MASKLENTG ) )
#define DEBUG_MASK_CLEAR(mid, x) if (DEBUG_MASK_GET(mid, x)) ( g_aulDebugMask[mid][(x)/DEBUG_MASKLENTG] ^= ( 1 << (x)%DEBUG_MASKLENTG) )
	
/* END:   Added by weizengke, 2013/10/27 */

typedef struct tag_MSGQueue_S {
	MID_ID_EM mid;
	DEBUG_TYPE_EM type;
    char szMsgBuf[BUFSIZE];
	unsigned long thread_id;
	struct tm stTime;

}MSGQUEUE_S;

extern char *szModuleName[];
extern char *szDebugName[];
extern unsigned long g_aulDebugMask[][DEBUG_TYPE_MAX/32 + 1];
extern int g_debug_switch;

#define JUDGE_INFO 0
#define JUDGE_WARNING 1
#define JUDGE_ERROR 2
#define JUDGE_FATAL 3
#define JUDGE_SYSTEM_ERROR 4

extern void RunDelay(int t);
extern void MSGQueueMain();
extern void debugcenter_print(MID_ID_EM mid, DEBUG_TYPE_EM type, const char *format, ...);
extern void pdt_debug_print(const char *format, ...);

extern void MSG_StartDot();
extern void MSG_StopDot();

#define DEBUG_Debug(x, args...) debugcenter_print(MID_DEBUG, x, args)

#endif


