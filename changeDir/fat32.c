#include "fat32.h"
#include <stdlib.h>



void readBootSector(FILE *disk, FAT32_BootSector *bootSector){

    fseek(disk, 0, SEEK_SET);
    if (fread(bootSector, sizeof(FAT32_BootSector), 1, disk) != 1) {
        perror("Failed to read boot sector");
        exit(1);
    }

}


uint32_t getNextCluster(FILE *disk, const FAT32_BootSector *bs, uint32_t currentCluster) {
    
    uint32_t FATOffset = currentCluster * 4;                                        // move to next cluster -> if first, 2 * 4 = 8 (byte/pos)
    uint32_t sector = bs->reservedSectorCount + (FATOffset / bs->bytesPerSector);   // the way microsoft calculates the sector of FAT pos
    uint32_t offset = FATOffset % bs->bytesPerSector;                               // 8 % 512 = 8 move to the 8th pos
    uint32_t nextCluster;

    fseek(disk, sector * bs->bytesPerSector + offset, SEEK_SET);                    // reservedSectorCount * bytePerSector + offset -> move to the 8th pos if is the first cluster to inspect
	if (fread(&nextCluster, 4, 1, disk) != 1) {
        perror("Error reading next cluster");
    }
    return nextCluster & 0x0FFFFFFF;                                                // return the nextCluster after AND it with 0x0FFFFFFF

}


void readCluster(FILE *disk, const FAT32_BootSector *bs, uint32_t cluster, uint8_t *buffer, uint32_t block){    // read data clusters times the block number

    uint32_t firstDataSector = bs->reservedSectorCount + (bs->numFATs * bs->FATSize32);
    uint32_t firstSector = firstDataSector + (cluster - 2) * bs->sectorsPerCluster;
    fseek(disk, firstSector * bs->bytesPerSector, SEEK_SET);                                                    // Set the starting point to read
	size_t bytesToRead = (bs->sectorsPerCluster * bs->bytesPerSector) * block;
    if (fread(buffer, bytesToRead, 1, disk) != 1) {
        perror("Error reading cluster data");
    }

}
