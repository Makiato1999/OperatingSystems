#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

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
    if (main_boot_sector->jump_boot[0] == 'E' &&
        main_boot_sector->jump_boot[1] == 'B' &&
        main_boot_sector->jump_boot[2] == 'h')
    {
        printf("Check Pass\n");
    }
    else
    {
        printf("Inconsistent file system: jump_boot must be 'EBh', value is '%s'.\n", main_boot_sector->jump_boot);
        exit(1);
    }
    if (main_boot_sector->jump_boot[0] == '7' &&
        main_boot_sector->jump_boot[1] == '6' &&
        main_boot_sector->jump_boot[2] == 'h')
    {
        printf("Check Pass\n");
    }
    else
    {
        printf("Inconsistent file system: jump_boot must be '76h', value is '%s'.\n", main_boot_sector->jump_boot);
        exit(1);
    }
    if (main_boot_sector->jump_boot[0] == '9' &&
        main_boot_sector->jump_boot[1] == '0' &&
        main_boot_sector->jump_boot[2] == 'h')
    {
        printf("Check Pass\n");
    }
    else
    {
        printf("Inconsistent file system: jump_boot must be '90h', value is '%s'.\n", main_boot_sector->jump_boot);
        exit(1);
    }

    // check FileSystemName
    char valid_fileSysName[8] = "EXFAT   ";
    int i = 0;
    for (i = 0; i < 8; i++)
    {
        if (main_boot_sector->fs_name[i] == valid_fileSysName[i])
        {
        }
        else
        {
            printf("Inconsistent file system: FileSystemName must be 'EXFAT   ', value is '%s'.\n", main_boot_sector->fs_name);
            exit(1);
        }
    }
    printf("Check Pass\n");

    // check MustBeZero
    for (i = 0; i < 53; i++)
    {
        if (main_boot_sector->must_be_zero[i] == 0)
        {
        }
        else
        {
            printf("Inconsistent file system: MustBeZero must be '0', value is '%s'.\n", main_boot_sector->must_be_zero);
            exit(1);
        }
    }
}
