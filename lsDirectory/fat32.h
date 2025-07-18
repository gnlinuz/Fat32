#ifndef FAT32_H
#define FAT32_H

#include <stdio.h>
#include "types.h"

void readBootSector(FILE *disk, FAT32_BootSector *bootSector);
uint32_t getNextCluster(FILE *disk, const FAT32_BootSector *bs, uint32_t currentCluster);
void readCluster(FILE *disk, const FAT32_BootSector *bs, uint32_t cluster, uint8_t *buffer);

#endif