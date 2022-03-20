#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define maxNum_eachLine 1024
int stdin_done = 0;
int lowerCase_done = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

void *lowerCase_thread(void *arg)
{
    pthread_mutex_lock(&mutex);
    while (stdin_done == 0)
    {
        // keep wait until stdin done
        pthread_cond_wait(&cond, &mutex);
    }
    char *original_message = arg;
    printf("In lowerCase thread: \n");
    printf("- Your input: %s", original_message);
    printf("- lowerCase for your input: ");
    unsigned long i;
    for (i = 0; i < strlen(original_message); i++)
    {
        printf("%c", tolower(original_message[i]));
    }
    printf("\n");
    // update status
    lowerCase_done = 1;
    pthread_cond_signal(&cond2);
    pthread_mutex_unlock(&mutex);

    pthread_exit((void *)pthread_self());
}

void *upperCase_thread(void *arg)
{
    pthread_mutex_lock(&mutex2);
    while (stdin_done == 0)
    {
        // keep wait until stdin done
        pthread_cond_wait(&cond, &mutex2);
    }
    while (lowerCase_done == 0)
    {
        // keep wait until lowerCase done
        pthread_cond_wait(&cond2, &mutex2);
    }
    char *original_message = arg;
    printf("In upperCase thread: \n");
    printf("- Your input: %s", original_message);
    printf("- upperCase for your input: ");
    unsigned long i;
    for (i = 0; i < strlen(original_message); i++)
    {
        printf("%c", toupper(original_message[i]));
    }
    printf("\n");
    pthread_mutex_unlock(&mutex2);

    pthread_exit((void *)pthread_self());
}

int main(void)
{
    // get standard input
    char message[maxNum_eachLine];
    pthread_mutex_lock(&mutex);
    stdin_done = 0;
    fgets(message, maxNum_eachLine, stdin);
    printf("Your input is: %s\n", message);
    // update status
    stdin_done = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    pthread_t P1, P2;
    pthread_create(&P1, NULL, lowerCase_thread, message);
    pthread_create(&P2, NULL, upperCase_thread, message);
    pthread_join(P1, NULL);
    pthread_join(P2, NULL);

    return EXIT_SUCCESS;
}
