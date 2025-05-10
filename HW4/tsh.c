/* 
 * tsh - A tiny shell program with job control
 * 
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {
        /* Read command line */
        if (emit_prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
            app_error("fgets error");
        if (feof(stdin)) { /* End of file (ctrl-d) */
            fflush(stdout);
            exit(0);
        }

        /* Evaluate the command line */
        eval(cmdline);
        fflush(stdout);
        fflush(stdout);
    }

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline)
{
    char *argv[MAXARGS];         // 参数数组
    int bg;                      // 是否后台
    pid_t pid;
    sigset_t mask_all, mask_one, prev_one;

    bg = parseline(cmdline, argv);     // 解析命令行
    if (argv[0] == NULL) return;       // 空命令行，直接返回

    if (!builtin_cmd(argv)) {
        // Step 1: 屏蔽 SIGCHLD，防止竞争

        if (sigemptyset(&mask_one) < 0) {
            unix_error("sigemptyset error");
        }
        if (sigaddset(&mask_one, SIGCHLD) < 0) {
            unix_error("sigaddset error");
        }
        if (sigprocmask(SIG_BLOCK, &mask_one, &prev_one) < 0) {
            unix_error("sigprocmask block error");
        }

        // Step 2: 创建子进程
        if ((pid = fork()) < 0) {
            unix_error("fork error");
        }

        if (pid == 0) {  // 子进程
            if (sigprocmask(SIG_SETMASK, &prev_one, NULL) < 0) {
                unix_error("sigprocmask restore error in child");
            }
            if (setpgid(0, 0) < 0) {
                unix_error("setpgid error");
            }

            if (execve(argv[0], argv, environ) < 0) {
                printf("%s: Command not found.\n", argv[0]);
                exit(1);
            }
        }

        // Step 3: 父进程添加 job
        if (sigfillset(&mask_all) < 0) {
            unix_error("sigfillset error");
        }
        if (sigprocmask(SIG_BLOCK, &mask_all, NULL) < 0) {
            unix_error("sigprocmask block all error");
        }

        if (!addjob(jobs, pid, bg ? BG : FG, cmdline)) {
            unix_error("addjob error");
        }

        if (sigprocmask(SIG_SETMASK, &prev_one, NULL) < 0) {
            unix_error("sigprocmask restore error");
        }

        // Step 4: 判断前台/后台执行
        if (!bg) {
            waitfg(pid);  // 阻塞直到任务结束
        } else {
            int jid = pid2jid(pid);
            printf("[%d] (%d) %s", jid, pid, cmdline);
        }
    }
}



/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	argv[--argc] = NULL;
    }
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately. Returns 1 if the command was built-in, 0 otherwise.
 */
