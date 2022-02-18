//-----------------------------------------
// NAME: Xiaoran Xie
// STUDENT NUMBER: 7884702
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Robert Guderian
// ASSIGNMENT: assignment 1, QUESTION: a1-threads
//
// REMARKS: threads herder
//
//-----------------------------------------
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
unsigned long currThread = 0;  // current Thread id
long i = 0;                    // control index of threads
int numOfWorkers = 0;          // current number of workers
int prevNumOfWorkers = 0;      // previous number of workers
pid_t parent;                  // parent thread id
pthread_t children[128];       // space for saving child id
int rc;                        // pthread_create return value

void handler(int signo);
void *worker_thread(void *arg);

int main()
{
    // main thread is also main process
    parent = getpid();
    printf("I am thread 0 (%d)\n", parent);

    while (1)
    {
        //  initialize and then catch signal
        if (signal(SIGHUP, handler) == SIG_ERR)
        {
            perror("receive SIGHUP signal failed\n");
            exit(1);
        }
        if (signal(SIGINT, handler) == SIG_ERR)
        {
            perror("receive SIGINT signal failed\n");
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
                rc = pthread_create(&children[i], NULL, worker_thread, (void *)i);
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
            int j;
            for (j = 0; j < prevNumOfWorkers - numOfWorkers; j++)
            {
                currThread = children[--i];
                usleep(100);
                // let children exit
                // pthread_kill(&children[--i], SIGUSR1);
                pthread_join(children[i], NULL);
                printf("Stopping %ld\nThread %ld is going to a better place\n", i, i);
            }
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
//------------------------------------------------------
// myRoutine: handler
//
// PURPOSE: signal handler
// INPUT PARAMETERS:
//	   int signo
//------------------------------------------------------
void handler(int signo)
{
    if (signo == SIGHUP)
    {
        printf("\ncatch SIGHUP!\n");
        isCatchSignal = true;
    }
    if (signo == SIGINT)
    {
        printf("\n");
        while (i > 0)
        {
            currThread = children[--i];
            usleep(100);
            // let children exit
            // pthread_kill(&children[--i], SIGUSR1);
            pthread_join(children[i], NULL);
            printf("Stopping %ld\nThread %ld is going to a better place\n", i, i);
        }
        exit(0);
    }
    return;
}
//------------------------------------------------------
// myRoutine: worker_thread
//
// PURPOSE: worker_thread
// INPUT PARAMETERS:
//	   void *arg
//------------------------------------------------------
void *worker_thread(void *arg)
{
    usleep(100);
    printf("Thread %ld has started\n", (long)arg);
    while (currThread != pthread_self())
    {
        sleep(1);
    }
    pthread_exit((void *)pthread_self());
}

