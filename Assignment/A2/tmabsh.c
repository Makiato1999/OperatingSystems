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
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define maxNum_eachLine 1024

void read_script_file(char *scriptName);

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
    
    return EXIT_SUCCESS;// exit(0)
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
        char *line[maxNum_eachLine];	
        // split the line and get command
        char *temp;
        int index = 0;
        int length = 0;
        temp = strtok(buffer, " \t\r\n");
        while (temp != NULL)
        {
            line[index] = temp;
            printf("(%s)\n", line[index]);
            index++;
            length++;
            // next
            temp = strtok(NULL, " \t\r\n");// 如果是结尾的string，里面会自带一个\n\0
        }
        parse_line(line, length);
        // printf("**** %s \n", buffer);
        printf("\n"); 	   			
	}
	fclose(fp);  
	printf("\n");
}