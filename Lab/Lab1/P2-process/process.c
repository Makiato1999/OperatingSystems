#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>

int counter = 0;
int numOfProcess = 0;

void handler(int signo);

int main(int argc, char *argv[])
{
    // exception
    if (argc < 2)
    {
        printf("Invalid arguments\n");
        exit(1);
    }
    numOfProcess = atoi(argv[1]);
    if (numOfProcess <= 0)
    {
        printf("Invalid arguments\n");
        exit(1);
    }

    pid_t child;
    // group of processes
    pid_t children[numOfProcess];

    if (signal(SIGUSR1, handler) == SIG_ERR)
    {
        perror("receive child signal failed\n");
        exit(1);
    }
    else if (signal(SIGUSR2, handler) == SIG_ERR)
    {
        perror("receive itself signal failed\n");
        exit(1);
    }
    int i;
    for (i = 0; i < numOfProcess; i++)
    {
        if ((child = fork()) < 0)
        {
            // fork failed
            perror("fork failed\n");
            exit(1);
        }
        if (child > 0)
        {
            children[i] = getpid();
            if (setpgid(getpid(), children[0]) == -1)
            {
                perror("setpgid failed\n");
            }
            // printf("children[0]: %d\n", children[0]);
            // printf("groupID:%d, getpid;%d\n", getpgid(getpid()), children[i]);
        }
        else if (child == 0)
        {
            break;
        }
        sleep(1);
    }
    if (child == 0)
    {
        // child
        pid_t parent = getppid();
        if (kill(parent, SIGUSR1) == -1)
        {
            perror("child kill failed\n");
            exit(1);
        }
        while (1)
        {
            sleep(1);
            if (signal(SIGUSR2, handler) == SIG_ERR)
            {
                perror("receive parent signal failed\n");
                exit(1);
            }
            else if (signal(SIGUSR2, handler) != SIG_ERR)
            {
                printf("child(%d) catches the signal SIGUSR2\n", getpid());
                break;
            }
        }
        exit(EXIT_SUCCESS);
    }
    // parent
    while (counter <= numOfProcess)
    {
        sleep(1);
        //printf("parent counter:%d\n", counter);
        if (counter == numOfProcess)
        {
            counter++;
        }
    }

    if (kill(-children[0], SIGUSR2) == -1)
    {
        perror("parent kill failed\n");
        exit(1);
    }
    wait(NULL);
    printf("\nall processes have done!\n");
    return 0;
}

void handler(int signo)
{
    if (signo == SIGUSR1)
    {
        printf("the %dth time parent(%d) catches the child signal SIGUSR1\n", ++counter, getppid());
    }
    else if (signo == SIGUSR2)
    {
        printf("parent(%d) catches the signal SIGUSR2\n", getppid());
    }
    return;
}
