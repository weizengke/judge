#include <iostream>
#include <windows.h>
#include <process.h>
#include <stdlib.h>
//#include <thread>
#include <string>
//#include <mutex>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <queue>

#include "..\include\pdt_common_inc.h"

using namespace std;


int main()
{

	pdt_debug_print("OS Main-task Running...");

    extern void MSGQueueMain(void *);
//	std::thread t_debug(MSGQueueMain);
	_beginthread(MSGQueueMain,0,NULL);
	RunDelay(10);

	extern void OJ_TaskEntry(void *);
//	std::thread t_oj(OJ_TaskEntry);
	_beginthread(OJ_TaskEntry,0,NULL);

	RunDelay(10);

	extern void cmd_main_entry (void *);
//	std::thread t_cmd(cmd_main_entry);

	_beginthread(cmd_main_entry,0,NULL);

	RunDelay(10);


	pdt_debug_print("OS Main-task init ok...");


//	t_debug.join();
//	t_oj.join();

	/* 	×èÈû  */
//	t_cmd.join();
	//t_test.join();

	while(1)
	{
		RunDelay(1);
	}


	return 0;
}
