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
    for (i = 1; i < argc; i++)
    {
        commandLine = argv[i];
        printf("%d is about to start [%s]\n", getpid(), commandLine);
    }

    // create children processes
    int status;
    int fd[argc - 1][2];
    for (i = 0; i < argc; i++)
    {
        // create pipe
        int result;
        if ((result = pipe(fd[i])) == -1)
        {
            perror("Failed to create pipe!\n");
            exit(1);
        }
    }

    pid_t pid;
    for (i = 1; i < argc; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("Failed to fork!\n");
            exit(1);
        }
        else if (pid == 0)
        {
            commandLine = argv[i];
            close(fd[i][0]); // parent close read
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
                    break;
                }
                commandArr[counter] = p;
                printf("command: %s\n", commandArr[counter]);
                counter++;
            }
            // dup2(fd[i][0], STDOUT_FILENO);
            if (execvp(commandArr[0], commandArr) < 0)
            {
                perror("Failed to execvp!\n");
                exit(1);
            }
        }
        else
        {
            close(fd[i][1]); // parent close write
            /*
            int length;
            char message[maxNum_eachLine];
            while ((length = read(fd[i][0], message, maxNum_eachLine)) > 0)
            {
                message[length] = '\0';
                printf("%s\n", message);
            }*/
        }
    }
    /*
    while (1)
    {
        char message[maxNum_eachLine];
        printf("Please type something: ");
        fgets(message, sizeof(message), stdin);
        printf("\nYou typed: %s", message);
        write(fd[1], message, strlen(message) + 1);
    }*/

    // wait all processes
    while (wait(&status) == -1)
    {
        printf("wait failed\n");
    }

    return 0;
}
