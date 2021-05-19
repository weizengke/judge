#ifndef _TELNET_H_
#define _TELNET_H_

/* 
通常情况下，客户机向服务器发送字符而服务器将其回显到用户的终端上，但是，如果网络的时延回引起回显速度太慢，用户可能更愿意让本地系统回显字符。在客户机允许本地系统回显前，它要向服务器发送以下序列：
IAC DONT ECHO 服务器收到请求后，发出3个字符的响应：
IAC WONT ECHO 表示服务器已经按请求同意关闭回显。


*/

/*
SE    240(F0)     子选项结束
SB    250(FA)     子选项开始
IAC   255(FF)     选项协商的第一个字节
WILL  251(FB)     发送方激活选项(接收方同意激活选项)
DO    253(FD)     接收方同意（发送方想让接收方激活选项）
WONT  252(FC)     接收方不同意
DONT  254(FE)     接受方回应WONT
*/
#define TEL_SE   0xF0
#define TEL_SB   0xF0
#define TEL_IAC  0xFF
#define TEL_WILL 0xFB
#define TEL_DO   0xFD
#define TEL_WONT 0xFC
#define TEL_DONT 0xFE

/*
1(0x01)    回显(echo)
3(0x03)    抑制继续进行(传送一次一个字符方式可以选择这个选项)
24(0x18)   终端类型
31(0x1F)   窗口大小
32(0x20)   终端速率
33(0x21)   远程流量控制
34(0x22)   行方式
36(0x24)   环境变量
*/
#define TEL_ECHO  0x01


#endif