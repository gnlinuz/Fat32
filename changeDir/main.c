#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "types.h"
#include "cd.h"
#include "rwFile.h"
#include "fat32.h"


int main(int argc, char *argv[]){

    const char *DEFAULT_ROOT_PATH = "/";

    FAT32_CWDInfo info;
    read_state(&info);                                                              // read the config file that holds the currentPath, deviceName and currentCluster

    FILE *disk = fopen(info.deviceName, "rb");                                      // use the config file to open device ex/dev/sdd1
    if(!disk){
        perror("Failed to open disk image");
        return 1;
    }

    if(argc != 2){
        fprintf(stderr, "Usage: %s filename.txt\n",argv[0]);
        return 1;
    }
    const char *filename = argv[1];                                                 // argv holds the folder name to change directory

    FAT32_BootSector bs;                                                            // BPB
    readBootSector(disk, &bs);                                                      // read BPB

    if(strcasecmp(DEFAULT_ROOT_PATH, filename) == 0){                               // if cdir / is used to go directly to root dir
        
        strncpy(info.currentPath, DEFAULT_ROOT_PATH, sizeof(info.currentPath)-1);   // -1 ensure the null termination when use strncpy 
        info.currentCluster = bs.rootCluster;
        write_state_binary(&info);
        printf("/\n");

        fclose(disk);
        return 0;                                                                   // Stop the program here

    }

    findAndPrintFile(disk, &bs, filename, &info);

    fclose(disk);
    return 0;

}
