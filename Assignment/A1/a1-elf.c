//-----------------------------------------
// NAME: Xiaoran Xie
// STUDENT NUMBER: 7884702
// COURSE: COMP 3430, SECTION: A01
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
typedef struct ELF_HEADER
{
	unsigned char magicNumber[4];// 'ELF'
	uint8_t class;// 1 is 64-bits, 2 is 32-bits
	uint8_t endianness;
} elf_header;
#pragma pack(pop)

void check_if_elf(elf_header *elf_header, int handle);
void read_elf_header(elf_header *elf_header, int handle);
void print_elf_header_info(elf_header *elf_header);

int main(int argc, char *argv[])
{
	// exception
	if (argc < 2)
	{
		printf("Invalid arguments\n");
		exit(1);
	}
	// read file
	char *filename = argv[1];
	int fd = open(filename, O_RDONLY);

	// parse file
	elf_header elf_header;
	check_if_elf(&elf_header, fd);
	read_elf_header(&elf_header, fd);

	// output
	print_elf_header_info(&elf_header);

	return EXIT_SUCCESS;
}
//------------------------------------------------------
// myRoutine: check_if_elf
//
// PURPOSE: check if this is elf file
// INPUT PARAMETERS:
//     elf_header *elf_header
//	   int handle
//------------------------------------------------------
void check_if_elf(elf_header *elf_header, int handle)
{
	read(handle, &elf_header->magicNumber, 4);
	// check whether this is ELF file
	if (elf_header->magicNumber[1] != 'E' ||
		elf_header->magicNumber[2] != 'L' ||
		elf_header->magicNumber[3] != 'F')
	{
		perror("This is not ELF file\n");
		exit(1);
	}
}
//------------------------------------------------------
// myRoutine: read_elf_header
//
// PURPOSE: read elf header
// INPUT PARAMETERS:
//     elf_header *elf_header
//	   int handle
//------------------------------------------------------
void read_elf_header(elf_header *elf_header, int handle)
{
	assert(elf_header != NULL);
	assert(handle >= 0);

	read(handle, &elf_header->class, 1);
}
//------------------------------------------------------
// myRoutine: print_elf_header_info
//
// PURPOSE: print elf header information
// INPUT PARAMETERS:
//	   elf_header *elf_header
//------------------------------------------------------
void print_elf_header_info(elf_header *elf_header)
{
	printf("ELF header:\n");
	if (elf_header->class == 1)
	{
		printf("* 32-bit\n");
	}
	else if (elf_header->class == 2)
	{
		printf("* 64-bit\n");
	}
}

