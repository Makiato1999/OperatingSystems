#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>

#define COUNT_UP_TO 100000000

static atomic_flag flag = ATOMIC_FLAG_INIT;
static int counter = 0;


void temp_lock()
{
    while (atomic_flag_test_and_set(&flag) == 1)
    {
        ; // spin wait
    }
}

void temp_unlock()
{
    atomic_flag_clear(&flag);
}

void *count_up(void *args)
{
    (void)args;

    for (int i = 0; i < COUNT_UP_TO; i++)
    {
        temp_lock();
        counter++;
        temp_unlock();
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
