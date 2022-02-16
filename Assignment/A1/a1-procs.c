//-----------------------------------------
// NAME: Xiaoran Xie
// STUDENT NUMBER: 7884702
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Robert Guderian
// ASSIGNMENT: assignment 1, QUESTION: a1-procs
//
// REMARKS: Process herder
//
//-----------------------------------------
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
    // 32-bits is 0, 64-bits is 1
    false = 0,
    true
} boolean;
boolean isCatchSignal = false; // control parent sleep(wait)
int numOfWorkers = 0;          // current number of workers
int prevNumOfWorkers = 0;      // previous number of workers
pid_t child;                   // child id
pid_t parent;                  // parent id
int children[128];             // space for saving child id
int i = 0;                     // control index of processes

void handler(int signo);

int main()
{
    int status;
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
            // process config file
            while (i < numOfWorkers)
            {
                if ((child = fork()) < 0)
                {
                    perror("fork failed\n");
                    exit(1);
                }
                if (child > 0)
                {
                    children[i] = child;
                }
                else if (child == 0)
                {
                    break;
                }
                i++;
                usleep(800);
            }
            if (child == 0)
            {
                printf("Child(%d) is starting\n", getpid());
                // boolean isQuit = false;
                if (signal(SIGUSR1, handler) == SIG_ERR)
                {
                    perror("receive SIGUSR1 failed\n");
                    exit(1);
                }
                if (signal(SIGINT, handler) == SIG_ERR)
                {
                    perror("receive SIGINT failed\n");
                    exit(1);
                }
                while (1)
                {
                    sleep(1);
                }
            }
        }
        else if (prevNumOfWorkers > numOfWorkers)
        {
            printf("Changing setting to %d\n", numOfWorkers);
            int j;
            for (j = 0; j < prevNumOfWorkers - numOfWorkers; j++)
            {

                // send signal1 to let children exit
                if (kill(children[--i], SIGUSR1) == -1)
                {
                    perror("parent kill failed\n");
                    exit(1);
                }
                wait(&status);
                printf("Child(%d) is being killed\nChildren(%d) is exiting\n", children[i], children[i]);
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
    // wait all processes
    while (wait(&status) == -1)
    {
        printf("wait failed\n");
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
    if (signo == SIGUSR1)
    {
        exit(0);
    }
    if (signo == SIGINT)
    {
        if (getpid() == parent)
        {
            printf("\nParent(%d) is exiting\nParent(%d) is being killed\n", parent, parent);
        }
        exit(0);
    }
    return;
}
