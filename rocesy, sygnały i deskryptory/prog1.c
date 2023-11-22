#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ERR(source)(fprintf(stderr,"%s:%d\n", __FILE__, __LINE__),perror(source), kill(0,SIGKILL),exit(EXIT_FAILURE))

void Life_child(int i)
{
    srand(time(NULL)*getpid());
    int t = 5 + rand()%6;
    sleep(t);
    printf("PROCESS with pid %d termiantes\n",getpid());
}

void usage(char*name)
{
    fprintf(stderr,"USAGE:%s błąd", name);
    exit(EXIT_FAILURE);
}
void GetChildern(int n)
{
    pid_t pid;
    for(n--; n>=0;n--)
    {
        pid = fork();
        if(pid < 0)
            ERR("fork");
        if(!pid){
            Life_child(n);
            exit(EXIT_SUCCESS);
        }
    }
}

int main(int argc, char** argv)
{
    int n;
    if(argc<2)
        usage(argv[0]);
    n = atoi(argv[1]);
    if(n<=0)
        usage(argv[1]);
    GetChildern(n);
    while(n> 0)
    {
        sleep(3);
        pid_t pid;
        for(;;)
        {
        
            pid = waitpid(0,NULL,WNOHANG);
            if(pid>0)
                n--;
            if(0==pid)
                break;
            if(0>=pid){
                if(errno == ECHILD)
                    break;
                ERR("waitpid");
            }
        }
        printf("PARENT: %d processes remain\n",n);
    }
    return EXIT_SUCCESS;
}