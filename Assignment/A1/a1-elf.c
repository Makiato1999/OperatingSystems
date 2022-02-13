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
	// (section) address of the section header table in the file
	uint32_t sectHeaderAddr32b;
	uint64_t sectHeaderAddr64b;
	// size of an entry and the number of entries in the program header table
	uint16_t sizeOfProgEntry;
	uint16_t numOfProgHeader;
	// size of an entry and the number of entries in the section header table
	uint16_t sizeOfSectEntry;
	uint16_t numOfSectHeader;
	// entry in the section headers that is the string table
	// index of the section header table entry that contains the section names
	uint16_t indexOfNameTable;
} elf_header;

typedef struct PROGRAM_HEADER
{
	// segment type
	uint32_t segmentType;
	// segment Offset
	uint32_t segOffset32b;
	uint64_t segOffset64b;
	// virtual address of the segment in memory
	uint32_t virtualAddr32b;
	uint64_t virtualAddr64b;
	// size in the file image of the program header
	uint32_t sizeInImage32b;
	uint64_t sizeInImage64b;
	// store first up to 32 bytes for program header
	unsigned char database[32];
} program_header;

typedef struct SECTION_HEADER
{
	// An offset to a string in the .shstrtab section that represents the name of this section.
	uint32_t nameOffset;
	// section header type
	uint32_t type;
	// virtual address of the section in memory
	uint32_t virtualAddr32b;
	uint64_t virtualAddr64b;
	// size in the file image of the section header
	uint32_t sizeInImage32b;
	uint64_t sizeInImage64b;
	// store first up to 32 bytes for program header
	unsigned char database[32];
} section_header;

// variables
typedef enum VERSION
{
	// 32-bits is 0, 64-bits is 1
	version_32bits = 0,
	version_64bits
} version;
version flag;
uint64_t offset = 0;	// bytes that the pointer has moved
int rest = 0;			// bytes that each parts has leaved
uint64_t startAddr = 0; // each parts start address (each program or section)
unsigned char *name;	// section name
int tempAddr = 0;

#pragma pack(pop)

