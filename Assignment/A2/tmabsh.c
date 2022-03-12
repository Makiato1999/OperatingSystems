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
#include <sys/stat.h>
#include <assert.h>

#define maxNum_eachLine 100
#define permission 0664

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
void parse_commandLine_pipes(char *commandLine[], int length);
void redirection_implement(char *sub_commandLine[], int length);
void processSubstitution_implement(char *sub_commandLine);
void exe_func(char *execvp_command[]);
void trim(char *strIn, char *strOut);

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
        printf("CommandLine: %s\n", buffer);
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
    char *p;
    int counter = 0;
    int numOfPipe = 0;
    int length = 0;
    boolean isFindPipe = false;
    unsigned long i;
    // whether there is "|" in command line
    for (i = 0; i < strlen(buffer) && isFindPipe == false; i++)
    {
        if (buffer[i] == '|')
        {
            isFindPipe = true;
        }
    }

    if (isFindPipe == false)
    {
        // @test:
        // printf("notFindPipe!\n");
        p = strtok(buffer, "|");
        commandLine[0] = p;
        numOfPipe = 0;
        length = 0;
    }
    else if (isFindPipe == true)
    {
        // @test:
        // printf("isFindPipe!\n");
        // whether there is "|" in command line
        p = strtok(buffer, "|");
        commandLine[counter] = p;
        counter++;
        length++;
        numOfPipe++;
        while ((p = strtok(NULL, "|")) != NULL)
        {
            commandLine[counter] = p;
            length++;
            numOfPipe++;
            // @test:
            // printf("command %d: %s\n", counter, commandLine[counter]);
            counter++;
        }
        // @test
        // printf("length: %d!\n", length);
    }

    // if there is no pipe in command line
    if (numOfPipe == 0)
    {
        // commandLine is like "head -5 words > first5words.txt"
        parse_commandLine_noPipe(commandLine[0]);
    }
    // if there are pipes in command line
    else if (numOfPipe > 0)
    {
        // commandLine is like "sort -R < words | head -5 > rand5words.txt"
        // "sort -R < words | head -5 | sort -d > randsort5words.txt"
        parse_commandLine_pipes(commandLine, length);
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
    char *temp = (char *)malloc(strlen(commandLine) * sizeof(char));
    strcpy(temp, commandLine);

    // whether there is a process substitution
    boolean isFindProcessSubstit = false;
    unsigned long i;
    for (i = 0; i < strlen(temp) && isFindProcessSubstit == false; i++)
    {
        if (temp[i] == '(')
        {
            temp[i] = ' ';
        }
        else if (temp[i] == ')')
        {
            isFindProcessSubstit = true;
            temp[i] = '\0';
        }
    }

    if (isFindProcessSubstit == true)
    {
        // @test:
        // printf("- changed commandLine: (%s)\n", temp);
        processSubstitution_implement(temp);
    }
    else
    {
        // whether there is "<" or ">"
        // commandLine is like "head -5 words > first5words.txt"
        // sub_commandLineis[0] is like "head" in "head -5 words > first5words.txt"
        char *sub_commandLine[maxNum_eachLine];
        char *commandKeyword;
        int counter = 0;
        commandKeyword = strtok(commandLine, " \t\r\n");
        sub_commandLine[counter] = commandKeyword;
        // @test:
        // printf("- sub_commandLine[%d]: (%s)\n", counter, sub_commandLine[counter]);
        counter++;
        while ((commandKeyword = strtok(NULL, " \t\r\n")) != NULL)
        {
            sub_commandLine[counter] = commandKeyword;
            // @test:
            // printf("- sub_commandLine[%d]: (%s)\n", counter, sub_commandLine[counter]);
            counter++;
        }
        sub_commandLine[counter] = NULL;

        redirection_implement(sub_commandLine, counter);
    }
}
//------------------------------------------------------
// myRoutine: parse_commandLine_pipes
//
// PURPOSE: parse command line with pipes
// INPUT PARAMETERS:
//	   char *commandLine[]
//     int length
//------------------------------------------------------
void parse_commandLine_pipes(char *commandLine[], int length)
{
    char *recursionArr[maxNum_eachLine];
    // for avoiding commandLine[0] is like "sort -R < words ", we need to polish it
    // get updated commandLine without extra spaces
    char updated_commandLine[maxNum_eachLine];
    trim(commandLine[0], updated_commandLine);
    commandLine[0] = updated_commandLine;
    // @test
    // printf("curr length: %d, commandLine[0]: (%s)\n", length, commandLine[0]);

    int i;
    for (i = 1; i < length; i++)
    {
        char updated_commandLine[maxNum_eachLine];
        trim(commandLine[i], updated_commandLine);
        recursionArr[i - 1] = updated_commandLine;
        // @test:
        // printf("curr length: %d, recursionArr[%d]: (%s)\n", length, i - 1, recursionArr[i - 1]);
    }

    pid_t pid;
    pid_t next_pid;
    //  open pipes
    int fd[2];
    int state;
    int result;
    if ((result = pipe(fd)) == -1)
    {
        perror("Failed to create pipe!\n");
        exit(1);
    }
    // create processes
    if ((pid = fork()) < 0)
    {
        perror("Failed to fork!\n");
        exit(1);
    }
    if (pid == 0)
    {
        close(fd[0]);
        int nfd;
        if ((nfd = dup2(fd[1], STDOUT_FILENO)) < 0)
        {
            perror("Failed to dup2 redirect!\n");
            exit(1);
        }
        close(fd[1]);
        // commandLine is like "sort -R < words | head -5 > rand5words.txt"
        // commandLine[0] is like "sort -R < words"
        parse_commandLine_noPipe(commandLine[0]);
        exit(0);
    }
    else
    {
        // int state;
        if ((next_pid = fork()) < 0)
        {
            perror("Failed to fork!\n");
            exit(1);
        }
        if (next_pid == 0)
        {
            close(fd[1]);
            int nfd;
            if ((nfd = dup2(fd[0], STDIN_FILENO)) < 0)
            {
                perror("Failed to dup2 redirect!\n");
                exit(1);
            }
            close(fd[0]);

            if (length == 2)
            {
                // recursionArr[0] is like "head -5 > rand5words.txt"
                parse_commandLine_noPipe(recursionArr[0]);
            }
            else if (length > 2)
            {
                // "sort -R < words | head -5 | sort -d > randsort5words.txt"
                parse_commandLine_pipes(recursionArr, length - 1);
            }
            exit(0);
        }
        else
        {
            close(fd[0]);
            close(fd[1]);
        }
        // wait all processes
        while (wait(&state) == -1)
        {
            perror("wait failed\n");
        }
        printf("\nSuccessed to execute all commands!\n");
    }
}
//------------------------------------------------------
// myRoutine: redirection_implement
//
// PURPOSE: check redirection sign and process
// INPUT PARAMETERS:
//	   char *sub_commandLine[]
//	   int length
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
                // printf("redirectFile: %s\n", redirectFile);

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
                execvpPath[j] = NULL;
                free(redirectFile);
                isFind = true;

                exe_func(execvpPath);
                // redirect done
            }
            else if (strcmp(sub_commandLine[i], "<") == 0)
            {
                redirectFile = (char *)malloc(strlen(sub_commandLine[i + 1]) * sizeof(char));
                strcpy(redirectFile, sub_commandLine[i + 1]);
                // @test:
                // printf("redirectFile: %s\n", redirectFile);
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
                execvpPath[j] = NULL;
                free(redirectFile);
                isFind = true;

                exe_func(execvpPath);
                // redirect done
            }
        }
        if (isFind == false)
        {
            exe_func(sub_commandLine);
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
// myRoutine: processSubstitution_implement
//
// PURPOSE: check process Substitution
// INPUT PARAMETERS:
//	   char *commandLine
//------------------------------------------------------
void processSubstitution_implement(char *commandLine)
{
    char *sub_commandLine[maxNum_eachLine];
    char *outside_sub_commandLine[maxNum_eachLine];
    char *inside_sub_commandLine[maxNum_eachLine];
    char *commandKeyword;
    int counter = 0;
    int index = 0;
    int i = 0;
    // commandLine is like "head -5 < sort -R words"
    // inside_sub_commandLine[0] is like "sort"
    // inside_sub_commandLine[1] is like "-R"
    // inside_sub_commandLine[2] is like "words"
    commandKeyword = strtok(commandLine, " ");
    sub_commandLine[counter] = commandKeyword;
    // @test:
    // printf("- sub_commandLine[%d]: (%s)\n", counter, sub_commandLine[counter]);
    counter++;
    while ((commandKeyword = strtok(NULL, " ")) != NULL)
    {
        sub_commandLine[counter] = commandKeyword;
        // @test:
        // printf("- sub_commandLine[%d]: (%s)\n", counter, sub_commandLine[counter]);
        counter++;
    }
    while (strcmp(sub_commandLine[i], "<") != 0)
    {
        outside_sub_commandLine[i] = sub_commandLine[i];
        // printf("- outside_sub_commandLine[%d]: (%s)\n", i, outside_sub_commandLine[i]);
        i++;
    }
    outside_sub_commandLine[i] = NULL;
    i++;
    while (sub_commandLine[i] != NULL)
    {
        inside_sub_commandLine[index] = sub_commandLine[i];
        // printf("- inside_sub_commandLine[%d]: (%s)\n", index, inside_sub_commandLine[index]);
        i++;
        index++;
    }
    inside_sub_commandLine[index] = NULL;

    // execute
    // open fifo
    int fd;
    int nfd;
    int state;
    const char *fifoName = "./temp";
    int n;
    if ((n = mkfifo(fifoName, S_IRUSR | S_IWUSR)) < 0)
    {
        perror("Failed to create fifo!\n");
        exit(1);
    }
    pid_t pid;
    if ((pid = fork()) < 0)
    {
        perror("Failed to fork!\n");
        exit(1);
    }
    if (pid == 0)
    {
        if ((fd = open(fifoName, O_WRONLY)) == -1)
        {
            perror("Failed to open fifo!\n");
        }
        if ((nfd = dup2(fd, STDOUT_FILENO)) < 0)
        {
            perror("Failed to redirect!\n");
            exit(1);
        }
        close(fd);
        unlink(fifoName);
        exe_func(inside_sub_commandLine);
    }
    // wait all processes
    while (wait(&state) == -1)
    {
        perror("wait failed\n");
    }

    if ((fd = open(fifoName, O_RDONLY)) == -1)
    {
        perror("Failed to open fifo!\n");
    }
    if ((nfd = dup2(fd, STDIN_FILENO)) < 0)
    {
        perror("Failed to redirect!\n");
        exit(1);
    }
    close(fd);
    unlink(fifoName);
    exe_func(outside_sub_commandLine);
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
//------------------------------------------------------
// myRoutine: trim
//
// PURPOSE: remove space from head and tail for a String
// INPUT PARAMETERS:
//     char *strIn, char *strOut
//------------------------------------------------------
void trim(char *strIn, char *strOut)
{
    int i = 0;
    int j;
    j = strlen(strIn) - 1;
    while (strIn[i] == ' ')
    {
        ++i;
    }
    while (strIn[j] == ' ')
    {
        --j;
    }
    strncpy(strOut, strIn + i, j - i + 1);
    strOut[j - i + 1] = '\0';
}
