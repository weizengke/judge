#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#include <csignal>
#include <wait.h>
#include <sys/resource.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include "stdio.h"
int judge_solution_run()
{
    pid_t exec_pid = fork();
    if (exec_pid < 0) {
        printf("solution fork failed. [%s]", strerror(errno));
    } else if (exec_pid == 0) {

        if (0 != chdir("/home/jungle/code/online-judge/judger-kernel/build/linux/bin/temp/7_1706840111/")) {
            exit(5);
        }

        freopen("data1.in", "r", stdin);
        freopen("user.out", "w", stdout);

        if (0 != chroot("/home/jungle/code/online-judge/judger-kernel/build/linux/bin/temp/7_1706840111/")) {
            exit(6);
        }

        execl("./7.exe", "./7.exe", (char *)0); 
  
        exit(0);
    } else {
        int status = 0;
        rusage rused{};
        do {
            if (wait4(exec_pid, &status, 0, &rused) < 0) {
                printf("wait4 executor failed: %s", strerror(errno));
                kill(exec_pid, SIGKILL);
                exit(0);
            }

            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == 0) {
                    printf( "solution run finish.");
                } else {
                    printf("abnormal quit, exit_code: %d", WEXITSTATUS(status));
                }
                break;
            }

            // RE/TLE/OLE
            if (WIFSIGNALED(status) || (WIFSTOPPED(status) && WSTOPSIG(status) != SIGTRAP)) {
                int signo = 0;
                if (WIFSIGNALED(status)) {
                    signo = WTERMSIG(status);
                    printf("child process killed by signal %d, %s", signo, strsignal(signo));
                } else {
                    signo = WSTOPSIG(status);
                    printf("child process stopped by signal %d, %s", signo, strsignal(signo));
                }
                switch (signo) {
                    // Ignore
                    case SIGCHLD:
                        break;
                        // TLE
                    case SIGALRM:    // alarm() and setitimer(ITIMER_REAL)
                    case SIGVTALRM:  // setitimer(ITIMER_VIRTUAL)
                    case SIGXCPU:    // exceeds soft processor limit
                        printf("Time Limit Exceeded: %s", strsignal(signo));
                        break;
                        // OLE
                    case SIGXFSZ:  // exceeds file size limit
                        printf( "Output Limit Exceeded");
                        break;
                        // RE
                    case SIGSEGV:  // segmentation violation
                    case SIGFPE:   // any arithmetic exception
                    case SIGBUS:   // the process incurs a hardware fault
                    case SIGABRT:  // abort() function
                    case SIGKILL:  // exceeds hard processor limit
                        break;
                    default:
                        break;
                } 
                kill(exec_pid, SIGKILL);
                break;
            } 

        }while(0);
    }

    return 0;
}

int main()
{
    judge_solution_run();
    return 0;
}