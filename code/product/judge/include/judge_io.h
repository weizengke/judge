#ifndef _JUDGE_IO_H_
#define _JUDGE_IO_H_


extern void write_log(int level, const char *fmt, ...);

extern void MSG_OUPUT_DBG(const char *fmt, ...);

extern void judge_outstring(const char *format, ...);

extern int reset_file(const char *filename);
extern int read_buffer(const char *filename, char * buffer, int buf_size);
extern int write_buffer(const char *filename, const char *fmt, ...);


extern void Judge_PushQueue(int solutionId);


#endif
