#ifndef RWFILE_H
#define RWFILE_H

#include "types.h"

int read_state(FAT32_CWDInfo *info);
void write_state_binary(const FAT32_CWDInfo *info);

#endif
