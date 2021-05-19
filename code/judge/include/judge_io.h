#ifndef _JUDGE_IO_H_
#define _JUDGE_IO_H_


extern void write_log(int level, const char *fmt, ...);

extern void MSG_OUPUT_DBG(const char *fmt, ...);

extern void judge_outstring(const char *format, ...);

extern void judge_request_enqueue(int vtyId, int solutionId);


#endif
