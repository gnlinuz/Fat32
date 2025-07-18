#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*  
    Created 28/Jun/2025
    Author: George Nikolaidis. 
    Reading the root Dir of a FAT32 file system.
    sudo ./lsdir /dev/sdc1 
*/

// Define structures to match FAT32 BPB (BIOS Parameter Block)
// The use of __attribute__((packed)) in the typedef struct
// is specifically to prevent the compiler from inserting padding bytes between structure members, ensuring the structure's layout in memory exactly matches 
//  the layout defined by the FAT32 boot sector specification.
// Without __attribute__((packed)), the structure may have unexpected padding, which would make reading a FAT32 boot sector from a raw disk inaccurate — you'd end up with incorrect field values.
// The same if you use at the top #pragma pack(push, 1) and at the end #pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint8_t  jumpBoot[3];
    uint8_t  OEMName[8];
    uint16_t bytesPerSector;
    uint8_t  sectorsPerCluster;
    uint16_t reservedSectorCount;
    uint8_t  numFATs;
    uint16_t rootEntryCount;
    uint16_t totalSectors16;
    uint8_t  media;
    uint16_t FATSize16;
    uint16_t sectorsPerTrack;
    uint16_t numberOfHeads;
    uint32_t hiddenSectors;
    uint32_t totalSectors32;

    uint32_t FATSize32;
    uint16_t extFlags;
    uint16_t FSVersion;
    uint32_t rootCluster;
    uint16_t FSInfo;
    uint16_t backupBootSector;
    uint8_t  reserved[12];

    uint8_t  driveNumber;
    uint8_t  reserved1;
    uint8_t  bootSignature;
    uint32_t volumeID;
    uint8_t  volumeLabel[11];
    uint8_t  fileSystemType[8];
} FAT32_BootSector;

typedef struct {
    uint8_t  DIR_Name[11];
    uint8_t  DIR_Attr;
    uint8_t  DIR_NTRes;
    uint8_t  DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
} DirectoryEntry;

typedef struct {
    uint8_t  LDIR_Ord;
    uint8_t  LDIR_Name1[10];
    uint8_t  LDIR_Attr;
    uint8_t  LDIR_Type;         // If zero, indicates a directory entry that is a sub-component of a long name.
    uint8_t  LDIR_Chksum;
    uint8_t  LDIR_Name2[12];
    uint16_t LDIR_FstClusLO;
    uint8_t  LDIR_Name3[4];
} LongDirectoryEntry;

typedef struct {
    char currentPath[256];     // Fixed size
    char deviceName[64];       // Fixed size
    uint32_t currentCluster;   // 4 bytes
} FAT32_CWDInfo;

#pragma pack(pop)

#define ATTR_LONG_NAME       0x0F   // Used to identify long name entries (0x3F AND 0x0F) = 0x0F
#define ATTR_LONG_NAME_MASK  0x3F   // Mask for detecting long name 0x3F, 0001 1111
#define ATTR_DIRECTORY       0x10   
#define ATTR_VOLUME_ID       0x08


