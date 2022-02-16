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
boolean isCatchSignal = false;
int numOfWorkers = 0;
int prevNumOfWorkers = 0;
pid_t child;
pid_t children[128];
int counter = 0;
int i = 0;

void handler(int signo);

int main()
{
    // int status;
    //  catch signal
    pid_t parent = getpid();
    printf("I am parent(%d)\n", parent); // parent
    if (signal(SIGHUP, handler) == SIG_ERR)
    {
        perror("receive SIGHUP signal failed\n");
        exit(1);
    }

    while (1)
    {
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
                if (child == 0)
                {
                    children[i] = getpid();
                    break;
                }
                i++;
                usleep(800);
            }
            if (child == 0)
            {
                printf("Child(%d) is starting\n", getpid());
                while (1)
                {
                    sleep(1);
                    if (signal(SIGKILL, handler) != SIG_ERR)
                    {
                        printf("---------\n");
                        break;
                    }
                }
                printf("Child(%d) is being killed\nChild(%d) is exiting", getpid(), getpid());
                exit(EXIT_SUCCESS);
            }
        }
        else if (prevNumOfWorkers > numOfWorkers)
        {
            printf("Changing setting to %d\n", numOfWorkers);
            int j;
            for (j = 0; j < prevNumOfWorkers - numOfWorkers; j++)
            {
                printf("2222222222\n");
                // send signal1 to let children exit
                if (kill(children[--i], SIGKILL) == -1)
                {
                    perror("parent kill failed\n");
                    exit(1);
                }
            }
        }
        printf("========\n");
        // sleep until get signal, prepare read config file again
        while (isCatchSignal == false)
        {
            sleep(1);
        }
        printf("++++++++++\n");
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
    if (signo == SIGKILL)
    {
    }
    return;
}
