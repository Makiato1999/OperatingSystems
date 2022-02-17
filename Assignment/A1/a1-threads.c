#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum BOOLEAN
{
    false = 0,
    true
} boolean;
int i = 0;                     // control index of threads
int numOfWorkers = 0;          // current number of workers
int prevNumOfWorkers = 0;      // previous number of workers

int main()
{
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
    }

    return 0;
}