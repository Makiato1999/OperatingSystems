#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

#pragma pack(push)
#pragma pack(1)
typedef struct COFF_HEADER
{
    // MS-DOS Stub
    uint32_t stub;// ...?
    uint32_t offset;
    // Signature
    unsigned char Signature[4];
    // coff file header
    uint16_t MachineType;
    uint16_t NumberOfSections;
    uint32_t CreateTime;
    uint32_t SymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
} coff_header;
#pragma pack(pop)

void readFile(coff_header *header, int handle);
void printInfo(coff_header *header, char *fileName);

int main(int argc, char *argv[])
{
    // exception
    if (argc < 2)
    {
        printf("Invalid arguments\n");
        exit(1);
    }
    // 64bit x86
    char *fileName = argv[1];
    coff_header header;
    int header_fd = open(fileName, O_RDONLY);
    readFile(&header, header_fd);
    printInfo(&header, fileName);
    return EXIT_SUCCESS;
}

void readFile(coff_header *header, int handle)
{
    assert(header != NULL);
    assert(handle >= 0);

    // skip MS-DOS Stub
    ///*------------------------------------
    lseek(handle, 0x3c, SEEK_SET);
    read(handle, &header->offset, 4);
    lseek(handle, 0, SEEK_SET);
    lseek(handle, header->offset, SEEK_CUR);
    // skip Signature
    read(handle, &header->Signature, 4);
    //-------------------------------------*/
    // 2bytes for MachineType
    read(handle, &header->MachineType, 2);
    // 2bytes for NumberOfSections
    read(handle, &header->NumberOfSections, 2);
    // 4bytes for CreateTime
    read(handle, &header->CreateTime, 4);
    // 4bytes for SymbolTable, should be 0
    read(handle, &header->SymbolTable, 4);
    // 4bytes for NumberOfSymbols
    read(handle, &header->NumberOfSymbols, 4);
    // 2bytes for SizeOfOptionalHeader
    read(handle, &header->SizeOfOptionalHeader, 2);
    // 2bytes for Characteristics
    read(handle, &header->Characteristics, 2);
}

void printInfo(coff_header *header, char *fileName)
{
    assert(header != NULL);
    // check 'PE'
    ///*------------------------------------
    assert(header->Signature[0] == 'P');
    assert(header->Signature[1] == 'E');
    assert(header->Signature[2] == '\0');
    assert(header->Signature[3] == '\0');
    //-------------------------------------*/

    printf("File: %s\n\n", fileName);
    printf("Machine type: 0x%04x\n", header->MachineType);
    printf("Number of sections: %d\n", header->NumberOfSections);
    // The created time in a PE file
    time_t epoch = header->CreateTime;
    printf("Created: %s %u\n", asctime(gmtime(&epoch)), header->CreateTime);
    printf("Symbol table start: 0x%08x\n", header->SymbolTable);
    printf("Number of symbols: %d\n", header->NumberOfSymbols);
    printf("Size of optional header: %d\n", header->SizeOfOptionalHeader);
    printf("Characteristics: 0x%04x\n", header->Characteristics);
}