int builtin_cmd(char **argv) {
    if (argv[0] == NULL)
        return 1;

    if (!strcmp(argv[0], "quit")) {  // 退出 shell
        exit(0);
    }

    if (!strcmp(argv[0], "jobs")) {  // 列出所有作业
        listjobs(jobs);
        return 1;
    }

    if (!strcmp(argv[0], "bg") || !strcmp(argv[0], "fg")) {
        do_bgfg(argv);
        return 1;
    }

    if (!strcmp(argv[0], "&"))  // 忽略单独的 "&"
        return 1;

    return 0;  // 不是内建命令，交由 eval 执行
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands.
 *   - bg <job>: resumes a stopped background job
 *   - fg <job>: brings a background/stopped job to foreground
 */
void do_bgfg(char **argv)
{
    struct job_t *job = NULL;
    int jid;
    pid_t pid;

    // 检查是否有参数
    if (argv[1] == NULL) {
        printf("%s command requires PID or %%jobid argument\n", argv[0]);
        return;
    }

    // 处理 %jobid 形式
    if (argv[1][0] == '%') {
        jid = atoi(&argv[1][1]);
        job = getjobjid(jobs, jid);
        if (job == NULL) {
            printf("%s: No such job\n", argv[1]);
            return;
        }
    }
    // 处理 PID 形式
    else if (isdigit(argv[1][0])) {
        pid = atoi(argv[1]);
        job = getjobpid(jobs, pid);
        if (job == NULL) {
            printf("(%d): No such process\n", pid);
            return;
        }
    }
    // 非法格式
    else {
        printf("%s: argument must be a PID or %%jobid\n", argv[0]);
        return;
    }

    // 恢复进程（包括 stopped 和 running 状态）
    if (kill(-job->pid, SIGCONT) < 0) {
        unix_error("kill (SIGCONT) error");
    }

    if (!strcmp(argv[0], "fg")) {
        job->state = FG;
        waitfg(job->pid);  // 等待前台进程结束
    } else {  // bg
        job->state = BG;
        printf("[%d] (%d) %s", job->jid, job->pid, job->cmdline);
    }
}


/*
 * waitfg - Busy-wait until the specified PID is no longer the foreground job.
 */
void waitfg(pid_t pid)
{
    while (pid == fgpid(jobs)) {
        usleep(1000);  // 1ms sleep 避免忙等
    }
}



/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - Handles SIGCHLD signals sent by the kernel when:
 *   - a child process terminates normally (WIFEXITED)
 *   - a child process is terminated by a signal (WIFSIGNALED)
 *   - a child process is stopped by a signal like SIGTSTP (WIFSTOPPED)
 * 
 * This handler reaps all zombie children using a non-blocking waitpid call,
 * and updates the job list accordingly. It does not wait for currently running children.
 */
void sigchld_handler(int sig)
{
    int olderrno = errno;  // 保存 errno 防止被系统调用破坏
    pid_t pid;
    int status;

    // Reap all terminated or stopped children
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        // 正常退出
        if (WIFEXITED(status)) {
            if (!deletejob(jobs, pid)) {
                fprintf(stderr, "Warning: deletejob failed for pid %d\n", pid);
            }
        }
        // 被信号终止（如 SIGINT）
        else if (WIFSIGNALED(status)) {
            int jid = pid2jid(pid);
            printf("Job [%d] (%d) terminated by signal %d\n", 
                   jid, pid, WTERMSIG(status));
            if (!deletejob(jobs, pid)) {
                fprintf(stderr, "Warning: deletejob failed for pid %d\n", pid);
            }
        }
        // 被暂停（如 SIGTSTP）
        else if (WIFSTOPPED(status)) {
            int jid = pid2jid(pid);
            struct job_t *job = getjobpid(jobs, pid);
            if (job) {
                job->state = ST;
                printf("Job [%d] (%d) stopped by signal %d\n", 
                       jid, pid, WSTOPSIG(status));
            } else {
                fprintf(stderr, "Warning: stopped process (%d) not found in job list\n", pid);
            }
        }
    }

    // 错误处理：waitpid 失败时
    if (pid < 0 && errno != ECHILD) {
        unix_error("waitpid error");
    }

    errno = olderrno;  // 恢复 errno
}


/*
 * sigint_handler - Handles SIGINT (Ctrl-C) by forwarding the signal
 * to the foreground process group, allowing it to be terminated.
 */
void sigint_handler(int sig)
{
    int olderrno = errno; // 保存原 errno

    pid_t pid = fgpid(jobs);
    if (pid != 0) {
        // 将信号发送给前台进程组（负 pid 表示进程组）
        if (kill(-pid, SIGINT) < 0) {
            unix_error("kill (SIGINT) error");
        }
    }

    errno = olderrno; // 恢复 errno
}


/*
 * sigtstp_handler - Handles SIGTSTP (Ctrl-Z) by forwarding the signal
 * to the foreground process group, causing it to be suspended.
 */
void sigtstp_handler(int sig)
{
    int olderrno = errno;

    pid_t pid = fgpid(jobs);
    if (pid != 0) {
        if (kill(-pid, SIGTSTP) < 0) {
            unix_error("kill (SIGTSTP) error");
        }
    }

    errno = olderrno;
}


/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == 0) {
	    jobs[i].pid = pid;
	    jobs[i].state = state;
	    jobs[i].jid = nextjid++;
	    if (nextjid > MAXJOBS)
		nextjid = 1;
	    strcpy(jobs[i].cmdline, cmdline);
  	    if(verbose){
	        printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
	}
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == pid) {
	    clearjob(&jobs[i]);
	    nextjid = maxjid(jobs)+1;
	    return 1;
	}
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG: 
		    printf("Running ");
		    break;
		case FG: 
		    printf("Foreground ");
		    break;
		case ST: 
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ", 
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}



