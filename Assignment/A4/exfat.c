//-----------------------------------------
// NAME: Xiaoran Xie
// STUDENT NUMBER: 7884702
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Robert Guderian
// ASSIGNMENT: assignment 4
//
// REMARKS: file system
//
//-----------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

#pragma pack(1)
#pragma pack(push)
typedef struct MAIN_BOOT_SECTOR
{
    uint8_t jump_boot[3];
    char fs_name[8];
    uint8_t must_be_zero[53];
    uint64_t partition_offset;
    uint64_t volume_length;
    uint32_t fat_offset;
    uint32_t fat_length;
    uint32_t cluster_heap_offset;
    uint32_t cluster_count;
    uint32_t first_cluster_of_root_directory;
    uint32_t volume_serial_number;
    uint16_t fs_revision;
    uint16_t fs_flags;
    uint8_t bytes_per_sector_shift;
    uint8_t sectors_per_cluster_shift;
    uint8_t number_of_fats;
    uint8_t drive_select;
    uint8_t percent_in_use;
    uint8_t reserved[7];
    uint8_t bootcode[390];
    uint16_t boot_signature;
} main_boot_sector;

typedef struct VOLUME_LABEL
{
    uint8_t EntryType;
    uint8_t CharacterCount;
    uint16_t VolumeLabel[22];
    uint64_t Reserved;
} volume_label;

typedef struct ALLOCATION_BITMAP
{
    uint8_t EntryType;
    uint8_t BitmapFlags;
    char Reserved[18];
    uint32_t FirstCluster;
    uint64_t DataLength;
} allocation_bitmap;

typedef struct FILE
{
    uint8_t EntryType;
    uint8_t SecondaryCount;
    uint16_t SetChecksum;
    uint16_t FileAttributes;
    uint16_t Reserved1;
    uint32_t CreateTimestamp;
    uint32_t LastModifiedTimestamp;
    uint32_t LastAccessedTimestamp;
    uint8_t Create10msIncrement;
    uint8_t LastModified10msIncrement;
    uint8_t CreateUtcOffset;
    uint8_t LastModifiedUtcOffset;
    uint8_t LastAccessedUtcOffset;
    char Reserved2[7];
} file;

typedef struct STREAM_EXTENSION
{
    uint8_t EntryType;
    char GeneralSecondaryFlags[8];
    uint8_t Reserved1;
    // The valid number of File Name directory entries in a File directory entry set is NameLength / 15,
    // rounded up to the nearest integer.
    uint8_t NameLength;
    uint16_t NameHash;
    uint16_t Reserved2;
    uint64_t ValidDataLength;
    uint32_t Reserved3;
    uint32_t FirstCluster;
    uint64_t DataLength;
} stream_extension;

typedef struct FILE_NAME
{
    uint8_t EntryType;
    uint8_t GeneralSecondaryFlags;
    uint16_t FileName[30];
} file_name;

int fd = 0;
uint64_t sectorsPerCluster = 0;
uint64_t bytesPerSector = 0;
uint64_t bytesPerCluster = 0;
uint64_t clusterSize = 0;

#pragma pack(pop)

void read_volume(main_boot_sector *main_boot_sector, int handle);
void prepare_info_command(main_boot_sector *main_boot_sector, int handle, volume_label *volume_label, allocation_bitmap *allocation_bitmap);
void print_info_command(main_boot_sector *main_boot_sector, int handle, volume_label *volume_label, allocation_bitmap *allocation_bitmap);
void prepare_list_command(main_boot_sector *main_boot_sector, int handle, file *file, stream_extension *stream_extension, file_name *file_name);
void parse_list_command(uint64_t layerOfRecursion, uint32_t isFile, uint64_t root, main_boot_sector *main_boot_sector, int handle, file *file, stream_extension *stream_extension, file_name *file_name);
void prepare_get_command(char *path, main_boot_sector *main_boot_sector, int handle, file *file, stream_extension *stream_extension, file_name *file_name);
void parse_get_command(char *pathway[], uint64_t layerOfRecursion, uint32_t isFile, uint64_t root, main_boot_sector *main_boot_sector, int handle, file *file, stream_extension *stream_extension, file_name *file_name);
static char *unicode2ascii(uint16_t *unicode_string, uint8_t length);

