#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#pragma pack(push, 1)

typedef struct {
    uint8_t  jumpBoot[3];
    uint8_t  OEMName[8];
    uint16_t bytesPerSector;        // bytes per sector 512
    uint8_t  sectorsPerCluster;     // numer of secors per cluster 8 * 512 = 4096 size of one cluster
    uint16_t reservedSectorCount;   // The reserved sectors
    uint8_t  numFATs;               // Usualy 2
    uint16_t rootEntryCount;
    uint16_t totalSectors16;        
    uint8_t  media;
    uint16_t FATSize16;
    uint16_t sectorsPerTrack;
    uint16_t numberOfHeads;
    uint32_t hiddenSectors;
    uint32_t totalSectors32;        // total number of sectors for Fat32

    uint32_t FATSize32;             // Size of one FAT -> this is calculated 
    uint16_t extFlags;
    uint16_t FSVersion;
    uint32_t rootCluster;           // This is set to the cluster number '2' of the first cluster of the root directory
    uint16_t FSInfo;
    uint16_t backupBootSector;      // the backup of boot sector usualy 6
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
    uint8_t  LDIR_Type;             // If zero, indicates a directory entry that is a sub-component of a long name.
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


#endif