void check_if_elf(elf_header *elf_header, int handle);
void read_elf_header(elf_header *elf_header, int handle);
void print_elf_header(elf_header *elf_header);
void read_program_header(program_header *program_header, elf_header *elf_header, int handle);
void print_program_header(program_header *program_header, int index);
void read_section_header(section_header *section_header, elf_header *elf_header, int handle);
void print_section_header(section_header *section_header, int index);

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
	program_header program_header;
	read_program_header(&program_header, &elf_header, fd);
	section_header section_header;
	read_section_header(&section_header, &elf_header, fd);

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
		// (section) address of the section header table in the file
		read(handle, &elf_header->sectHeaderAddr32b, 4);
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
		read(handle, &elf_header->numOfSectHeader, 2);
		// entry in the section headers that is the string table
		read(handle, &elf_header->indexOfNameTable, 2);
	}
	else if (flag == version_64bits)
	{
		// address of the entry point from where the process starts executing
		read(handle, &elf_header->entryAddr64b, 8);
		// address of the program header table in the file
		read(handle, &elf_header->progHeaderAddr64b, 8);
		// (section) address of the section header table in the file
		read(handle, &elf_header->sectHeaderAddr64b, 8);
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
		read(handle, &elf_header->numOfSectHeader, 2);
		// entry in the section headers that is the string table
		read(handle, &elf_header->indexOfNameTable, 2);
	}
	// output
	print_elf_header(elf_header);
}
//------------------------------------------------------
// myRoutine: print_elf_header
//
// PURPOSE: print elf header information
// INPUT PARAMETERS:
//	   elf_header *elf_header
//------------------------------------------------------
void print_elf_header(elf_header *elf_header)
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
		printf("* there are %d section headers, each is %d bytes\n", elf_header->numOfSectHeader, elf_header->sizeOfSectEntry);
		// entry in the section headers that is the string table
		printf("* the section header string table is %d\n", elf_header->indexOfNameTable);
	}
	else if (flag == version_64bits)
	{
		// address of the entry point from where the process starts executing
		printf("* entry point address 0x%016lx\n", elf_header->entryAddr64b);
		// address of the program header table in the file
		printf("* program header table starts at 0x%016lx\n", elf_header->progHeaderAddr64b);
		// size of an entry and the number of entries in the program header table
		printf("* there are %d program headers, each is %d bytes\n", elf_header->numOfProgHeader, elf_header->sizeOfProgEntry);
		// size of an entry and the number of entries in the section header table
		printf("* there are %d section headers, each is %d bytes\n", elf_header->numOfSectHeader, elf_header->sizeOfSectEntry);
		// entry in the section headers that is the string table
		printf("* the section header string table is %d\n", elf_header->indexOfNameTable);
	}
}
//------------------------------------------------------
// myRoutine: read_program_header
//
// PURPOSE: read program head
// INPUT PARAMETERS:
//     program_header *header
//     elf_header *elf_header
//	   int handle
//------------------------------------------------------
void read_program_header(program_header *program_header, elf_header *elf_header, int handle)
{
	assert(program_header != NULL);
	assert(handle >= 0);

	if (flag == version_32bits)
	{
		startAddr = elf_header->progHeaderAddr32b;
		int i;
		for (i = 0; i < elf_header->numOfProgHeader; i++)
		{
			// move to program table address
			lseek(handle, startAddr, SEEK_SET);
			// segment type
			read(handle, &program_header->segmentType, 4);
			offset += 4;
			// skip 0 byte
			lseek(handle, 0, SEEK_CUR);
			// segment offset
			read(handle, &program_header->segOffset32b, 4);
			offset += 4;
			// virtual address of the segment in memory
			read(handle, &program_header->virtualAddr32b, 4);
			offset += 4;
			// skip 4 byte
			lseek(handle, 4, SEEK_CUR);
			offset += 4;
			// size in the file image of the program header
			read(handle, &program_header->sizeInImage32b, 4);
			offset += 4;
			// skip 4+4+4 byte
			lseek(handle, 12, SEEK_CUR);
			offset += 12;
			assert(offset == 32);
			// skip to next program header
			rest = (elf_header->sizeOfProgEntry) - (offset);
			tempAddr = lseek(handle, rest, SEEK_CUR);

			// store first up to 32 bytes for segment data
			lseek(handle, program_header->segOffset32b, SEEK_SET);
			int j;
			for (j = 0; j < 32; j++)
			{
				read(handle, &program_header->database[j], 1);
			}
			lseek(handle, tempAddr, SEEK_SET); // go back to beginning of program header

			// output
			print_program_header(program_header, i);
			// update start Address
			startAddr = lseek(handle, 0, SEEK_CUR);
			// update offset
			offset = 0;
		}
	}
	else if (flag == version_64bits)
	{
		startAddr = elf_header->progHeaderAddr64b;
		int i;
		for (i = 0; i < elf_header->numOfProgHeader; i++)
		{
			// move to program table address
			lseek(handle, startAddr, SEEK_SET);
			// segment type
			read(handle, &program_header->segmentType, 4);
			offset += 4;
			// skip 4 byte
			lseek(handle, 4, SEEK_CUR);
			offset += 4;
			// segment offset
			read(handle, &program_header->segOffset64b, 8);
			offset += 8;
			// virtual address of the segment in memory
			read(handle, &program_header->virtualAddr64b, 8);
			offset += 8;
			// skip 8 byte
			lseek(handle, 8, SEEK_CUR);
			offset += 8;
			// size in the file image of the program header
			read(handle, &program_header->sizeInImage64b, 8);
			offset += 8;
			// skip 8+8 byte
			lseek(handle, 16, SEEK_CUR);
			offset += 16;
			assert(offset == 56);
			// skip to next program header
			rest = (elf_header->sizeOfProgEntry) - (offset);
			tempAddr = lseek(handle, rest, SEEK_CUR);

			// store first up to 32 bytes for segment data
			lseek(handle, program_header->segOffset64b, SEEK_SET);
			int j;
			for (j = 0; j < 32; j++)
			{
				read(handle, &program_header->database[j], 1);
			}
			lseek(handle, tempAddr, SEEK_SET); // go back to beginning of program header

			// output
			print_program_header(program_header, i);
			// update start Address
			startAddr = lseek(handle, 0, SEEK_CUR);
			// update offset
			offset = 0;
		}
	}
	startAddr = 0;
}
//------------------------------------------------------
// myRoutine: print_program_header
//
// PURPOSE: print program head information
// INPUT PARAMETERS:
//	   program_header *program_header
//     int index
//------------------------------------------------------
void print_program_header(program_header *program_header, int index)
{
	printf("\nProgram header #%d: \n", index);
	// segment type
	printf("* segment type 0x%08x\n", program_header->segmentType);
	if (flag == version_32bits)
	{
		// virtual address of the segment in memory
		printf("* virtual address of segment 0x%016x\n", program_header->virtualAddr32b);
		// size in the file image of the program header
		printf("* size in file %u bytes\n", program_header->sizeInImage32b);
		// program header table starts at
		printf("* first up to 32 bytes starting at 0x%016x:\n", program_header->segOffset32b);
	}
	else if (flag == version_64bits)
	{
		// virtual address of the segment in memory
		printf("* virtual address of segment 0x%016lx\n", program_header->virtualAddr64b);
		// size in the file image of the program header
		printf("* size in file %lu bytes\n", program_header->sizeInImage64b);
		// program header table starts at
		printf("* first up to 32 bytes starting at 0x%016lx:\n", program_header->segOffset64b);
	}

	int i;
	for (i = 0; i < 32; i++)
	{
		if (i == 16)
		{
			printf("\n");
		}
		printf("%02x ", (program_header->database[i]));
	}
	printf("\n");
}
//------------------------------------------------------
// myRoutine: read_section_header
//
// PURPOSE: read section head
// INPUT PARAMETERS:
//     section_header *header
//	   elf_header *elf_header
//	   int handle
//------------------------------------------------------
void read_section_header(section_header *section_header, elf_header *elf_header, int handle)
{
	assert(section_header != NULL);
	assert(handle >= 0);

	if (flag == version_32bits)
	{
		startAddr = elf_header->sectHeaderAddr32b;
		int i;
		for (i = 0; i < elf_header->numOfSectHeader; i++)
		{
			// move to section table address
			lseek(handle, startAddr, SEEK_SET);
			// store first up to 32 bytes for section header
			int j;
			for (j = 0; j < 32; j++)
			{
				read(handle, &section_header->database[j], 1);
			}
			lseek(handle, -32, SEEK_CUR); // go back to beginning of section header
			// An offset to a string in the .shstrtab section that represents the name of this section.
			read(handle, &section_header->nameOffset, 4);
			offset += 4;
			// section header type
			read(handle, &section_header->type, 4);
			offset += 4;
			// skip 4 byte
			lseek(handle, 4, SEEK_CUR);
			offset += 4;
			// virtual address of the section in memory
			read(handle, &section_header->virtualAddr32b, 4);
			offset += 4;
			// skip 4 byte
			lseek(handle, 4, SEEK_CUR);
			offset += 4;
			// size in the file image of the section header
			read(handle, &section_header->sizeInImage32b, 4);
			offset += 4;
			assert(offset == 24);
			// skip to next section header
			rest = (elf_header->sizeOfSectEntry) - (offset);
			lseek(handle, rest, SEEK_CUR);

			// output
			print_section_header(section_header, i);
			// update start Address
			startAddr = lseek(handle, 0, SEEK_CUR);
			;
			// update offset
			offset = 0;
		}
	}
	else if (flag == version_64bits)
	{
		startAddr = elf_header->sectHeaderAddr64b;
		int i;
		for (i = 0; i < elf_header->numOfSectHeader; i++)
		{
			// move to section table address
			lseek(handle, startAddr, SEEK_SET);
			// store first up to 32 bytes for section header
			int j;
			for (j = 0; j < 32; j++)
			{
				read(handle, &section_header->database[j], 1);
			}
			lseek(handle, -32, SEEK_CUR); // go back to beginning of section header
			// An offset to a string in the .shstrtab section that represents the name of this section.
			read(handle, &section_header->nameOffset, 4);
			offset += 4;
			// section header type
			read(handle, &section_header->type, 4);
			offset += 4;
			// skip 8 byte
			lseek(handle, 8, SEEK_CUR);
			offset += 8;
			// virtual address of the section in memory
			read(handle, &section_header->virtualAddr64b, 8);
			offset += 8;
			// skip 8 byte
			lseek(handle, 8, SEEK_CUR);
			offset += 8;
			// size in the file image of the section header
			read(handle, &section_header->sizeInImage64b, 8);
			offset += 8;
			assert(offset == 40);
			// skip to next section header
			rest = (elf_header->sizeOfSectEntry) - (offset);
			lseek(handle, rest, SEEK_CUR);

			// output
			/*
			// get name
			lseek(handle, elf_header->sectHeaderAddr64b, SEEK_SET);								  // go to the section beginning address
			int StringTableAddr = (elf_header->indexOfNameTable) * (elf_header->sizeOfSectEntry); // find string table
			lseek(handle, StringTableAddr, SEEK_CUR);											  // jump to the string table beginning
			name = malloc(sizeof(char) * (section_header->nameOffset));							  // request space from heap																				  // save name as string (char *)
			lseek(handle, startAddr, SEEK_SET);													  // go home
			lseek(handle, offset, SEEK_CUR);*/

			print_section_header(section_header, i);
			// update start Address
			startAddr = lseek(handle, 0, SEEK_CUR);
			// update offset
			offset = 0;
		}
	}
}
//------------------------------------------------------
// myRoutine: print_section_header
//
// PURPOSE: print section head information
// INPUT PARAMETERS:
//	   section_header *section_header
//     int index
//------------------------------------------------------
void print_section_header(section_header *section_header, int index)
{
	printf("\nSection header #%d: \n", index);
	// name
	printf("* section name offset %x\n", section_header->nameOffset);

	// type
	printf("* type 0x%02x\n", section_header->type);
	if (flag == version_32bits)
	{
		// virtual address of the section in memory
		printf("* virtual address of section 0x%016x\n", section_header->virtualAddr32b);
		// size in the file image of the section header
		printf("* size in file %u bytes\n", section_header->sizeInImage32b);
		// Section header table starts at
		printf("* first up to 32 bytes starting at 0x%016lx:\n", startAddr);
	}
	else if (flag == version_64bits)
	{
		// virtual address of the section in memory
		printf("* virtual address of section 0x%016lx\n", section_header->virtualAddr64b);
		// size in the file image of the section header
		printf("* size in file %lu bytes\n", section_header->sizeInImage64b);
		// Section header table starts at
		printf("* first up to 32 bytes starting at 0x%016lx:\n", startAddr);
	}

	int i;
	for (i = 0; i < 32; i++)
	{
		if (i == 16)
		{
			printf("\n");
		}
		printf("%02x ", (section_header->database[i]));
	}
	printf("\n");
}
