#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdint.h>
#include <fcntl.h>
#include <assert.h>

typedef enum BOOLEAN
{
    false = 0,
    true
} boolean;
boolean isCatchSignal = false; // control parent sleep(wait)
long i = 0;                     // control index of threads
int numOfWorkers = 0;          // current number of workers
int prevNumOfWorkers = 0;      // previous number of workers
pid_t parent;                  // parent thread id
long children[128];           // space for saving child id
int rc;                        // pthread_create return value

void handler(int signo);
void *worker_thread(void *arg);

int main()
{
    parent = getpid();
    printf("I am parent(%d)\n", parent); // parent

    while (1)
    {
        //  initialize and then catch signal
        if (signal(SIGHUP, handler) == SIG_ERR)
        {
            perror("receive SIGHUP signal failed\n");
            exit(1);
        }

        // open config file
        char *fileName = "config.txt";
        FILE *fd = fopen(fileName, "r");
        if (fd == NULL)
        {
            perror("open config file failed!\n");
            exit(1);
        }
        // read config file
        fscanf(fd, "%d", &numOfWorkers);
        if (numOfWorkers <= 0)
        {
            perror("Invaild input for number of workers!\n");
            exit(1);
        }
        printf("Reading config file\n");
        isCatchSignal = false;
        fclose(fd);

        //  compare whether need to add workers
        if (prevNumOfWorkers == numOfWorkers)
        {
            printf("Same number of workers, not change\n");
        }
        else if (prevNumOfWorkers < numOfWorkers)
        {
            printf("Changing setting to %d\n", numOfWorkers);
            // create worker threads
            while (i < numOfWorkers)
            {
                pthread_t p;
                rc = pthread_create(&p, NULL, worker_thread, (void *)i);
                children[i] = i;
                printf("Starting %ld\n", i);
                if (rc)
                {
                    perror("thread creates failed\n");
                    exit(1);
                }
                i++;
            }
        }
        else if (prevNumOfWorkers > numOfWorkers)
        {
            printf("Changing setting to %d\n", numOfWorkers);
            
        }
        // sleep until get signal, prepare read config file again
        while (isCatchSignal == false)
        {
            sleep(1);
        }
        // update previous number of workers
        prevNumOfWorkers = numOfWorkers;
    }

    return 0;
}

void handler(int signo)
{
    if (signo == SIGHUP)
    {
        printf("\ncatch SIGHUP!\n");
        isCatchSignal = true;
    }
}

void *worker_thread(void *arg)
{
    usleep(800);
    printf("Thread %ld has started\n", (long)arg);
    while (1)
    {
        sleep(1);
    }
}

