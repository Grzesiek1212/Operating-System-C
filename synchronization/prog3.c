#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define PLAYER_COUNT 4
#define ROUNDS 10

struct player{
    int id;
    unsigned int seed;
    int *scores;
    int *rolls;
    pthread_barrier_t playerBarrier;
};

int main(int argc, char**argv)
{
    pthread_t threads[PLAYER_COUNT];
    int tab_scores[PLAYER_COUNT] = {0};
    struct player tab_player[PLAYER_COUNT];
    int rolls[PLAYER_COUNT];
    pthread_barrier_t barier;

    pthread_barrier_init(&barier,NULL,PLAYER_COUNT)


}