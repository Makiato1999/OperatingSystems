#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>

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
#pragma pack(pop)

void read_main_boot_sector(main_boot_sector *main_boot_sector, int handle);
void parse_main_boot_sector(main_boot_sector *main_boot_sector);
void parse_bitmap(main_boot_sector *main_boot_sector, int handle);

int main(int argc, char *argv[])
{
    // exception
    if (argc < 2)
    {
        perror("Invalid arguments\n");
        exit(1);
    }
    // read file
    char *filename = argv[1];
    int fd = open(filename, O_RDONLY);

    // parse file
    main_boot_sector main_boot_sector;
    read_main_boot_sector(&main_boot_sector, fd);
    parse_main_boot_sector(&main_boot_sector);
    parse_bitmap(&main_boot_sector, fd);

    return EXIT_SUCCESS;
}

void read_main_boot_sector(main_boot_sector *main_boot_sector, int handle)
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

void parse_main_boot_sector(main_boot_sector *main_boot_sector)
{
    // check JumpBoot
    //printf("jump_boot: %x\n", main_boot_sector->jump_boot[0]);
    if (main_boot_sector->jump_boot[0] == (uint8_t)0xEB &&
        main_boot_sector->jump_boot[1] == (uint8_t)0x76 &&
        main_boot_sector->jump_boot[2] == (uint8_t)0x90)
    {
        printf("--->Check JumpBoot Pass\n");
    }
    else
    {
        printf("Inconsistent file system: jump_boot must be 'EBh 76h 90h', value is '%x %x %x'.\n",
               main_boot_sector->jump_boot[0], main_boot_sector->jump_boot[1], main_boot_sector->jump_boot[2]);
        exit(1);
    }

    // check FileSystemName
    //printf("FileSystemName: %s\n", main_boot_sector->fs_name);
    char valid_fileSysName[8] = "EXFAT   ";
    int i = 0;
    for (i = 0; i < 8; i++)
    {
        if (main_boot_sector->fs_name[i] != valid_fileSysName[i])
        {
            printf("Inconsistent file system: FileSystemName must be 'EXFAT   ', value is '%s'.\n",
                   main_boot_sector->fs_name);
            exit(1);
        }
    }
    printf("--->Check FileSystemName Pass\n");

    // check MustBeZero
    for (i = 0; i < 53; i++)
    {
        if (main_boot_sector->must_be_zero[i] != (uint8_t)0x0)
        {
            printf("Inconsistent file system: MustBeZero must be 0, value is %x.\n",
                   main_boot_sector->must_be_zero[i]);
            exit(1);
        }
    }
    printf("--->Check MustBeZero Pass\n");

    // check VolumeLength
    //printf("VolumeLength: %lu\n", main_boot_sector->volume_length);
    uint64_t base = 0x1;
    uint64_t valid_atleast = (base << 20) / (base << main_boot_sector->bytes_per_sector_shift);
    uint64_t valid_atmost = ((base << 8) << 8) - 1;
    if (main_boot_sector->volume_length >= valid_atleast &&
        main_boot_sector->volume_length <= valid_atmost)
    {
        printf("--->Check VolumeLength Pass\n");
    }
    else
    {
        printf("Inconsistent file system: VolumeLength must be >= %lu and <= %lu, value is %lu.\n",
               valid_atleast, valid_atmost, main_boot_sector->volume_length);
        exit(1);
    }

    // check FatOffset
    //printf("FatOffset: %u\n", main_boot_sector->fat_offset);
    uint32_t valid_atmost_fatOffset = main_boot_sector->cluster_heap_offset - (main_boot_sector->fat_length * main_boot_sector->number_of_fats);
    if (main_boot_sector->fat_offset >= 24 &&
        main_boot_sector->fat_offset <= valid_atmost_fatOffset)
    {
        printf("--->Check FatOffset Pass\n");
    }
    else
    {
        printf("Inconsistent file system: FatOffset must be >= 24 and <= %u, value is %u.\n",
               valid_atmost_fatOffset, main_boot_sector->fat_offset);
        exit(1);
    }

    // check FatLength
    //printf("FatLength: %u\n", main_boot_sector->fat_length);
    uint32_t base32 = 0x1;
    uint32_t valid_atleast_fatLength = ((main_boot_sector->cluster_count + 2) * (base32 << 2)) / (base32 << main_boot_sector->bytes_per_sector_shift);
    uint32_t roundup_atleast = (uint32_t)valid_atleast_fatLength;
    uint32_t valid_atmost_fatLength = (main_boot_sector->cluster_heap_offset - main_boot_sector->fat_offset) / main_boot_sector->number_of_fats;
    uint32_t rounddown_atmost = (uint32_t)valid_atmost_fatLength + 1;
    if (main_boot_sector->fat_length >= roundup_atleast &&
        main_boot_sector->fat_length <= rounddown_atmost)
    {
        printf("--->Check FatLength Pass\n");
    }
    else
    {
        printf("Inconsistent file system: FatLength must be >= %u and <= %u, value is %u.\n",
               roundup_atleast, rounddown_atmost, main_boot_sector->fat_length);
        exit(1);
    }

    // check FirstClusterOfRootDirectory
    //printf("FirstClusterOfRootDirectory: %u\n", main_boot_sector->first_cluster_of_root_directory);
    uint32_t valid_atleast_rootDirectory = 2;
    uint32_t valid_atmost_rootDirectory = main_boot_sector->cluster_count + 1;
    if (main_boot_sector->first_cluster_of_root_directory >= valid_atleast_rootDirectory &&
        main_boot_sector->first_cluster_of_root_directory <= valid_atmost_rootDirectory)
    {
        printf("--->Check FirstClusterOfRootDirectory Pass\n");
    }
    else
    {
        printf("Inconsistent file system: FirstClusterOfRootDirectory must be >= 2 and <= %u, value is %u.\n",
               valid_atmost_rootDirectory, main_boot_sector->first_cluster_of_root_directory);
        exit(1);
    }

    // check BootSignature
    //printf("BootSignature: %x\n", main_boot_sector->boot_signature);
    uint16_t valid_BootSignature = 0xAA55;
    if (main_boot_sector->boot_signature == valid_BootSignature)
    {
        printf("--->Check BootSignature Pass\n");
    }
    else
    {
        printf("Inconsistent file system: BootSignature must be 'AA55h', value is '%x'.\n",
               main_boot_sector->boot_signature);
        exit(1);
    }
    printf("MBR appears to be consistent.\nFile system appears to be consistent.\n");
}

void parse_bitmap(main_boot_sector *main_boot_sector, int handle)
{
    // jump to the cluster_heap beginning
    lseek(handle, main_boot_sector->cluster_heap_offset, SEEK_SET);
    // jump two fatlength, then new location is cluster[2]
    lseek(handle, (main_boot_sector->fat_length) * 2, SEEK_CUR);
    // jump (first_cluster_of_root_directory - 2) index cluster
    uint32_t newIndex = main_boot_sector->first_cluster_of_root_directory - 2;
    lseek(handle, newIndex * 32, SEEK_CUR);
}
