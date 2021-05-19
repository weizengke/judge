#include "judge/include/judge_inc.h"

#ifdef _LINUX_
#include <csignal>
#include <wait.h>
#include <sys/resource.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/user.h>
#include <sys/ptrace.h>

#if (OS_YES == OSP_MODULE_JUDGE)
using namespace std;

int judge_get_testcases(JUDGE_SUBMISSION_S *submission, char szTestcases[JUDGE_MAX_CASE][UTIL_MAX_FNAME])
{   
    char szPath[UTIL_MAX_PATH] = {0};
    int caseNum = 0;
	DIR *dir;
	struct dirent *ptr;

	if (submission->mode == JUDGE_TEST_MODE) {
		sprintf(szTestcases[caseNum++], "data");
		return caseNum;
	}

    sprintf(szPath, "%s\/%d", g_judge_testcase_path, submission->problem.problemId);

	if ((dir = opendir(szPath)) == NULL) {
		return caseNum;
	}
 
	while ((ptr = readdir(dir)) != NULL) {
		if(ptr->d_type != 8) { /* file */
            continue;
		}

		char drive[UTIL_MAX_DRIVE] = {0};
		char dir[UTIL_MAX_DIR] = {0};
		char fname[UTIL_MAX_FNAME] = {0};
		char ext[UTIL_MAX_EXT] = {0};

		util_splitpath(ptr->d_name, drive, dir, fname, ext);
        if (0 != strcmp(ext, ".in")) {
            continue;
        }

        /* out file must exist. */
        char outFileName[UTIL_MAX_FNAME] = {0};
        sprintf(outFileName, "%s\/%d\/%s.out", g_judge_testcase_path, submission->problem.problemId, fname);
        if ((file_access(outFileName, 0 )) == -1) {
            continue;
	    }

		sprintf(szTestcases[caseNum++], "%s", fname);
	}

	closedir(dir);

    qsort(szTestcases, caseNum, sizeof(char)*UTIL_MAX_FNAME, string_cmp);

    return caseNum;
}

void judge_solution_memory_check(JUDGE_SUBMISSION_S *submission)
{    
    if (submission->solution.memory_cur < submission->rused.ru_maxrss) {
        submission->solution.memory_cur = submission->rused.ru_maxrss;
        if (submission->solution.memory_cur > submission->solution.memory_used) {
            submission->solution.memory_used = submission->solution.memory_cur;
        }                
    } 
    
    if (submission->solution.memory_cur > submission->problem.memory_limit) {
        submission->solution.verdictId = V_MLE;
        submission->solution.memory_used = submission->problem.memory_limit;
    }
}

void judge_solution_exit_check(JUDGE_SUBMISSION_S *submission)
{
    int status = submission->status;
    write_log(JUDGE_INFO, "solution exit: status=%d, WIFEXITED:%d, "
                "WEXITSTATUS:%d, WIFSIGNALED:%d, WIFSTOPPED:%d, WSTOPSIG:%d SIGTRAP=%d.", 
                status, WIFEXITED(status), WEXITSTATUS(status), WIFSIGNALED(status),
                WIFSTOPPED(status), WSTOPSIG(status), SIGTRAP);

    /* check exit normally */
    if (WIFEXITED(status)) {
        if (WEXITSTATUS(status) == 0) {
            write_log(JUDGE_INFO, "solution run finish.");
        } else {
            if (WEXITSTATUS(status) == 11) {
                return;
            }
            write_log(JUDGE_ERROR, "abnormal quit, exit_code: %d", WEXITSTATUS(status));
        }
        return;
    }

    // RE/TLE/OLE
    if (WIFSIGNALED(status) || (WIFSTOPPED(status) && WSTOPSIG(status) != SIGTRAP)) {
        int signo = 0;
        if (WIFSIGNALED(status)) {
            signo = WTERMSIG(status);
            write_log(JUDGE_ERROR, "child process killed by signal %d, %s", signo, strsignal(signo));
        } else {
            signo = WSTOPSIG(status);
            write_log(JUDGE_INFO, "child process stopped by signal %d, %s", signo, strsignal(signo));
        }

        switch (signo) {
            // Ignore
            case SIGCHLD:
                write_log(JUDGE_ERROR, "SIGCHLD");
                break;
                // TLE
            case SIGALRM:    // alarm() and setitimer(ITIMER_REAL)
            case SIGVTALRM:  // setitimer(ITIMER_VIRTUAL)
            case SIGXCPU:    // exceeds soft processor limit
                write_log(JUDGE_ERROR, "Time Limit Exceeded: %s", strsignal(signo));
                submission->solution.verdictId = V_OLE;
                break;
                // OLE
            case SIGXFSZ:  // exceeds file size limit
                write_log(JUDGE_ERROR, "Output Limit Exceeded: %s", strsignal(signo));
                submission->solution.verdictId = V_OLE;
                break;
                // RE
            case SIGSEGV:  // segmentation violation
            case SIGFPE:   // any arithmetic exception
            case SIGBUS:   // the process incurs a hardware fault
            case SIGABRT:  // abort() function
            case SIGKILL:  // exceeds hard processor limit
                write_log(JUDGE_ERROR, "Runtime Error: %s", strsignal(signo));
                submission->solution.verdictId = V_RE;
                break;
            default:
                write_log(JUDGE_ERROR,"signo default: %s", strsignal(signo));
                break;
        } 

        kill(submission->exec_pid, SIGKILL);
        return;
    } 

    return;
}

