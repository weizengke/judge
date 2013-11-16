#include <iostream>
#include <windows.h>
#include <stdlib.h>
#include <thread>
#include <string>
#include <mutex>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <queue>

#include "..\include\pdt_common_inc.h"

using namespace std;


int main()
{

#if 1
    extern void MSGQueueMain();
	std::thread t_debug(MSGQueueMain);
	RunDelay(10);
	pdt_debug_print("Debug task init ok...");

	extern void OJ_TaskEntry();
	std::thread t_oj(OJ_TaskEntry);
	RunDelay(10);
	pdt_debug_print("OJ task init ok...");

	extern int cmd_main_entry ();
	std::thread t_cmd(cmd_main_entry);
	RunDelay(10);


#if 0
	extern int test_main();
	std::thread t_test(test_main);
#endif

	//pdt_debug_print("JungleOS init has been finished...");
	t_debug.join();
	t_oj.join();

	/* 	×èÈû  */
	t_cmd.join();
	//t_test.join();
#endif
	return 0;
}
