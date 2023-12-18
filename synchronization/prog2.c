#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define BUFFERSIZE 256
#define READCHUNKS 4
#define THREAD_NUM 3
volatile sig_atomic_t work = 1;


void sigint_handler(int sig) { work = 0; }

void set_handler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0x00, sizeof(struct sigaction));
    act.sa_handler = f;
    
    if (-1 == sigaction(sigNo, &act, NULL))
        ERR("sigaction");
}


void cleanup(void *arg) { pthread_mutex_unlock((pthread_mutex_t *)arg); }


ssize_t bulk_write(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(write(fd, buf, count));
        if (c < 0)
            return c;
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

ssize_t bulk_read(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if (c < 0)
            return c;
        if (c == 0)
            return len;  // EOF
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

typedef struct
{
    int id;
    int *condition;
    int *counterl;
    pthread_cond_t *cond;
    pthread_mutex_t *mutex;
}args_t;

void read_write(int id)
{
    char file_name[20];
    char buffer[BUFFERSIZE];
    snprintf(file_name,sizeof(file_name), "random%d.bin", id);
    printf("zapisujemy do pliku\n");
    int i,in,out;
    if((in = open(file_name, O_WRONLY|O_CREAT| O_TRUNC | O_APPEND,0777))<0)
        ERR("open");
    if((out = open("/dev/urandom",O_RDONLY))<0)
        ERR("open");
    for(i = 0 ; i < READCHUNKS;i++)
    {
        const ssize_t read_size = bulk_read(out, buffer, BUFFERSIZE);
        if (read_size == -1)
            ERR("bulk_read");
        if (read_size == 0)
            break;
        if (bulk_write(in, buffer, read_size) == -1)
            ERR("bulk_write");
        sleep(1);
    }
    if (close(in) == -1)
        ERR("close");
    if (close(out) == -1)
        ERR("close");

}

void *thread_work(void * args)
{
    args_t arguments;
    memcpy(&arguments,args,sizeof(arguments));

    while(1){
        pthread_cleanup_push(cleanup,(void*)arguments.mutex);
        if(pthread_mutex_lock(arguments.mutex)!= 0)
            ERR("pthread_mutex_lock");
        (*arguments.counterl)++;
        while(!*arguments.condition && work)
        {
            if(pthread_cond_wait(arguments.cond,arguments.mutex)!= 0)
                ERR("pthread-cond_t");
        }
        *arguments.condition = 0;
        if(!work)
            pthread_exit(NULL);
        (*arguments.counterl)--;
        pthread_cleanup_pop(1);
        read_write(arguments.id);
    }
    return NULL;

} 

void init(pthread_t *tab, args_t *tab_args, pthread_mutex_t *mutex, pthread_cond_t *cond,int *condition, int *counterl)
{
    for(int  i = 0 ; i < THREAD_NUM;i++){
        tab_args[i].id = i+1;
        tab_args[i].condition = condition;
        tab_args[i].counterl = counterl;
        tab_args[i].cond = cond;
        tab_args[i].mutex = mutex;
        if(pthread_create(&tab[i],NULL,thread_work,(void*)&tab_args[i]) != 0)
            ERR("pthread_create");
    }
}

void do_work(pthread_cond_t *cond, pthread_mutex_t *mutex,int * condition, int * counterl)
{
    char buffer[BUFFERSIZE];
    while(work)
    {
        if(fgets(buffer, BUFFERSIZE, stdin) != NULL)
        {
            if(pthread_mutex_lock(mutex)!=0)
                ERR("pthread_mtuex_lock");
            if(*counterl == 0)
            {
                if(pthread_mutex_unlock(mutex)!=0)
                    ERR("pthread_mutex_unlock");
                fputs("nie ma dostepnych wątkow\n",stderr);

            }else
            {
                if(pthread_mutex_unlock(mutex)!=0)
                    ERR("pthread_mutex_unlock");
                *condition = 1;
                if(pthread_cond_signal(cond) !=0)
                    ERR("pthread_cond_signal");

            }
        }else{
            if(errno == EINTR)
                continue;
            ERR("fgets");
        }
    }
}


int main(int argc, char**argv)
{
    int condition = 0, counterl = 0;
    pthread_t thread_tab[THREAD_NUM];
    args_t thread_args[THREAD_NUM];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    init(thread_tab,thread_args,&mutex,&cond,&condition,&counterl);
    do_work(&cond,&mutex,&condition, &counterl);
    if (pthread_cond_broadcast(&cond) != 0)
        ERR("pthread_cond_broadcast");
    for (int i = 0; i < THREAD_NUM; i++)
        if (pthread_join(thread_tab[i], NULL) != 0)
            ERR("pthread_join");
    return EXIT_SUCCESS;

    return EXIT_SUCCESS;
}