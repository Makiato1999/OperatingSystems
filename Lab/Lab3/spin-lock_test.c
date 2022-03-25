#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define COUNT_UP_TO 100000000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int counter = 0;

void *count_up(void *args)
{
    (void) args;

    for ( int i = 0; i < COUNT_UP_TO; i++ )
    {
        pthread_mutex_lock(&mutex);
        counter++;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(void)
{
    pthread_t p1, p2;

    pthread_create(&p1, NULL, count_up, NULL);
    pthread_create(&p2, NULL, count_up, NULL);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    printf("Counter is %d\n", counter);
    return EXIT_SUCCESS;
}



