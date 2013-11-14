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

void timer()
{
    std::thread::id this_id = std::this_thread::get_id();
    cout<< "Run timer ok.......[" <<this_id<<"]"<<endl;
    for (int i=0; ; i++)
    {
        //pdt_debug_print("Do timer %d",i);
        RunDelay(1000);
    }
}


int main()
{

    std::thread t_msg(MSGQueueThread);
	RunDelay(10);

	std::thread t_timer(timer);
	RunDelay(10);

	extern int cmd_main_entry ();
	std::thread t_cmd(cmd_main_entry);
	RunDelay(10);

	extern void OJ_TaskEntry();
	std::thread t_oj(OJ_TaskEntry);
	RunDelay(1000);

	/* 	×èÈû  */
	t_oj.join();
	t_msg.join();
	t_cmd.join();
	t_timer.join();

	return 0;
}
