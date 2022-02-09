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
	// 'ELF'
	unsigned char magicNumber[4];
	// program is 32-bit or 64-bit
	uint8_t class;
	// endianness of the file
	uint8_t endianness;
	// target operating system ABI
	uint8_t osABI;
	// object file type
	uint16_t objType;
	// instruction set architecture
	uint16_t isa;
	// address of the entry point from where the process starts executing
	uint32_t entryAddr32b;
	uint64_t entryAddr64b;
	// address of the program header table in the file
	uint32_t progHeaderAddr32b;
	uint64_t progHeaderAddr64b;
	// size of an entry and the number of entries in the program header table
	uint16_t sizeOfProgEntry;
	uint16_t numOfProgHeader;
	// size of an entry and the number of entries in the section header table
	uint16_t sizeOfSectEntry;
	uint16_t numOfSecteader;
	// entry in the section headers that is the string table
	uint16_t entrySectHeaders;
} elf_header;
#pragma pack(pop)

typedef enum VERSION
{
	// 32-bits is 0, 64-bits is 1
	version_32bits = 0,
	version_64bits
} version;
version flag;

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
	assert(elf_header != NULL);
	assert(handle >= 0);

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

	// program is 32-bit or 64-bit
	read(handle, &elf_header->class, 1);
	if (elf_header->class == 1)
	{
		flag = version_32bits;
	}
	else if (elf_header->class == 2)
	{
		flag = version_64bits;
	}
	// endianness of the file
	read(handle, &elf_header->endianness, 1);
	// skip 1 byte
	lseek(handle, 1, SEEK_CUR);
	// target operating system ABI
	read(handle, &elf_header->osABI, 1);
	// skip offset and size of EI_PAD are 8bytes = 64bits
	lseek(handle, 8, SEEK_CUR);
	// object file type
	read(handle, &elf_header->objType, 2);
	// instruction set architecture
	read(handle, &elf_header->isa, 2);
	// skip 4 bytes
	lseek(handle, 4, SEEK_CUR);
	if (flag == version_32bits)
	{
		// address of the entry point from where the process starts executing
		read(handle, &elf_header->entryAddr32b, 4);
		// address of the program header table in the file
		read(handle, &elf_header->progHeaderAddr32b, 4);
		// skip 4 bytes for the start of the section header table
		lseek(handle, 4, SEEK_CUR);
		// skip 4 bytes for Interpretation of this field depends on the target architecture.
		lseek(handle, 4, SEEK_CUR);
		// skip 2 bytes for the size of this header
		lseek(handle, 2, SEEK_CUR);
		// size of an entry in the program header table
		read(handle, &elf_header->sizeOfProgEntry, 2);
		// number of entries in the program header table
		read(handle, &elf_header->numOfProgHeader, 2);
		// size of an entry 
		read(handle, &elf_header->sizeOfSectEntry, 2);
		// number of entries in the section header table
		read(handle, &elf_header->numOfSecteader, 2);
		// entry in the section headers that is the string table
		read(handle, &elf_header->entrySectHeaders, 2);
	}
	else if (flag == version_64bits)
	{
		// address of the entry point from where the process starts executing
		read(handle, &elf_header->entryAddr64b, 8);
		// address of the program header table in the file
		read(handle, &elf_header->progHeaderAddr64b, 8);
		// skip 8 bytes for the start of the section header table
		lseek(handle, 8, SEEK_CUR);
		// skip 4 bytes for Interpretation of this field depends on the target architecture.
		lseek(handle, 4, SEEK_CUR);
		// skip 2 bytes for the size of this header
		lseek(handle, 2, SEEK_CUR);
		// size of an entry in the program header table
		read(handle, &elf_header->sizeOfProgEntry, 2);
		// number of entries in the program header table
		read(handle, &elf_header->numOfProgHeader, 2);
		// size of an entry 
		read(handle, &elf_header->sizeOfSectEntry, 2);
		// number of entries in the section header table
		read(handle, &elf_header->numOfSecteader, 2);
		// entry in the section headers that is the string table
		read(handle, &elf_header->entrySectHeaders, 2);
	}
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

	// program is 32-bit or 64-bit
	if (flag == version_32bits)
	{
		printf("* 32-bit\n");
	}
	else if (flag == version_64bits)
	{
		printf("* 64-bit\n");
	}

	// endianness of the file
	if (elf_header->endianness == 1)
	{
		printf("* little endian\n");
	}
	else if (elf_header->endianness == 2)
	{
		printf("* big endian\n");
	}

	// target operating system ABI
	printf("* compiled for 0x%02x (operating system)\n", elf_header->osABI);

	// object file type
	printf("* has type 0x%02x\n", elf_header->objType);

	// instruction set architecture
	printf("* compiled for 0x%02x (isa)\n", elf_header->isa);

	if (flag == version_32bits)
	{
		// address of the entry point from where the process starts executing
		printf("* entry point address 0x%016x\n", elf_header->entryAddr32b);
		// address of the program header table in the file
		printf("* program header table starts at 0x%016x\n", elf_header->progHeaderAddr32b);
		// size of an entry and the number of entries in the program header table
		printf("* there are %d program headers, each is %d bytes\n", elf_header->numOfProgHeader, elf_header->sizeOfProgEntry);
		// size of an entry and the number of entries in the section header table
		printf("* there are %d section headers, each is %d bytes\n", elf_header->numOfSecteader, elf_header->sizeOfSectEntry);
		// entry in the section headers that is the string table
		printf("* the section header string table is %d\n", elf_header->entrySectHeaders);
	}
	else if (flag == version_64bits)
	{
		// address of the entry point from where the process starts executing
		printf("* entry point address 0x%016llx\n", elf_header->entryAddr64b);
		// address of the program header table in the file
		printf("* program header table starts at 0x%016llx\n", elf_header->progHeaderAddr64b);
		// size of an entry and the number of entries in the program header table
		printf("* there are %d program headers, each is %d bytes\n", elf_header->numOfProgHeader, elf_header->sizeOfProgEntry);
		// size of an entry and the number of entries in the section header table
		printf("* there are %d section headers, each is %d bytes\n", elf_header->numOfSecteader, elf_header->sizeOfSectEntry);
		// entry in the section headers that is the string table
		printf("* the section header string table is %d\n", elf_header->entrySectHeaders);
	}
}
