#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define maxNum_eachLine 1024

int main(int argc, char *argv[])
{
    // exception
    if (argc < 2)
    {
        perror("Invalid arguments!\n");
        exit(1);
    }
    int i;
    for (i = 1; i < argc; i++)
    {
        char *commandName = argv[i];
        printf("%d is about to start [%s]\n", getpid(), commandName);
    }
    while (1)
    {
        char stdInput[maxNum_eachLine];
        printf("Please type something: ");
        fgets(stdInput, sizeof(stdInput), stdin);
        printf("\nYou typed: %s", stdInput);
    }

    return 0;
}
