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
void parse_buffer(char buffer[], int length);
void parse_commandLine(char line[], int length);

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
    char buffer[maxNum_eachLine];
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
        fgets(buffer, maxNum_eachLine, fp);
        printf("buffer: %s\n", buffer);
        parse_buffer(buffer, strlen(buffer));
        printf("\n");
    }
    fclose(fp);
    printf("\n");
}
//------------------------------------------------------
// myRoutine: parse_buffer
//
// PURPOSE: parse buffer stream
// INPUT PARAMETERS:
//	   char buffer[]
//     int length
//------------------------------------------------------
void parse_buffer(char buffer[], int length)
{
    // calculate number of pipes in command line
    int numOfPipe = 0;
    int i;
    for (i = 0; i < length; i++)
    {
        if (buffer[i] == '|')
        {
            numOfPipe++;
            printf("numOfPipe: %d\n", numOfPipe);
        }
    }
    // return the index of the pipe in command line
    int indexOfpipe[numOfPipe];
    int index = 0;
    for (i = 0; i < length; i++)
    {
        if (buffer[i] == '|')
        {
            indexOfpipe[index] = i;
            printf("indexOfPipe: %d\n", indexOfpipe[index]);
            index++;
        }
    }
    // if there is no pipe in command line
    if (numOfPipe == 0)
    {
        // parse the command line
        parse_commandLine(buffer, length);
    }
    // if there are pipes in command line
    else if (numOfPipe > 0)
    {
        // create processes by using pipe numbers
    }
}
//------------------------------------------------------
// myRoutine: parse_commandLine
//
// PURPOSE: parse command line
// INPUT PARAMETERS:
//	   char line[]
//     int length
//------------------------------------------------------
void parse_commandLine(char line[], int length)
{
    int indexOfRedirect = 0;
    // return the index of the pipe in command line
    boolean isFind = false;
    int i;
    for (i = 0; i < length && !isFind; i++)
    {
        if (line[i] == '<' || line[i] == '>')
        {
            indexOfRedirect = i;
            isFind = true;
            printf("indexOfPipe: %d\n", indexOfRedirect);
        }
    }
    // if there is no redirect in command line
    if (isFind == false)
    {
        
    }
    // if there is redirect in command line
    else if (/* condition */)
    {
        /* code */
    }
}
//------------------------------------------------------
// myRoutine: redirection_implement
//
// PURPOSE: check redirection sign
// INPUT PARAMETERS:
//	   char *line[]
//     char *temp_array[]
//     bool ifredirection
//     int sign_index
//------------------------------------------------------
redirection_implement(line, temp_array2, ifredirection, sign_index)
{
}
