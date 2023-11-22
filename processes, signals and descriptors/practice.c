#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ERR(source) \
    (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), kill(0, SIGKILL), exit(EXIT_FAILURE))

#define UNUSED(x) (void)(x)
volatile sig_atomic_t last_signal = 0;

struct student
{
    int n;
    int pid;
    int issue_count;
};

void sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
        ERR("sigaction");
}

void setaction(void(*f)(int, siginfo_t *siginfo,void *),int sigNo)
 {
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_sigaction = f;
    act.sa_flags = SA_SIGINFO;
    if( -1 == sigaction(sigNo,&act,NULL))
        ERR("sigaction");
 }

 void sigusr1_handler(int sig, siginfo_t *siginfo, void *uncontext)
 {
    UNUSED(uncontext);
    UNUSED(sig);
    printf("Teacher has accepted solution of student [%d]\n", siginfo->si_pid);
    if(kill(siginfo->si_pid,SIGUSR2))
        ERR("kill");
 }

 void sigusr2_handler(int sig)
 {
    UNUSED(sig);
    last_signal = sig;
 }

void usage(const char *pname)
{
    fprintf(stderr, "USAGE: %s p t  p1 p2 ... pn\n", pname);  // TODO
    exit(EXIT_FAILURE);
}

int child_work(int stud_prob, int number, int p, int t)
{
    printf("Student[%d,%d] has started doing task!\n", number, getpid());
    printf("Student probability: %d\n", stud_prob);
    srand(time(NULL)*getpid());
    int issuecount = 0;
    struct timespec ts = {0,100000000};
    struct timespec err = {0, 500000000};
    int i,j,random;
    for(i = 0;i < p;i++){
        printf("Student[%d,%d] is startig doing part %d of %d!\n", number, getpid(), i + 1, p);
        for(j = 0 ; j < t;j++){
            nanosleep(&ts,NULL);
            random = rand()%100;
            if(random < stud_prob)
            {
                issuecount++;
                printf("Student[%d,%d] has issue (%d) doing task!\n", number, getpid(), issuecount);
                nanosleep(&err,NULL);
            }
        }
        printf("Student[%d,%d] has finished part %d of %d!\n", number, getpid(), i + 1, p);
        while(last_signal != SIGUSR2)
        {
            if(kill(getppid(), SIGUSR1))
                ERR("kill");
            sleep(10);
        }
        last_signal = 0;
    }
    return issuecount;

}



void create_children(int *stud_probs, int n, int p, int t, struct student *result)
{
    pid_t s;
    int i, count;
    for(i = 0 ; i < n; i++)
    {
        if((s = fork())<0)
            ERR("Fork:");
        if(!s)
        {
            sethandler(sigusr2_handler, SIGUSR2);
            count = child_work(stud_probs[i],i+1,p,t);
            exit(count);
        }
        else
        {
            result[i].n = i+1;
            result[i].pid = s;
            result[i].issue_count = 0;
        }
    }
}



int main(int argc, char *argv[])
{
    int i,total_issues = 0;
    if(argc< 4)
        usage(argv[0]);
    int p = atoi(argv[1]);
    int t = atoi(argv[2]);
    if(p < 1 || p>10 || t < 1 || t> 10)
        usage(argv[0]);
    int n = argc - 3;
    int child_num = n;
    int *stud_probs = (int*)malloc(sizeof(int)*n);
    for(int i = 0; i < n; i++)
    {
        stud_probs[i] = atoi(argv[3+i]);
        if(stud_probs[i]< 0 || stud_probs[i]>100)
            usage(argv[0]);
    }

    struct student *results = (struct student *)malloc(sizeof(struct student)*n);
    create_children(stud_probs,n,p,t,results);
    setaction(sigusr1_handler,SIGUSR1);
    while(n>0)
    {
        int status;
        pid_t pid;
        for(;;)
        {
            pid = waitpid(0,&status, WNOHANG);
            if(pid > 0){
                for(i = 0 ; i < child_num;i++)
                {
                    if(results[i].pid == pid)
                    {
                        results[i].issue_count = WEXITSTATUS(status);
                        total_issues += WEXITSTATUS(status);
                        break;
                    }
                }
                n--;
            }
            if(0 == pid)
                break;
            if(0 >= pid)
            {
                if(ECHILD == errno)
                    break;
                ERR("waitpid:");
            }
        }
    }
    printf("No. | Student ID | Issue Count\n");
    for(i= 0; i < child_num;i++)
    {
        printf("%3d | %10d | %-11d\n", results[i].n, results[i].pid, results[i].issue_count);
    }
    printf("Total issues: %d\n", total_issues);
    return EXIT_SUCCESS;
}
