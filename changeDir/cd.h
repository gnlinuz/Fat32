#ifndef CD_H
#define CD_H

#include <stdio.h>
#include "types.h"

void findAndPrintFile(FILE *disk, const FAT32_BootSector *bs, const char *filename, FAT32_CWDInfo *info);

#endif