void judge_solution_time_check(JUDGE_SUBMISSION_S *submission)
{
    long sec = submission->rused.ru_utime.tv_sec + submission->rused.ru_stime.tv_sec;
    long usec = submission->rused.ru_utime.tv_usec + submission->rused.ru_stime.tv_usec;
    usec += sec * 1000000;
    usec /= 1000;
    submission->solution.time_cur = usec;

    if (submission->solution.time_cur < 0) {
        submission->solution.time_cur = submission->problem.time_limit;
    }

    /* more than time_used */
    if (submission->solution.time_cur > submission->solution.time_used) {
        submission->solution.time_used = submission->solution.time_cur;
    }
    
    /* more than time_limit */
    if (submission->solution.time_used >= submission->problem.time_limit) {
        submission->solution.verdictId = V_TLE;
        submission->solution.time_used = submission->problem.time_limit;
    }
}

int judge_set_limit(JUDGE_SUBMISSION_S *submission) 
{
    rlimit lim{};

    lim.rlim_max = (submission->problem.time_limit + STD_T_LIM * 1000) / 1000 + 1;
    lim.rlim_cur = lim.rlim_max;
    if (setrlimit(RLIMIT_CPU, &lim) < 0) {
        write_log(JUDGE_ERROR, "setrlimit RLIMIT_CPU failed: %s", strerror(errno));
        return OS_ERR;
    }

    lim.rlim_cur = lim.rlim_max = STD_M_LIM + submission->problem.memory_limit * STD_KB;
    if (setrlimit(RLIMIT_AS, &lim) < 0) {
        write_log(JUDGE_ERROR, "setrlimit RLIMIT_AS failed: %s", strerror(errno));
        return OS_ERR;
    }

    lim.rlim_cur = lim.rlim_max = 65536 * STD_KB;
    if (setrlimit(RLIMIT_STACK, &lim) < 0) {
        write_log(JUDGE_ERROR, "setrlimit RLIMIT_STACK failed: %s", strerror(errno));
        return OS_ERR;
    }

    lim.rlim_cur = lim.rlim_max = (rlim_t) (STD_F_LIM);
    if (setrlimit(RLIMIT_FSIZE, &lim) < 0) {
        write_log(JUDGE_ERROR, "setrlimit RLIMIT_FSIZE failed: %s", strerror(errno));
        return OS_ERR;
    }

    write_log(JUDGE_INFO, "setrlimit ok");

    return OS_OK;
}

int judge_chroot(JUDGE_SUBMISSION_S *submission) {

    char cwd[256];
    char *tmp = getcwd(cwd, 256 - 1);
    if (tmp == nullptr) {
        return OS_ERR;
    }

    if (0 != chdir(submission->subPath)) {
        return OS_ERR;
    }
    
    return OS_OK;
}
int judge_solution_run(JUDGE_SUBMISSION_S *submission)
{
    char cmd[512] = {0};
    sprintf(cmd, "%s < %s > %s 2> %s", 
            submission->runCmd, submission->inFileName, submission->outFileName, submission->ErrorFile);

    write_log(JUDGE_INFO, "run %s", cmd);
    
    pid_t exec_pid = fork();
    if (exec_pid < 0) {
        write_log(JUDGE_ERROR,"solution fork failed. [%s]", strerror(errno));
    } else if (exec_pid == 0) {
        if (OS_OK != judge_set_limit(submission)) {
            exit(3);
        }

        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
            write_log(JUDGE_ERROR, "PTRACE_TRACEME failed: %s", strerror(errno));
            exit(4);
        }

#if 0
        char cwd[256];
        char *tmp = getcwd(cwd, 255);
        if (tmp == nullptr) {
            printf("getcwd failed: %s", strerror(errno));
            exit(5);
        }
        char root_path[256];
        sprintf(root_path, "%s\/%s", cwd, submission->workPath);
        if (0 != chdir(root_path)) {
            printf("chroot(%s) failed: %s", root_path, strerror(errno));
            exit(6);
        }
        // chroot, current directory will be the root dir
        if (0 != chroot(root_path)) {
            printf("chroot(%s) failed: %s", root_path, strerror(errno));
            exit(7);
        }
#endif
        system(cmd); 
  
        exit(11);
    } else {
        int status = 0;
        do {
            if (wait4(exec_pid, &status, 0, &submission->rused) < 0) {
                write_log(JUDGE_ERROR, "wait4 executor failed: %s", strerror(errno));
                kill(exec_pid, SIGKILL);
                exit(0);
            }

            submission->exec_pid = exec_pid;
            submission->status = status;
        }while(0);
    }

    return 0;
}

int judge_compile_run(JUDGE_SUBMISSION_S *submission)
{
    pid_t pid = fork();
    if (pid < 0) {
        write_log(JUDGE_ERROR,"compile fork err.");
    } else if (pid == 0) {
        /* set compile time limit */
        rlimit lim{};
        lim.rlim_cur = lim.rlim_max = 10;
        if (setrlimit(RLIMIT_CPU, &lim) < 0) {
            write_log(JUDGE_ERROR,"setrlimit RLIMIT_CPU failed: %s", strerror(errno));
            return OS_ERR;            
        }

        write_log(JUDGE_INFO,"compile thread start. (solutionId=%u)", submission->solution.solutionId);
        system(submission->compileCmd);
        write_log(JUDGE_INFO,"compile thread end. (solutionId=%u)", submission->solution.solutionId);
        exit(0);
    } else {
        int status = 0;
        if (waitpid(pid, &status, WUNTRACED) == -1) {
            Judge_Debug(DEBUG_TYPE_ERROR, "waitpid for compiler failed: %s", strerror(errno));
            return OS_ERR;  
        }

        if (WIFEXITED(status)) {
            Judge_Debug(DEBUG_TYPE_INFO, "compile succeeded");
        } else {
            if (WIFSIGNALED(status)) {
                Judge_Debug(DEBUG_TYPE_ERROR, "compile kill by %s", strsignal(WTERMSIG(status)));
            } 
        }
    }

	return OS_OK;
}
#endif

#endif