int main(int argc, char *argv[])
{
    // exception
    if (argc < 3)
    {
        perror("Invalid arguments\n");
        exit(1);
    }
    // get parameter
    char *volumeName = argv[1];
    char *commandName = argv[2];
    // read volume
    fd = open(volumeName, O_RDONLY);
    main_boot_sector main_boot_sector;
    volume_label volume_label;
    allocation_bitmap allocation_bitmap;
    file file;
    stream_extension stream_extension;
    file_name file_name;
    read_volume(&main_boot_sector, fd);
    // check command
    if (strcmp(commandName, "info") == 0)
    {
        printf("command: %s\n", commandName);
        prepare_info_command(&main_boot_sector, fd, &volume_label, &allocation_bitmap);
        print_info_command(&main_boot_sector, fd, &volume_label, &allocation_bitmap);
    }
    else if (strcmp(commandName, "list") == 0)
    {
        printf("command: %s\n", commandName);
        prepare_list_command(&main_boot_sector, fd, &file, &stream_extension, &file_name);
    }
    else if (strcmp(commandName, "get") == 0)
    {
        printf("command: %s\n", commandName);
        char *path = argv[3];
        prepare_get_command(path, &main_boot_sector, fd, &file, &stream_extension, &file_name);
    }
    return EXIT_SUCCESS;
}
//------------------------------------------------------
// myRoutine: read_volume
//
// PURPOSE: read and save main_boot_sector
// INPUT PARAMETERS:
//     main_boot_sector *main_boot_sector
//     int handle
//------------------------------------------------------
void read_volume(main_boot_sector *main_boot_sector, int handle)
{
    assert(main_boot_sector != NULL);
    assert(handle >= 0);

    // JumpBoot 3
    read(handle, &main_boot_sector->jump_boot, 3);
    // FileSystemName 8
    read(handle, &main_boot_sector->fs_name, 8);
    // MustBeZero 53
    read(handle, &main_boot_sector->must_be_zero, 53);
    // PartitionOffset 8
    read(handle, &main_boot_sector->partition_offset, 8);
    // VolumeLength 8
    read(handle, &main_boot_sector->volume_length, 8);
    // FatOffset 4
    read(handle, &main_boot_sector->fat_offset, 4);
    // FatLength 4
    read(handle, &main_boot_sector->fat_length, 4);
    // cluster_heap_offset 4
    read(handle, &main_boot_sector->cluster_heap_offset, 4);
    // ClusterCount 4
    read(handle, &main_boot_sector->cluster_count, 4);
    // FirstClusterOfRootDirectory 4
    read(handle, &main_boot_sector->first_cluster_of_root_directory, 4);
    // VolumeSerialNumber 4
    read(handle, &main_boot_sector->volume_serial_number, 4);
    // FileSystemRevision 2
    read(handle, &main_boot_sector->fs_revision, 2);
    // VolumeFlags 2
    read(handle, &main_boot_sector->fs_flags, 2);
    // BytesPerSectorShift 1
    read(handle, &main_boot_sector->bytes_per_sector_shift, 1);
    // SectorsPerClusterShift 1
    read(handle, &main_boot_sector->sectors_per_cluster_shift, 1);
    // NumberOfFats 1
    read(handle, &main_boot_sector->number_of_fats, 1);
    // DriveSelect 1
    read(handle, &main_boot_sector->drive_select, 1);
    // PercentInUse 1
    read(handle, &main_boot_sector->percent_in_use, 1);
    // Reserved 7
    read(handle, &main_boot_sector->reserved, 7);
    // BootCode 390
    read(handle, &main_boot_sector->bootcode, 390);
    // BootSignature 2
    read(handle, &main_boot_sector->boot_signature, 2);
}
//------------------------------------------------------
// myRoutine: prepare_info_command
//
// PURPOSE: read and save volume_label and allocation_bitmap
// INPUT PARAMETERS:
//     main_boot_sector *main_boot_sector
//     int handle
//     volume_label *volume_label
//     allocation_bitmap *allocation_bitmap
//------------------------------------------------------
void prepare_info_command(main_boot_sector *main_boot_sector, int handle, volume_label *volume_label, allocation_bitmap *allocation_bitmap)
{
    assert(main_boot_sector != NULL);
    assert(handle >= 0);
    assert(volume_label != NULL);

    sectorsPerCluster = 1 << main_boot_sector->sectors_per_cluster_shift;
    bytesPerSector = 1 << main_boot_sector->bytes_per_sector_shift;
    bytesPerCluster = (1 << main_boot_sector->sectors_per_cluster_shift) * (1 << main_boot_sector->bytes_per_sector_shift);
    // Volume label
    // jump to cluster heap, then jump to first cluster
    lseek(handle, (main_boot_sector->cluster_heap_offset) * (bytesPerSector), SEEK_SET);
    lseek(handle, (main_boot_sector->first_cluster_of_root_directory - 2) * (sectorsPerCluster) * (bytesPerSector), SEEK_CUR);
    while (1)
    {
        uint8_t temp_entryType;
        read(handle, &temp_entryType, 1);
        if (temp_entryType == 0x83)
        {
            lseek(handle, -1, SEEK_CUR);
            read(handle, &volume_label->EntryType, 1);
            read(handle, &volume_label->CharacterCount, 1);
            read(handle, &volume_label->VolumeLabel, 22);
            read(handle, &volume_label->Reserved, 8);
            break;
        }
        else
        {
            lseek(handle, 31, SEEK_CUR);
        }
    }

    // Free space on the volume in KB
    // jump to cluster heap, then jump to first cluster
    lseek(handle, (main_boot_sector->cluster_heap_offset) * (bytesPerSector), SEEK_SET);
    lseek(handle, (main_boot_sector->first_cluster_of_root_directory - 2) * (sectorsPerCluster) * (bytesPerSector), SEEK_CUR);
    while (1)
    {
        uint8_t temp_entryType;
        read(handle, &temp_entryType, 1);
        if (temp_entryType == 0x81)
        {
            lseek(handle, -1, SEEK_CUR);
            read(handle, &allocation_bitmap->EntryType, 1);
            read(handle, &allocation_bitmap->BitmapFlags, 1);
            read(handle, &allocation_bitmap->Reserved, 18);
            read(handle, &allocation_bitmap->FirstCluster, 4);
            read(handle, &allocation_bitmap->DataLength, 8);
            break;
        }
        else
        {
            lseek(handle, 31, SEEK_CUR);
        }
    }
}
//------------------------------------------------------
// myRoutine: print_info_command
//
// PURPOSE: print info
// INPUT PARAMETERS:
//     main_boot_sector *main_boot_sector
//     int handle
//     volume_label *volume_label
//     allocation_bitmap *allocation_bitmap
//------------------------------------------------------
void print_info_command(main_boot_sector *main_boot_sector, int handle, volume_label *volume_label, allocation_bitmap *allocation_bitmap)
{
    assert(main_boot_sector != NULL);
    assert(handle >= 0);
    assert(volume_label != NULL);

    uint64_t i = 0;
    int used_bitmap_cells_ap_amount = 0;
    uint64_t bitmap_cells_population = 0;
    int unused_bitmap_cells_amount = 0;
    uint64_t freeSpace = 0;

    // Volume label
    char *temp = unicode2ascii(volume_label->VolumeLabel, 22);
    printf("- Volume label: %s\n", temp);
    // Volume serial number
    printf("- Volume serial number: %u\n", main_boot_sector->volume_serial_number);
    // Free space
    lseek(handle, (main_boot_sector->cluster_heap_offset) * (bytesPerSector), SEEK_SET);
    lseek(handle, (allocation_bitmap->FirstCluster - 2) * (sectorsPerCluster) * (bytesPerSector), SEEK_CUR);
    for (i = 0; i < allocation_bitmap->DataLength; i++)
    {
        uint8_t data;
        read(handle, &data, 1);
        used_bitmap_cells_ap_amount += __builtin_popcount(data);
    } // total have cluster_count bitmap cells
    bitmap_cells_population = main_boot_sector->cluster_count;
    unused_bitmap_cells_amount = bitmap_cells_population - used_bitmap_cells_ap_amount;
    freeSpace = unused_bitmap_cells_amount * bytesPerCluster / 1024;
    printf("- Free space on the volume: %luKB\n", freeSpace);
    // The cluster size, both in sectors and in bytes OR KB
    printf("- The cluster size is %lu sectors\n", sectorsPerCluster);
    printf("- The cluster size is %lu bytes\n", bytesPerCluster);
}
//------------------------------------------------------
// myRoutine: prepare_list_command
//
// PURPOSE: read and save file, stream_extension, file_name
// INPUT PARAMETERS:
//     main_boot_sector *main_boot_sector
//     int handle
//     file *file
//     stream_extension *stream_extension
//     file_name *file_name
//------------------------------------------------------
void prepare_list_command(main_boot_sector *main_boot_sector, int handle, file *file, stream_extension *stream_extension, file_name *file_name)
{
    assert(main_boot_sector != NULL);
    assert(handle >= 0);
    assert(file != NULL);
    assert(stream_extension != NULL);
    assert(file_name != NULL);

    sectorsPerCluster = 1 << main_boot_sector->sectors_per_cluster_shift;
    bytesPerSector = 1 << main_boot_sector->bytes_per_sector_shift;
    bytesPerCluster = (1 << main_boot_sector->sectors_per_cluster_shift) * (1 << main_boot_sector->bytes_per_sector_shift);
    clusterSize = bytesPerSector * sectorsPerCluster;
    // printf("numOfFAT: %d\n", main_boot_sector->number_of_fats);
    // printf("clusterSizeByBytes: %lu\n", clusterSize);

    parse_list_command(0, 9999, main_boot_sector->first_cluster_of_root_directory, main_boot_sector, handle, file, stream_extension, file_name);
}
//------------------------------------------------------
// myRoutine: parse_list_command
//
// PURPOSE: parse entries: file, stream_extension, file_name
// INPUT PARAMETERS:
//     uint64_t layerOfRecursion
//     uint32_t isFile
//     uint64_t root
//     main_boot_sector *main_boot_sector
//     int handle
//     file *file
//     stream_extension *stream_extension
//     file_name *file_name
//------------------------------------------------------
void parse_list_command(uint64_t layerOfRecursion, uint32_t isFile, uint64_t root, main_boot_sector *main_boot_sector, int handle, file *file, stream_extension *stream_extension, file_name *file_name)
{
    uint64_t fileEnrtyDone = 0;
    uint64_t streamExtensionEntryDone = 0;
    uint64_t fileNameEnrtyDone = 0;
    uint64_t FATvalue = 0xffffffff;
    uint16_t fileNameString[1024];
    uint16_t currFileNameIndex = 0;
    uint16_t fileNameLoopFlag = 0;
    uint16_t i = 0;
    do
    {
        if (FATvalue != 0 && FATvalue != 0xffffffff)
        {
            root = FATvalue;
        }
        // printf("root: %lu\n", root);
        lseek(handle, main_boot_sector->fat_offset * bytesPerSector, SEEK_SET);
        lseek(handle, root * 4, SEEK_CUR);
        read(handle, &FATvalue, 4);
        // printf("FAT[%lu]: %lu\n", root, FATvalue);
        if (isFile == 1)
        {
            // printf("          file return recusion\n");
            return;
        }

        lseek(handle, (main_boot_sector->cluster_heap_offset) * (bytesPerSector), SEEK_SET);
        lseek(handle, (root - 2) * (sectorsPerCluster) * (bytesPerSector), SEEK_CUR);

        uint64_t entryCounter = 0;
        // char spaceInRecursion[1024];
        while (entryCounter < clusterSize / 32)
        {
            uint8_t temp_entryType = 0;
            read(handle, &temp_entryType, 1);
            // printf("temp_entryType: %x\n", temp_entryType);
            if (temp_entryType == 0x85 && fileNameLoopFlag == 0)
            {
                assert(temp_entryType == 0x85);
                lseek(handle, -1, SEEK_CUR);
                // file entry
                read(handle, &file->EntryType, 1);
                read(handle, &file->SecondaryCount, 1);
                read(handle, &file->SetChecksum, 2);
                read(handle, &file->FileAttributes, 2);
                read(handle, &file->Reserved1, 2);
                read(handle, &file->CreateTimestamp, 4);
                read(handle, &file->LastModifiedTimestamp, 4);
                read(handle, &file->LastAccessedTimestamp, 4);
                read(handle, &file->Create10msIncrement, 1);
                read(handle, &file->LastModified10msIncrement, 1);
                read(handle, &file->CreateUtcOffset, 1);
                read(handle, &file->LastModifiedUtcOffset, 1);
                read(handle, &file->LastAccessedUtcOffset, 1);
                read(handle, &file->Reserved2, 7);
                entryCounter += 1;
                fileEnrtyDone = 1;
                // printf("|     file entryCounter: %lu\n", entryCounter);
            }
            else if (temp_entryType == 0xc0 && fileEnrtyDone == 1 && fileNameLoopFlag == 0)
            {
                assert(temp_entryType == 0xc0);
                lseek(handle, -1, SEEK_CUR);
                // stream extension entry
                read(handle, &stream_extension->EntryType, 1);
                read(handle, &stream_extension->GeneralSecondaryFlags, 1);
                read(handle, &stream_extension->Reserved1, 1);
                read(handle, &stream_extension->NameLength, 1);
                read(handle, &stream_extension->NameHash, 2);
                read(handle, &stream_extension->Reserved2, 2);
                read(handle, &stream_extension->ValidDataLength, 8);
                read(handle, &stream_extension->Reserved3, 4);
                read(handle, &stream_extension->FirstCluster, 4);
                read(handle, &stream_extension->DataLength, 8);
                entryCounter += 1;
                streamExtensionEntryDone = 1;
                // printf("|     stream extension entryCounter: %lu\n", entryCounter);
            }
            else if (temp_entryType == 0xc1 && fileEnrtyDone == 1 && streamExtensionEntryDone == 1)
            {
                lseek(handle, -1, SEEK_CUR);
                // printf("file->SecondaryCount: %d\n", file->SecondaryCount);
                if (i < file->SecondaryCount - 1)
                {
                    // file entry
                    read(handle, &file_name->EntryType, 1);
                    read(handle, &file_name->GeneralSecondaryFlags, 1);
                    read(handle, &file_name->FileName, 30);
                    // file entry
                    uint16_t j = 0;
                    for (j = 0; j < 15; j++)
                    {
                        fileNameString[currFileNameIndex] = file_name->FileName[j];
                        currFileNameIndex += 1;
                    }
                    entryCounter += 1;
                    // printf("|     file name entryCounter: %lu\n", entryCounter);
                    i += 1;
                    fileNameEnrtyDone = 0;
                    fileNameLoopFlag = 1;
                }
                if (i == file->SecondaryCount - 1)
                {
                    fileNameEnrtyDone = 1;
                    currFileNameIndex = 0;
                    i = 0;
                    fileNameLoopFlag = 0;
                }
            }
            else
            {
                lseek(handle, 31, SEEK_CUR);
                fileEnrtyDone = 0;
                streamExtensionEntryDone = 0;
                fileNameEnrtyDone = 0;
                entryCounter += 1;
                // printf("|     none entryCounter: %lu\n", entryCounter);
            }
            if (fileEnrtyDone == 1 && streamExtensionEntryDone == 1 && fileNameEnrtyDone == 1)
            {
                uint64_t j = 0;
                for (j = 0; j < layerOfRecursion; j++)
                {
                    printf("-");
                }
                layerOfRecursion += 1;
                // printf("first cluster: %u\n", stream_extension->FirstCluster);
                printf("%s\n", unicode2ascii(fileNameString, stream_extension->NameLength));
                if ((file->FileAttributes & (1 << 4)) >> 4 == 0)
                {
                    assert((file->FileAttributes & (1 << 4)) >> 4 == 0);
                    // file is 0
                    isFile = 1;
                    // printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> into recusion\n");
                    assert(isFile == 1);
                    parse_list_command(layerOfRecursion, isFile, stream_extension->FirstCluster, main_boot_sector, handle, file, stream_extension, file_name);
                    // printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< quit recusion\n");
                    // printf("*** root: %lu\n", root);
                    //  printf("*** FAT[%lu]: %lu\n", root, FATvalue);
                }
                else
                {
                    assert((file->FileAttributes & (1 << 4)) >> 4 == 1);
                    // directory is 1
                    isFile = 0;
                    // printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> into recusion\n");
                    assert(isFile == 0);
                    parse_list_command(layerOfRecursion, isFile, stream_extension->FirstCluster, main_boot_sector, handle, file, stream_extension, file_name);
                    // printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< quit recusion\n");
                    // printf("*** root: %lu\n", root);
                    //  printf("*** FAT[%lu]: %lu\n", root, FATvalue);
                }
                layerOfRecursion -= 1;
                lseek(handle, (main_boot_sector->cluster_heap_offset) * (bytesPerSector), SEEK_SET);
                lseek(handle, (root - 2) * (sectorsPerCluster) * (bytesPerSector), SEEK_CUR);
                lseek(handle, entryCounter * 32, SEEK_CUR);
                isFile = 0;
                fileEnrtyDone = 0;
                streamExtensionEntryDone = 0;
                fileNameEnrtyDone = 0;
            }
        }
        entryCounter = 0;
    } while (FATvalue != 0 && FATvalue != 0xFFFFFFFF);

    // printf("          directory return recusion\n");
    return;
}
//------------------------------------------------------
// myRoutine: prepare_get_command
//
// PURPOSE: read and save file, stream_extension, file_name
// INPUT PARAMETERS:
//     char *path
//     main_boot_sector *main_boot_sector
//     int handle
//     file *file
//     stream_extension *stream_extension
//     file_name *file_name
//------------------------------------------------------
void prepare_get_command(char *path, main_boot_sector *main_boot_sector, int handle, file *file, stream_extension *stream_extension, file_name *file_name)
{
    assert(path != NULL);
    assert(main_boot_sector != NULL);
    assert(handle >= 0);
    assert(file != NULL);
    assert(stream_extension != NULL);
    assert(file_name != NULL);

    sectorsPerCluster = 1 << main_boot_sector->sectors_per_cluster_shift;
    bytesPerSector = 1 << main_boot_sector->bytes_per_sector_shift;
    bytesPerCluster = (1 << main_boot_sector->sectors_per_cluster_shift) * (1 << main_boot_sector->bytes_per_sector_shift);
    clusterSize = bytesPerSector * sectorsPerCluster;

    char *pathway[1024];
    char *p;
    int counter = 0;
    p = strtok(path, "/");
    pathway[counter] = p;
    counter += 1;
    while ((p = strtok(NULL, "/")) != NULL)
    {
        pathway[counter] = p;
        counter += 1;
    }
    int i = 0;
    for (i = 0; i < counter; i++)
    {
        // printf("[%s]\n", pathway[i]);
    }

    parse_get_command(pathway, 0, 9999, main_boot_sector->first_cluster_of_root_directory, main_boot_sector, handle, file, stream_extension, file_name);
}
//------------------------------------------------------
// myRoutine: parse_get_command
//
// PURPOSE: parse entries: file, stream_extension, file_name
// INPUT PARAMETERS:
//     char *pathway[]
//     uint64_t layerOfRecursion
//     uint32_t isFile
//     uint64_t root
//     main_boot_sector *main_boot_sector
//     int handle
//     file *file
//     stream_extension *stream_extension
//     file_name *file_name
//------------------------------------------------------
void parse_get_command(char *pathway[], uint64_t layerOfRecursion, uint32_t isFile, uint64_t root, main_boot_sector *main_boot_sector, int handle, file *file, stream_extension *stream_extension, file_name *file_name)
{
    uint64_t fileEnrtyDone = 0;
    uint64_t streamExtensionEntryDone = 0;
    uint64_t fileNameEnrtyDone = 0;
    uint64_t FATvalue = 0xffffffff;
    uint16_t fileNameString[1024];
    uint16_t currFileNameIndex = 0;
    uint16_t fileNameLoopFlag = 0;
    uint16_t i = 0;
    do
    {
        if (FATvalue != 0 && FATvalue != 0xffffffff)
        {
            root = FATvalue;
        }
        // printf("root: %lu\n", root);
        lseek(handle, main_boot_sector->fat_offset * bytesPerSector, SEEK_SET);
        lseek(handle, root * 4, SEEK_CUR);
        read(handle, &FATvalue, 4);
        // printf("FAT[%lu]: %lu\n", root, FATvalue);
        if (isFile == 1)
        {
            // printf("          file return recusion\n");
            return;
        }

        lseek(handle, (main_boot_sector->cluster_heap_offset) * (bytesPerSector), SEEK_SET);
        lseek(handle, (root - 2) * (sectorsPerCluster) * (bytesPerSector), SEEK_CUR);

        uint64_t entryCounter = 0;
        // char spaceInRecursion[1024];
        while (entryCounter < clusterSize / 32)
        {
            uint8_t temp_entryType = 0;
            read(handle, &temp_entryType, 1);
            // printf("temp_entryType: %x\n", temp_entryType);
            if (temp_entryType == 0x85 && fileNameLoopFlag == 0)
            {
                assert(temp_entryType == 0x85);
                lseek(handle, -1, SEEK_CUR);
                // file entry
                read(handle, &file->EntryType, 1);
                read(handle, &file->SecondaryCount, 1);
                read(handle, &file->SetChecksum, 2);
                read(handle, &file->FileAttributes, 2);
                read(handle, &file->Reserved1, 2);
                read(handle, &file->CreateTimestamp, 4);
                read(handle, &file->LastModifiedTimestamp, 4);
                read(handle, &file->LastAccessedTimestamp, 4);
                read(handle, &file->Create10msIncrement, 1);
                read(handle, &file->LastModified10msIncrement, 1);
                read(handle, &file->CreateUtcOffset, 1);
                read(handle, &file->LastModifiedUtcOffset, 1);
                read(handle, &file->LastAccessedUtcOffset, 1);
                read(handle, &file->Reserved2, 7);
                entryCounter += 1;
                fileEnrtyDone = 1;
                // printf("|     file entryCounter: %lu\n", entryCounter);
            }
            else if (temp_entryType == 0xc0 && fileEnrtyDone == 1 && fileNameLoopFlag == 0)
            {
                assert(temp_entryType == 0xc0);
                lseek(handle, -1, SEEK_CUR);
                // stream extension entry
                read(handle, &stream_extension->EntryType, 1);
                read(handle, &stream_extension->GeneralSecondaryFlags, 1);
                read(handle, &stream_extension->Reserved1, 1);
                read(handle, &stream_extension->NameLength, 1);
                read(handle, &stream_extension->NameHash, 2);
                read(handle, &stream_extension->Reserved2, 2);
                read(handle, &stream_extension->ValidDataLength, 8);
                read(handle, &stream_extension->Reserved3, 4);
                read(handle, &stream_extension->FirstCluster, 4);
                read(handle, &stream_extension->DataLength, 8);
                entryCounter += 1;
                streamExtensionEntryDone = 1;
                // printf("|     stream extension entryCounter: %lu\n", entryCounter);
            }
            else if (temp_entryType == 0xc1 && fileEnrtyDone == 1 && streamExtensionEntryDone == 1)
            {
                lseek(handle, -1, SEEK_CUR);
                // printf("file->SecondaryCount: %d\n", file->SecondaryCount);
                if (i < file->SecondaryCount - 1)
                {
                    // file entry
                    read(handle, &file_name->EntryType, 1);
                    read(handle, &file_name->GeneralSecondaryFlags, 1);
                    read(handle, &file_name->FileName, 30);
                    // file entry
                    uint16_t j = 0;
                    for (j = 0; j < 15; j++)
                    {
                        fileNameString[currFileNameIndex] = file_name->FileName[j];
                        currFileNameIndex += 1;
                    }
                    entryCounter += 1;
                    // printf("|     file name entryCounter: %lu\n", entryCounter);
                    i += 1;
                    fileNameEnrtyDone = 0;
                    fileNameLoopFlag = 1;
                }
                if (i == file->SecondaryCount - 1)
                {
                    fileNameEnrtyDone = 1;
                    currFileNameIndex = 0;
                    i = 0;
                    fileNameLoopFlag = 0;
                }
            }
            else
            {
                lseek(handle, 31, SEEK_CUR);
                fileEnrtyDone = 0;
                streamExtensionEntryDone = 0;
                fileNameEnrtyDone = 0;
                entryCounter += 1;
                // printf("|     none entryCounter: %lu\n", entryCounter);
            }
            if (fileEnrtyDone == 1 && streamExtensionEntryDone == 1 && fileNameEnrtyDone == 1)
            {
                uint64_t j = 0;
                for (j = 0; j < layerOfRecursion; j++)
                {
                    // printf("-");
                }
                // printf("first cluster: %u\n", stream_extension->FirstCluster);
                // printf("%s\n", unicode2ascii(fileNameString, stream_extension->NameLength));
                if (strcmp(unicode2ascii(fileNameString, stream_extension->NameLength), pathway[layerOfRecursion]) == 0)
                {
                    char tempCombo[1024];
                    uint64_t k = 0;
                    strcat(tempCombo, pathway[k]);
                    for (k = 1; k < layerOfRecursion; k++)
                    {
                        strcat(tempCombo, "/");
                        strcat(tempCombo, pathway[k]);
                    }
                    mkdir(tempCombo, 0777);
                }
                layerOfRecursion += 1;
                if ((file->FileAttributes & (1 << 4)) >> 4 == 0)
                {
                    assert((file->FileAttributes & (1 << 4)) >> 4 == 0);
                    // file is 0
                    isFile = 1;
                    // printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> into recusion\n");
                    assert(isFile == 1);
                    parse_get_command(pathway, layerOfRecursion, isFile, stream_extension->FirstCluster, main_boot_sector, handle, file, stream_extension, file_name);
                    // printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< quit recusion\n");
                    // printf("*** root: %lu\n", root);
                    //  printf("*** FAT[%lu]: %lu\n", root, FATvalue);
                }
                else
                {
                    assert((file->FileAttributes & (1 << 4)) >> 4 == 1);
                    // directory is 1
                    isFile = 0;
                    // printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> into recusion\n");
                    assert(isFile == 0);
                    parse_get_command(pathway, layerOfRecursion, isFile, stream_extension->FirstCluster, main_boot_sector, handle, file, stream_extension, file_name);
                    // printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< quit recusion\n");
                    // printf("*** root: %lu\n", root);
                    //  printf("*** FAT[%lu]: %lu\n", root, FATvalue);
                }
                layerOfRecursion -= 1;
                lseek(handle, (main_boot_sector->cluster_heap_offset) * (bytesPerSector), SEEK_SET);
                lseek(handle, (root - 2) * (sectorsPerCluster) * (bytesPerSector), SEEK_CUR);
                lseek(handle, entryCounter * 32, SEEK_CUR);
                isFile = 0;
                fileEnrtyDone = 0;
                streamExtensionEntryDone = 0;
                fileNameEnrtyDone = 0;
            }
        }
        entryCounter = 0;
    } while (FATvalue != 0 && FATvalue != 0xFFFFFFFF);

    // printf("          directory return recusion\n");
    return;
}
/**
 * Convert a Unicode-formatted string containing only ASCII characters
 * into a regular ASCII-formatted string (16 bit chars to 8 bit
 * chars).
 *
 * NOTE: this function does a heap allocation for the string it
 *       returns, caller is responsible for `free`-ing the allocation
 *       when necessary.
 *
 * uint16_t *unicode_string: the Unicode-formatted string to be
 *                           converted.
 * uint8_t   length: the length of the Unicode-formatted string (in
 *                   characters).
 *
 * returns: a heap allocated ASCII-formatted string.
 */
static char *unicode2ascii(uint16_t *unicode_string, uint8_t length)
{
    assert(unicode_string != NULL);
    assert(length > 0);

    char *ascii_string = NULL;

    if (unicode_string != NULL && length > 0)
    {
        // +1 for a NULL terminator
        ascii_string = calloc(sizeof(char), length + 1);

        if (ascii_string)
        {
            // strip the top 8 bits from every character in the
            // unicode string
            for (uint8_t i = 0; i < length; i++)
            {
                ascii_string[i] = (char)unicode_string[i];
            }
            // stick a null terminator at the end of the string.
            ascii_string[length] = '\0';
        }
    }

    return ascii_string;
}
