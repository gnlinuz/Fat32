#include <stdio.h>
#include <stdlib.h>
#include "fat32.h"
#include "dir.h"
#include "readFile.h"
#include "types.h"

int main(int argc, char *argv[]) {

    FAT32_CWDInfo info;
    read_state(&info);  

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *disk = fopen(info.deviceName, "rb");
    if (!disk) {
        perror("Failed to open disk image");
        return 1;
    }

    FAT32_BootSector bs;
    readBootSector(disk, &bs);
    findAndPrintFile(disk, &bs, argv[1], &info);

    fclose(disk);
    return 0;
}
