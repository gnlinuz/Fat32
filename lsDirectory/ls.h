#ifndef LS_H
#define LS_H

#include <stdio.h>
#include "types.h"

void listRootDirectory(FILE *disk, const FAT32_BootSector *bs, FAT32_CWDInfo *info);

#endif