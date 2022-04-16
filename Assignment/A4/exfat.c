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

uint64_t sectorsPerCluster = 0;
uint64_t bytesPerSector = 0;
uint64_t bytesPerCluster = 0;

#pragma pack(pop)

void read_volume(main_boot_sector *main_boot_sector, int handle);
void process_info_command(main_boot_sector *main_boot_sector, int handle, volume_label *volume_label, allocation_bitmap *allocation_bitmap);
// void process_list_command(main_boot_sector *main_boot_sector, int handle, volume_label *volume_label, allocation_bitmap *allocation_bitmap);
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
    int fd = open(volumeName, O_RDONLY);
    main_boot_sector main_boot_sector;
    volume_label volume_label;
    allocation_bitmap allocation_bitmap;
    read_volume(&main_boot_sector, fd);
    // check command
    if (strcmp(commandName, "info") == 0)
    {
        printf("command: %s\n", commandName);
        process_info_command(&main_boot_sector, fd, &volume_label, &allocation_bitmap);
    }
    else if (strcmp(commandName, "list") == 0)
    {
        printf("command: %s\n", commandName);
        // process_list_command(&main_boot_sector, fd, &volume_label, &allocation_bitmap);
    }
    else if (strcmp(commandName, "get") == 0)
    {
        /* code */
    }
    return EXIT_SUCCESS;
}
//------------------------------------------------------
// myRoutine: read_volume
//
// PURPOSE: add all tasks to ready_queue
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
// myRoutine: process_info_command
//
// PURPOSE: add all tasks to ready_queue
// INPUT PARAMETERS:
//     main_boot_sector *main_boot_sector
//     int handle
//     volume_label *volume_label
//     allocation_bitmap *allocation_bitmap
//------------------------------------------------------
void process_info_command(main_boot_sector *main_boot_sector, int handle, volume_label *volume_label, allocation_bitmap *allocation_bitmap)
{
    assert(main_boot_sector != NULL);
    assert(handle >= 0);
    assert(volume_label != NULL);

    uint8_t temp_entryType;
    uint64_t i = 0;
    int used_bits_amount = 0;
    uint64_t bits_population = 0;
    // int unused_bits_amount = 0;
    //int freeSpace = 0;

    sectorsPerCluster = 1 << main_boot_sector->sectors_per_cluster_shift;
    bytesPerSector = 1 << main_boot_sector->bytes_per_sector_shift;
    bytesPerCluster = (1 << main_boot_sector->sectors_per_cluster_shift) * (1 << main_boot_sector->bytes_per_sector_shift);
    // Volume label
    // jump to cluster heap, then jump to first cluster
    lseek(handle, (main_boot_sector->cluster_heap_offset) * (bytesPerSector), SEEK_SET);
    lseek(handle, (main_boot_sector->first_cluster_of_root_directory - 2) * (sectorsPerCluster) * (bytesPerSector), SEEK_CUR);
    while (1)
    {
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
    char *temp = unicode2ascii(volume_label->VolumeLabel, 22);
    printf("- Volume label: %s\n", temp);

    // Volume serial number
    printf("- Volume serial number: %u\n", main_boot_sector->volume_serial_number);

    // Free space on the volume in KB
    // jump to cluster heap, then jump to first cluster
    lseek(handle, (main_boot_sector->cluster_heap_offset) * (bytesPerSector), SEEK_SET);
    lseek(handle, (main_boot_sector->first_cluster_of_root_directory - 2) * (sectorsPerCluster) * (bytesPerSector), SEEK_CUR);
    while (1)
    {
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
    printf("- DataLength: %lu\n", allocation_bitmap->DataLength);
    lseek(handle, (main_boot_sector->cluster_heap_offset) * (bytesPerSector), SEEK_SET);
    lseek(handle, (allocation_bitmap->FirstCluster - 2) * (sectorsPerCluster) * (bytesPerSector), SEEK_CUR);
    for (i = 0; i < allocation_bitmap->DataLength; i++)
    {
        uint8_t data;
        read(handle, &data, 1);
        used_bits_amount += __builtin_popcount(data);
    }
    printf("- bits_population: %lu\n", bits_population);
    printf("- Free space on the volume: %luKB\n", ((main_boot_sector->cluster_count - used_bits_amount) * bytesPerCluster) / 1024);

    // The cluster size, both in sectors and in bytes OR KB
    printf("- The cluster size is %lu sectors\n", sectorsPerCluster);
    printf("- The cluster size is %lu bytes\n", bytesPerCluster);
}
//------------------------------------------------------
// myRoutine: process_list_command
//
// PURPOSE: add all tasks to ready_queue
// INPUT PARAMETERS:
//     main_boot_sector *main_boot_sector
//     int handle
//     volume_label *volume_label
//     allocation_bitmap *allocation_bitmap
//------------------------------------------------------
/*
void process_list_command(main_boot_sector *main_boot_sector, int handle, volume_label *volume_label, allocation_bitmap *allocation_bitmap)
{

}*/
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
