#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

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
    uint8_t  LDIR_Type;
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

#endif
