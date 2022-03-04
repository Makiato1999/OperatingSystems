#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <string.h>

#define maxNum_eachLine 1024
#define maxNum_eachCommand 20
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
    char *commandLine;
    for (i = 0; i < argc - 1; i++)
    {
        commandLine = argv[i + 1];
        printf("%d is about to start [%s]\n", getpid(), commandLine);
    }

    // create pipes
    int state;
    int fd[argc - 1][2];
    for (i = 0; i < argc - 1; i++)
    {
        int result;
        if ((result = pipe(fd[i])) == -1)
        {
            perror("Failed to create pipe!\n");
            exit(1);
        }
    }

    pid_t pid;
    for (i = 0; i < argc - 1; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("Failed to fork!\n");
            exit(1);
        }
        else if (pid == 0)
        {
            commandLine = argv[i + 1];
            close(fd[i][1]);
            dup2(fd[i][0], STDIN_FILENO);
            close(fd[i][0]);
            /*
            int counter = 0;
            char *commandArr[maxNum_eachCommand];
            commandArr[counter] = strtok(commandLine, " ");
            printf("command: %s\n", commandArr[counter]);
            while (commandArr[counter] != NULL) 
            {
                counter++;
                commandArr[counter] = strtok(NULL, " ");
                printf("command: %s\n", commandArr[counter]);
            }
            commandArr[counter] = NULL;
            */
            // strtok
            char *commandArr[maxNum_eachCommand];
            int counter = 0;
            char *p;
            p = strtok(commandLine, " ");
            commandArr[counter] = p;
            printf("command: %s\n", commandArr[counter]);
            counter++;
            while (1)
            {
                p = strtok(NULL, " ");
                if (p == NULL)
                {
                    commandArr[counter] = NULL;
                    break;
                }
                commandArr[counter] = p;
                printf("command: %s\n", commandArr[counter]);
                counter++;
            }

            if (execvp(commandArr[0], commandArr) < 0)
            {
                perror("Failed to execvp!\n");
                exit(1);
            }
            exit(0);
        }
        usleep(200);
    }
    int j;
    for (j = 0; j < argc - 1; j++)
    {
        close(fd[j][0]); // parent close write
    }
    
    char message[maxNum_eachLine];
    while (fgets(message, maxNum_eachLine, stdin) != NULL)
    {
        for (j = 0; j < argc - 1; j++) 
        {
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