int read_state(FAT32_CWDInfo *info) {
    FILE *fp = fopen(".fat32_cwd", "rb");
    if (!fp) {
        perror("Failed to open file for reading");
        return 1;
    }

    if (fread(info, sizeof(FAT32_CWDInfo), 1, fp) != 1) {
        perror("Failed to read data");
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}


void readBootSector(FILE *disk, FAT32_BootSector *bootSector) {
    fseek(disk, 0, SEEK_SET);
    fread(bootSector, sizeof(FAT32_BootSector), 1, disk);
}

uint32_t getNextCluster(FILE *disk, const FAT32_BootSector *bs, uint32_t currentCluster) {
    uint32_t FATOffset = currentCluster * 4;
    uint32_t sector = bs->reservedSectorCount + (FATOffset / bs->bytesPerSector);
    uint32_t offset = FATOffset % bs->bytesPerSector;
    uint32_t nextCluster;

    fseek(disk, sector * bs->bytesPerSector + offset, SEEK_SET);
    fread(&nextCluster, 4, 1, disk);

    return nextCluster & 0x0FFFFFFF;
}

void readCluster(FILE *disk, const FAT32_BootSector *bs, uint32_t cluster, uint8_t *buffer) {
    uint32_t firstDataSector = bs->reservedSectorCount + (bs->numFATs * bs->FATSize32);
    uint32_t firstSector = firstDataSector + (cluster - 2) * bs->sectorsPerCluster;
    fseek(disk, firstSector * bs->bytesPerSector, SEEK_SET);
    fread(buffer, bs->sectorsPerCluster * bs->bytesPerSector, 1, disk); // reads the first data cluster ex: 4096 bytes
}



void listRootDirectory(FILE *disk, const FAT32_BootSector *bs, FAT32_CWDInfo *info) {
    uint8_t buffer[bs->sectorsPerCluster * bs->bytesPerSector];
    uint32_t cluster = info->currentCluster;        // read the config file for the pos of the current cluster
    // uint32_t cluster = bs->rootCluster;
    char longName[256] = {0};
    char partName[14];
    DirectoryEntry *entry;

    do {
        readCluster(disk, bs, cluster, buffer);
        entry = (DirectoryEntry *)buffer;

        for (int i = 0; i < bs->sectorsPerCluster * bs->bytesPerSector / sizeof(DirectoryEntry); i++) { // calculates the number of 32 Bytes clusters 
            DirectoryEntry *dir = &entry[i];

            if (dir->DIR_Name[0] == 0x00) return;   // Microsoft says that if dir names == 0 there is no need to look further
                                                    // The special 0 value, rather than the 0xE5 value, indicates to FAT file system driver code that the
                                                    // rest of the entries in this directory do not need to be examined because they are all free.    
            if ((dir->DIR_Attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
                LongDirectoryEntry *ldir = (LongDirectoryEntry *)dir;
                int pos = 0;
                uint16_t ch;

                for (int j = 0; j < 10; j += 2) {
                    ch = ldir->LDIR_Name1[j] | (ldir->LDIR_Name1[j+1] << 8);    // ch = 0x2D | (0x4E << 8) = 0x4E2D it shifts to create 2 byte and or it finally create 0x4E2D 0r 0x002D
                    if (ch == 0xFFFF || ch == 0x0000) break;
                    partName[pos++] = (char)ch;                                 // pos++ means: use pos, then increment it.
                }
                for (int j = 0; j < 12; j += 2) {
                    ch = ldir->LDIR_Name2[j] | (ldir->LDIR_Name2[j+1] << 8);
                    if (ch == 0xFFFF || ch == 0x0000) break;
                    partName[pos++] = (char)ch;
                }
                for (int j = 0; j < 4; j += 2) {
                    ch = ldir->LDIR_Name3[j] | (ldir->LDIR_Name3[j+1] << 8);
                    if (ch == 0xFFFF || ch == 0x0000) break;
                    partName[pos++] = (char)ch;
                }

                partName[pos] = '\0';
                char temp[256];
                strcpy(temp, longName);
                strcpy(longName, partName);
                strcat(longName, temp);
            } else {
                if ((dir->DIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0x00) {
                    printf("%s\n", longName[0] ? longName : (char *)dir->DIR_Name);
                } else if ((dir->DIR_Attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
                    printf("%s/\n", longName[0] ? longName : (char *)dir->DIR_Name);
                }
                longName[0] = '\0';
            }
        }
        cluster = getNextCluster(disk, bs, cluster);
    } while (cluster < 0x0FFFFFF8);
}

int main(int argc, char *argv[]) {

    FAT32_CWDInfo info;
    read_state(&info);                        // read the config file that holds the current dir, device name and current cluster

    FILE *disk = fopen(info.deviceName , "rb");     // open the device file ex. /dev/sdd1 to read
    if (!disk) {
        perror("Failed to open disk image");
        return 1;
    }

    FAT32_BootSector bs;
    readBootSector(disk, &bs);                  // read the BPB
    listRootDirectory(disk, &bs, &info);         // list the contents of the directory

    //fclose(fileConf);
    fclose(disk);
    return 0;
}
