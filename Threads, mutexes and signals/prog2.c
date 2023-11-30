#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 4096
#define DEFAULT_N 1000
#define DEFAULT_K 10
#define BIN_COUNT 11
#define NEXT_DOUBLE(seedptr) ((double)rand_r(seedptr) / (double)RAND_MAX)
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

typedef unsigned int UINT;

typedef struct argsThrower
{
    pthread_t tid;
    UINT seed;
    int *pBallsThrown;
    int *pBallsWaiting;
    int *bins;
    pthread_mutex_t *mxBins;
    pthread_mutex_t *pmxBallsThrown;
    pthread_mutex_t *pmxBallsWaiting;
}argsThrower_t;

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s \n", name);
    exit(EXIT_FAILURE);
}

void ReadArguments(int argc, char **argv, int *threadCount, int *samplesCount)
{
    *threadCount = DEFAULT_N;
    *samplesCount= DEFAULT_K;
    if(argc != 3)
        usage("zła ilość argumentów");
    
    *threadCount = atoi(argv[1]);
    *samplesCount = atoi(argv[2]);

    if(*threadCount <= 0)
        usage("źle podana ilość kulek");
    
    if(*samplesCount<=0)
        usage("źle podana ilość rzucających");
}

int throwBall(UINT *seedptr)
{
    int result = 0;
    for(int i = 0; i < BIN_COUNT - 1;i++)
        if(NEXT_DOUBLE(seedptr)>0.5)
            result++;
    return result;
}

void *throwing_func(void *voidargs)
{
    argsThrower_t *args = voidargs;
    while(1)
    {
        pthread_mutex_lock(args->pmxBallsWaiting);
        if(*args->pBallsWaiting>0)
        {
            (*args->pBallsWaiting)--;
            pthread_mutex_unlock(args->pmxBallsWaiting);
        }else
        {
            pthread_mutex_unlock(args->pmxBallsWaiting);
            break;
        }
        int binno = throwBall(&args->seed); // funkcja do napisania
        pthread_mutex_lock(&args->mxBins[binno]);
        args->bins[binno] ++;
        pthread_mutex_unlock(&args->mxBins[binno]);
        pthread_mutex_lock(args->pmxBallsThrown);
        (*args->pBallsThrown)+= 1;
        pthread_mutex_unlock(args->pmxBallsThrown);
    }
    return NULL;
}

void make_throwers(argsThrower_t *args, int throwersCount)
{
    pthread_attr_t threadAttr;
    if(pthread_attr_init(&threadAttr))
        ERR("Coulnd't create pthread_attr_t");
    if (pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED))
        ERR("Couldn't setdetachsatate on pthread_attr_t");
    for(int i = 0 ; i < throwersCount;i++)
    {
        if(pthread_create(&args[i].tid, &threadAttr,throwing_func,&args[i])) // do napisai funkcja
            ERR("Couldn't create thread");
    }
    pthread_attr_destroy(&threadAttr);
}


int main(int argc, char**argv){
    int ballsCount, throwersCount;
    ReadArguments(argc, argv, &ballsCount, &throwersCount);
    
    int ballsThrown = 0, bt = 0, ballsWaiting = ballsCount;
    pthread_mutex_t mxBallsThrown = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mxBallsWaiting = PTHREAD_MUTEX_INITIALIZER;
    int bins[BIN_COUNT];
    pthread_mutex_t mxBins[BIN_COUNT];

    for(int i = 0; i < BIN_COUNT;i++)
    {
        bins[i] = 0;
        if(pthread_mutex_init(&mxBins[i], NULL))
            ERR("Couldn't initialize mutex");
    }
    argsThrower_t *args = (argsThrower_t *)malloc(sizeof(argsThrower_t)*throwersCount);
    if(args == NULL)
        ERR("Malloc error for thowers arguments");
    srand(time(NULL));
    for(int i = 0 ; i < throwersCount;i++){
        args[i].seed = (UINT)rand();
        args[i].pBallsThrown = &ballsThrown;
        args[i].pBallsWaiting = &ballsWaiting;
        args[i].bins = bins;
        args[i].pmxBallsThrown = &mxBallsThrown;
        args[i].pmxBallsWaiting = &mxBallsWaiting;
        args[i].mxBins = mxBins;
    }
    make_throwers(args,throwersCount);
    while(bt < ballsCount)
    {
        sleep(1);
        pthread_mutex_lock(&mxBallsThrown);
        bt = ballsThrown;
        pthread_mutex_unlock(&mxBallsThrown);
    }
    int realBallsCount = 0;
    double meanValue = 0.0;
    for (int i = 0; i < BIN_COUNT; i++)
    {
        realBallsCount += bins[i];
        meanValue += bins[i] * i;
    }
    meanValue = meanValue / realBallsCount;
    printf("Bins count:\n");
    for (int i = 0; i < BIN_COUNT; i++)
        printf("%d\t", bins[i]);
    printf("\nTotal balls count : %d\nMean value: %f\n", realBallsCount, meanValue);
     for (int i = 0; i < BIN_COUNT; i++) pthread_mutex_destroy(&mxBins[i]);
            free(args);
    exit(EXIT_SUCCESS);




}