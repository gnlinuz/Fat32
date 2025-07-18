#ifndef DIR_H
#define DIR_H

#include <stdio.h>
#include "types.h"

#define ATTR_LONG_NAME       0x0F
#define ATTR_LONG_NAME_MASK  0x3F
#define ATTR_DIRECTORY       0x10
#define ATTR_VOLUME_ID       0x08

void findAndPrintFile(FILE *disk, const FAT32_BootSector *bs, const char *filename, FAT32_CWDInfo *info);

#endif
