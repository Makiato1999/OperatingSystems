#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <string.h>

#define maxNum_eachLine 1024
#define maxNum_eachCommand 50
#define STDIN_FILENO 0  // Standard input
#define STDOUT_FILENO 1 // Standard output

int main(int argc, char *argv[])
{
    // exception
    if (argc < 2)
    {
        perror("Invalid arguments!\n");
        exit(1);
    }

    int i;
    int state;
    pid_t pid;
    char *commandLine;
    // create pipe array to save pipes
    int fd[argc - 1][2];
    for (i = 0; i < argc - 1; i++)
    {
        commandLine = argv[i + 1];
        printf("%d is about to start [%s]\n", getpid(), commandLine);

        // open pipes
        int result;
        if ((result = pipe(fd[i])) == -1)
        {
            perror("Failed to create pipe!\n");
            exit(1);
        }
        pid = fork();
        if (pid == -1)
        {
            perror("Failed to fork!\n");
            exit(1);
        }
        else if (pid == 0)
        {
            // close write and open read
            close(fd[i][1]);
            // redirect the pipe output
            dup2(fd[i][0], STDIN_FILENO);
            close(fd[i][0]);

            // strtok to get single command
            commandLine = argv[i + 1];
            char *commandArr[maxNum_eachCommand];
            int counter = 0;
            char *p;
            p = strtok(commandLine, " ");
            commandArr[counter] = p;
            // test:
            //printf("command %d: %s\n", i, commandArr[counter]);
            counter++;
            while (p != NULL)
            {
                p = strtok(NULL, " ");
                commandArr[counter] = p;
                // test: 
                // printf("command %d: %s\n", i, commandArr[counter]);
                counter++;
            }
            commandArr[counter] = NULL;
            // excute execvp
            if (execvp(commandArr[0], commandArr) < 0)
            {
                perror("Failed to execvp!\n");
                exit(1);
            }
            exit(0);
        }
        // sleep for correct output order
        usleep(200);
    }
    // close all read and open write
    int j;
    for (j = 0; j < argc - 1; j++)
    {
        close(fd[j][0]);
    }
    // get standard input
    char message[maxNum_eachLine];
    while (fgets(message, maxNum_eachLine, stdin) != NULL)
    {
        for (j = 0; j < argc - 1; j++) 
        {
            // send message to pipe
            write(fd[j][1], message, strlen(message));
        }
    }
    for (j = 0; j < argc - 1; j++)
    {
        close(fd[j][1]);
    }
    // wait all processes
    while (wait(&state) == -1)
    {
        printf("wait failed\n");
    }

    return 0;
}
