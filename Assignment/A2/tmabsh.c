//-----------------------------------------
// NAME: Xiaoran Xie
// STUDENT NUMBER: 7884702
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Robert Guderian
// ASSIGNMENT: assignment 2
//
// REMARKS: read script file / shell commands
//
//-----------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define maxNum_eachLine 100

#pragma pack(push)
#pragma pack(1)
typedef enum BOOLEAN
{
    true = 0,
    false
} boolean;
boolean isPipe;
#pragma pack(pop)

void read_script_file(char *scriptName);
void parse_buffer(char buffer[]);
void parse_commandLine_noPipe(char *commandLine);
void redirection_implement(char *sub_commandLine[], int length);
void exe_func(char *execvp_command[]);

int main(int argc, char *argv[])
{
    // execption
    if (argc < 2)
    {
        perror("Invalid arguments!\n");
        exit(1);
    }
    // read script file
    char *scriptName = argv[1];
    read_script_file(scriptName);

    return EXIT_SUCCESS; // exit(0)
}
//------------------------------------------------------
// myRoutine: read_script_file
//
// PURPOSE: read script file
// INPUT PARAMETERS:
//	   char *scriptName
//------------------------------------------------------
void read_script_file(char *scriptName)
{
    // read script file
    FILE *fp = NULL;
    char p[maxNum_eachLine];
    fp = fopen(scriptName, "r");
    // exception
    if (fp == NULL)
    {
        perror("Failed to open script file!\n");
        exit(1);
    }

    while (!feof(fp))
    {
        // read each line
        fgets(p, maxNum_eachLine, fp);
        // @test:
        char *buffer;
        buffer = strtok(p, "\t\r\n");
        printf("buffer: %s\n", buffer);
        parse_buffer(buffer);
        printf("\n");
    }
    fclose(fp);
}
//------------------------------------------------------
// myRoutine: parse_buffer
//
// PURPOSE: parse buffer stream
// INPUT PARAMETERS:
//	   char buffer[]
//------------------------------------------------------
void parse_buffer(char buffer[])
{
    char *commandLine[maxNum_eachLine];
    int counter = 0;
    int numOfPipe = -1;
    char *p;
    // whether there is "|" in command line
    p = strtok(buffer, "|");
    while (p != NULL)
    {
        commandLine[counter] = p;
        numOfPipe++;
        // @test:
        // printf("command %d: %s\n", counter, commandLine[counter]);
        counter++;
        p = strtok(NULL, "|");
    }

    // if there is no pipe in command line
    if (numOfPipe == 0)
    {
        // commandLine is like "head -5 words > first5words.txt"
        parse_commandLine_noPipe(commandLine[counter - 1]);
    }
    // if there are pipes in command line
    else if (numOfPipe > 0)
    {
        // create processes by using pipe numbers
    }
}
//------------------------------------------------------
// myRoutine: parse_commandLine_noPipe
//
// PURPOSE: parse command line without pipe
// INPUT PARAMETERS:
//	   char *commandLine
//------------------------------------------------------
void parse_commandLine_noPipe(char *commandLine)
{
    // whether there is "<" or ">"
    // commandLine is like "head -5 words > first5words.txt"
    // sub_commandLineis[0] is like "head" in "head -5 words > first5words.txt"
    char *sub_commandLine[maxNum_eachLine];
    char *commandKeyword;
    int counter = 0;
    commandKeyword = strtok(commandLine, " \t\r\n");
    while (commandKeyword != NULL)
    {
        sub_commandLine[counter] = commandKeyword;
        // @test:
        printf("sub_commandLine[%d]: %s\n", counter, sub_commandLine[counter]);
        counter++;
        commandKeyword = strtok(NULL, " \t\r\n");
    }
    // sub_commandLine[counter] = NULL;
    redirection_implement(sub_commandLine, counter);
}
//------------------------------------------------------
// myRoutine: redirection_implement
//
// PURPOSE: check redirection sign
// INPUT PARAMETERS:
// char *sub_commandLine[]
// char *redirectFile
//------------------------------------------------------
void redirection_implement(char *sub_commandLine[], int length)
{
    int state;
    pid_t pid;
    if ((pid = fork()) < 0)
    {
        perror("Failed to fork!\n");
        exit(1);
    }
    if (pid == 0)
    {
        int fd;
        int nfd;
        boolean isFind = false;
        char *execvpPath[maxNum_eachLine];
        char *redirectFile = "";
        int j = 0;
        int i = 0;
        for (i = 0; (i < length) && (isFind == false); i++)
        {
            // find the redirect file which is like "first5words.txt"
            if (strcmp(sub_commandLine[i], ">") == 0)
            {
                redirectFile = (char *)malloc(strlen(sub_commandLine[i + 1]) * sizeof(char));
                strcpy(redirectFile, sub_commandLine[i + 1]);
                // @test:
                printf("redirectFile: %s\n", redirectFile);
                
                // execute redirect command
                if ((fd = open(redirectFile, O_RDWR | O_CREAT, 0666)) < 0)
                {
                    perror("Failed to open file!\n");
                    exit(1);
                }
                if ((nfd = dup2(fd, STDOUT_FILENO)) < 0)
                {
                    perror("Failed to redirect!\n");
                    exit(1);
                }
                close(fd);
                while (strcmp(sub_commandLine[j], ">") != 0)
                {
                    execvpPath[j] = sub_commandLine[j];
                    j++;
                }
                free(redirectFile);
                isFind = true;

                if (execvp(execvpPath[0], execvpPath) < 0)
                {
                    perror("Failed to execute!\n");
                    exit(1);
                }
                // redirect done
            }
            else if (strcmp(sub_commandLine[i], "<") == 0)
            {
                redirectFile = (char *)malloc(strlen(sub_commandLine[i + 1]) * sizeof(char));
                strcpy(redirectFile, sub_commandLine[i + 1]);
                // @test:
                printf("redirectFile: %s\n", redirectFile);
                // execute redirect command
                if ((fd = open(redirectFile, O_RDONLY, 7777)) < 0)
                {
                    perror("Failed to open file!\n");
                    exit(1);
                }
                if ((nfd = dup2(fd, STDIN_FILENO)) < 0)
                {
                    perror("Failed to redirect!\n");
                    exit(1);
                }
                close(fd);
                while (strcmp(sub_commandLine[j], "<") != 0)
                {
                    execvpPath[j] = sub_commandLine[j];
                    j++;
                }
                free(redirectFile);
                isFind = true;

                if (execvp(execvpPath[0], execvpPath) < 0)
                {
                    perror("Failed to execute!\n");
                    exit(1);
                }
                // redirect done
            }
        }
        if (isFind == false)
        {
            while (sub_commandLine[j] != NULL)
            {
                execvpPath[j] = sub_commandLine[j];
                j++;
            }
            if (execvp(execvpPath[0], execvpPath) < 0)
            {
                perror("Failed to execute!\n");
                exit(1);
            }
        }
        exit(0);
    }
    // wait all processes
    while (wait(&state) == -1)
    {
        perror("wait failed\n");
    }
}
//------------------------------------------------------
// myRoutine: exe_func
//
// PURPOSE: implement exec function
// INPUT PARAMETERS:
//     char *execvp_command[]
//------------------------------------------------------
void exe_func(char *execvp_command[])
{
    if (execvp(execvp_command[0], execvp_command) < 0)
    {
        perror("Failed to execute!\n");
        exit(1);
    }
}
