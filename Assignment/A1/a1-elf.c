//-----------------------------------------
// NAME: Xiaoran Xie
// STUDENT NUMBER: 7884702
// COURSE: COMP 3430, SECTION: A02
// INSTRUCTOR: Robert Guderian
// ASSIGNMENT: assignment 1, QUESTION: a1-elf
// 
// REMARKS: Reading and interpreting binary files (ELF)
//
//-----------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#pragma pack(push)
#pragma pack(1)
typedef struct FILE_HEADER
{
    uint32_t magicNumber;
	uint8_t class;
	uint8_t endianness;
} file_header;
typedef struct PROGRAM_HEADER
{
} program_header;
typedef struct SECTION_HEADER
{
} section_header;
#pragma pack(pop)

void readFile(coff_header *header, int handle);
void printInfo(coff_header *header, char *fileName);

int main(int argc, char *argv[])
{
    //exception
	if (argc < 2)
	{
		printf("Invalid arguments\n");
		exit(0);	
	}
    char *filename = argv[1];
    int fd = open(fileName, O_RDONLY);

    return EXIT_SUCCESS;
}
