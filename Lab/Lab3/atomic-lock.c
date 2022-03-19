#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>

#define COUNT_UP_TO 100000000

typedef struct LOCK_T 
{ 
    atomic_int flag; 
} lock_t;

static lock_t lock;
static int counter = 0;

void init(lock_t *mutex) 
{
    mutex->flag = 0;
}

void temp_lock(lock_t *mutex) 
{
    while (mutex->flag == 1)
    {
        ;// spin wait
    }
    mutex->flag = 1;
}

void temp_unlock(lock_t *mutex) 
{
    mutex->flag = 0;
}

void *count_up(void *args)
{
    (void) args;

    for ( int i = 0; i < COUNT_UP_TO; i++ )
    {
        temp_lock(&lock);
        counter++;
        temp_unlock(&lock);
    }

    return NULL;
}

int main(void)
{
    pthread_t p1, p2;

    init(&lock);

    pthread_create(&p1, NULL, count_up, NULL);
    pthread_create(&p2, NULL, count_up, NULL);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    printf("Counter is %d\n", counter);
    return EXIT_SUCCESS;
}



