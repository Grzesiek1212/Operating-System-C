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
#include <pthread.h>

#define DEFAULT_NRANGE 10
#define DEFAULT_WORKINGTHREAD  4

#define ERR(source) \
    (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), kill(0, SIGKILL), exit(EXIT_FAILURE))

#define UNUSED(x) (void)(x)
typedef unsigned int UINT;
typedef struct timespec timespec_t;

typedef struct
{
    pthread_t tid;
    int *working;
    pthread_mutex_t *mxArray;
    int *array;
    int n;
    UINT seed;
}threads_t;



void usage(const char *pname)
{
    fprintf(stderr, "USAGE: %s n p\n", pname);
    exit(EXIT_FAILURE);
}

void ReadArguments(int argc, char **argv, int *nrange, int *wrokingthread);
void init_arrays(int *array, pthread_mutex_t *mxArray, threads_t *threads, int *working_threads, int n, int p);
void parent_work(sigset_t *newMask, threads_t *threads, int p);
void spawn_print(int assigned_thread, threads_t *threads);
int find_thread(threads_t *threads, int p);
void spawn_swap(int assigned_thread, threads_t *threads);
void *swap(void*args);
void msleep(UINT milisec);
void *printArray(void *args);

int main(int argc, char *argv[])
{
    int n, p;
    ReadArguments(argc, argv, &n, &p);
    srand(time(NULL));
    int *array = malloc(sizeof(int) * n);
    pthread_mutex_t *mxArray = malloc(sizeof(pthread_mutex_t) * n);
    threads_t *threads = malloc(sizeof(threads_t) * p);
    int *working_threads = malloc(sizeof(int) * p);

    if (array == NULL || mxArray == NULL || threads == NULL || working_threads == NULL)
        ERR("Not able to alocate");
    init_arrays(array, mxArray, threads, working_threads, n, p);
    sigset_t oldMask, newMask;
    sigemptyset(&newMask);
    sigaddset(&newMask, SIGUSR1);
    sigaddset(&newMask, SIGUSR2);
    sigaddset(&newMask, SIGINT);
    if (pthread_sigmask(SIG_BLOCK, &newMask, &oldMask))
        ERR("SIG_BLOCK error");

    parent_work(&newMask, threads, p);
    for (i = 0; i < p; i++)
    {

        if (*(threads[i].working) == 0)
            continue;
        if (pthread_join(threads[i].tid, NULL))
            ERR("Can't join with 'signal handling' thread");
    }
    free(array);
    free(mxArray);
    free(threads);
    free(working_threads);
    exit(EXIT_SUCCESS);
}

void ReadArguments(int argc, char **argv, int *nrange, int *wrokingthread)
{
    *nrange = DEFAULT_NRANGE;
    *wrokingthread = DEFAULT_WORKINGTHREAD;

    if (argc >= 2)
    {
        *nrange = atoi(argv[1]);
        if (*nrange < 8 || *nrange > 256)
        {
            printf("Invalid value for 'nrange'");
            exit(EXIT_FAILURE);
        }
    }
    if (argc >= 3)
    {
        *wrokingthread = atoi(argv[2]);
        if (*wrokingthread< 1 || *wrokingthread>16)
        {
            printf("Invalid value for 'wrokingthread'");
            exit(EXIT_FAILURE);
        }
    }
}

void init_arrays(int *array, pthread_mutex_t *mxArray, threads_t *threads, int *working_threads, int n, int p)
{
    int i;
    for (i = 0; i < n; i++)
    {
        array[i] = i;
        if (pthread_mutex_init(&mxArray[i], NULL))
            ERR("Couldn't initialize mutex!");
    }

    for (i = 0; i < p; i++)
    {
        working_threads[i] = 0;
        threads[i].working = &working_threads[i];
        threads[i].array = array;
        threads[i].mxArray = mxArray;
        threads[i].n = n;
        threads[i].seed = rand();
        threads[i].tid = 0;
    }
}

void parent_work(sigset_t *newMask, threads_t *threads, int p)
{
    int signo;
    while (1)
    {
        printf("aaaa\n");
        if (sigwait(newMask, &signo))
            ERR("sigwait failed.");
        printf("SYGNAL %d", signo);
        int assigned_thread = find_thread(threads, p);
        if (assigned_thread == -1)
        {
            printf("All thread busy, aborting request\n");
            continue;
        }
        switch (signo)
        {
            case SIGUSR1:
                spawn_swap(assigned_thread, threads);
                break;
            case SIGUSR2:
                spawn_print(assigned_thread, threads);
                break;
            case SIGINT:
		        *(threads[assigned_thread].working) = 0;
                printf("Exit\n");
                return;
            default:
                ERR("Unknow signal");
        }
    }
}

int find_thread(threads_t *threads, int p)
{
    int i;
    for (i = 0; i < p; i++)
    {
        if (*(threads[i].working) == 0)
        {
            if (threads[i].tid != 0)
            {
                if (pthread_join(threads[i].tid, NULL))
                    ERR("Cannot join thread");
                threads[i].tid = 0;
            }
            *(threads[i].working) = 1;
            return i;
        }
    }
    return -1;
}

void spawn_swap(int assigned_thread, threads_t *threads)
{
    if(pthread_create(&(threads[assigned_thread].tid),NULL,swap,&threads[assigned_thread]))
        ERR("pthread_create error");
}

void *swap(void *args)
{
    threads_t *arguments = args;
    int n = arguments->n;
    int a = rand_r(&arguments->seed) % (n-1), b = -1,i;
    while(a > b)
    {
        b = rand_r(&arguments->seed)%n;
    }
    for(i = 0 ; i <= (b-a)/2;i++)
    {
        if(a+i == b-i)
        {
            break;
        }
        pthread_mutex_lock(&(arguments->mxArray[a+i]));
        pthread_mutex_lock(&(arguments->mxArray[b -i]));
        int temp = arguments->array[a+i];
        arguments->array[a+i] = arguments->array[b-i];
        arguments->array[b-i] = temp;
        pthread_mutex_unlock(&(arguments->mxArray[a+i]));
        pthread_mutex_unlock(&(arguments->mxArray[b-i]));
        msleep(5);
    }
    *(arguments->working) = 0;
    pthread_exit(EXIT_SUCCESS);
}

void msleep(UINT milisec)
{
    time_t sec = (int)(milisec / 1000);
    milisec = milisec - (sec * 1000);
    timespec_t req = {0};
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;
    if (nanosleep(&req, &req))
        ERR("nanosleep");
}

void spawn_print(int assigned_thread, threads_t *threads)
{
    pthread_attr_t threadAttr;

    if (pthread_attr_init(&threadAttr))
        ERR("Couldn't create pthread_attr_t");
    if (pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED))
        ERR("Couldn't setdetachsatate on pthread_attr_t");
    if (pthread_create(&(threads[assigned_thread].tid), &threadAttr, printArray, &threads[assigned_thread]))
        ERR("Couldn't create thread");
    pthread_attr_destroy(&threadAttr);
}

void *printArray(void *args)
{
    threads_t *arguments = (threads_t *)args;
    int i = 0, n = arguments->n;
    for (i = 0; i < n; i++)
    {
        pthread_mutex_lock(&(arguments->mxArray[i]));
    }
    for (i = 0; i < n; i++)
    {
        printf("%d ", arguments->array[i]);
    }
    printf("\n");
    for (i = 0; i < n; i++)
    {
        pthread_mutex_unlock(&(arguments->mxArray[i]));
    }
    *(arguments->working) = 0;
    pthread_exit(EXIT_SUCCESS);
}