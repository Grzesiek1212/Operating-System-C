#include <math.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
typedef unsigned int UINT;
#define MAXLINE 4096
#define DEFAULT_THREADCOUNT 10
#define DEFAULT_SAMPLESIZE 100


void usage(char *name)
{
    fprintf(stderr, "USAGE: %s \n", name);
    exit(EXIT_FAILURE);
}

struct ThreadData
{
    int num_points;
    double result;
    UINT seed;
};

void *Carlo(void *arg)
{
    int i;
    struct ThreadData *threadData = (struct ThreadData*)arg;
    int points_inside_circle = 0;

    for(i = 0; i < threadData->num_points;i++){
        //[-1;1]x[-1;1]
        double x = ((double)rand_r(&threadData->seed)/(double)RAND_MAX);
        double y = ((double)rand_r(&threadData->seed)/(double)RAND_MAX);

        if(sqrt(x*x + y*y) <= 1)
            points_inside_circle += 1;
    }
    
    threadData->result = 4*((double)points_inside_circle/threadData->num_points);
    pthread_exit(NULL);
}

void ReadArguments(int argc, char **argv, int *threadCount, int *samplesCount)
{
    *threadCount = DEFAULT_THREADCOUNT;
    *samplesCount = DEFAULT_SAMPLESIZE;
    if(argc != 3)
        usage("zła ilość argumentów");
    
    *threadCount = atoi(argv[1]);
    *samplesCount = atoi(argv[2]);

    if(*threadCount <= 0)
        usage("źle podana ilość wątków");
    
    if(*samplesCount<=0)
        usage("źle podana ilość punktow");
    
}

int main(int argc, char**argv)
{
    int i,k,n;
    ReadArguments(argc,argv,&k,&n);
    struct ThreadData *threadData = (struct ThreadData*)malloc(sizeof(struct ThreadData)*k);
    if(threadData == NULL)
        usage("malloc error for estiamtion argumetns!");
    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t)*k);
    if(threads == NULL)
        usage("malloc error");
    double totalpia = 0,wynik;
    srand(time(NULL));
    for(i = 0; i < k;i++){
        threadData[i].seed = rand();
        threadData[i].num_points = n;

        if(pthread_create(&threads[i],NULL,Carlo,(void *)&threadData[i]))
            ERR("pthread_create");
    }

    for(i = 0; i < k;i++){
        if(pthread_join(threads[i],NULL))
            ERR("pthread_join");
        totalpia += threadData[i].result;
    }

    wynik = totalpia/k;

    printf("średnie przyblizenie liczby Pi wynosi:%f \n", wynik);
    free(threadData);
    free(threads);
    return EXIT_SUCCESS;

}