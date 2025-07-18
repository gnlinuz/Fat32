#include <stdio.h>
#include <string.h>

#include "types.h"
#include "readFile.h"
#include "fat32.h"
#include "ls.h"

int main() {

    FAT32_CWDInfo info;
    read_state(&info);                              // read the config file that holds the current dir, device name and current cluster

    FILE *disk = fopen(info.deviceName , "rb");     // open the device file ex. /dev/sdd1 to read
    if (!disk) {
        perror("Failed to open disk image");
        return 1;
    }

    FAT32_BootSector bs;
    readBootSector(disk, &bs);                      // read the BPB
    listRootDirectory(disk, &bs, &info);            // list the contents of the directory

    fclose(disk);
    return 0;

}
