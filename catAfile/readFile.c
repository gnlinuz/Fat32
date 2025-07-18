#include "readFile.h"
#include <stdio.h>


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

