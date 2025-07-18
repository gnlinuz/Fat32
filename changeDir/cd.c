#include <stdio.h>      // FILE, printf, fread, fseek
#include <stdint.h>     // uint8_t, uint32_t
#include <string.h>     // strlen, memcpy, strcasecmp
#include <ctype.h>      // toupper
#include <strings.h>    // for linux env and old compiler

#include "types.h"
#include "fat32.h"
#include "rwFile.h"
#include "cd.h"


void findAndPrintFile(FILE *disk, const FAT32_BootSector *bs, const char *filename, FAT32_CWDInfo *info){

    uint8_t buffer [bs->bytesPerSector * bs->sectorsPerCluster];                    // ex. 512 * 8 = 4096 The buffer size is 4096 one cluster
    uint32_t cluster = info->currentCluster;                                        // READ THE CURRENT FOLDER FAT POS 6
    char longName[256] = {0};
    char partName[14];
    uint32_t block = 1;
    //uint8_t flag = 0; 

    DirectoryEntry *entry;

    do {
        readCluster(disk, bs, cluster, buffer, block);                                                  // read the whole cluster ex. 4096 bytes

        entry = (DirectoryEntry *)buffer;                                                               // Cast DirectoryEntry to buffer
        size_t entries = (bs->sectorsPerCluster * bs->bytesPerSector) / sizeof(DirectoryEntry);
        for (size_t i = 0; i < entries; i++) {                                                          // calculates the number of clusters (32 Bytes) 
            DirectoryEntry *dir = &entry[i];                                                            // A *dir pointer of DirectoryEntry points to &entry[i] 

            if (dir->DIR_Name[0] == 0x00) return;                                                       // Microsoft says that if dir names == 0 there is no need to look further
                                                                                                        // The special 0 value, rather than the 0xE5 value, indicates to FAT file system driver code that the
                                                                                                        // rest of the entries in this directory do not need to be examined because they are all free.    
            if ((dir->DIR_Attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
                LongDirectoryEntry *ldir = (LongDirectoryEntry *)dir;                                   // cast LongDirectoryEntry ->dir to *ldir
                int pos = 0;
                uint16_t ch;

                for (int j = 0; j < 10; j += 2) {
                    ch = ldir->LDIR_Name1[j] | (ldir->LDIR_Name1[j+1] << 8);                            // ch = 0x2D | (0x4E << 8) = 0x4E2D it shifts to create 2 byte and or it finally create 0x4E2D 0r 0x002D
                    if (ch == 0xFFFF || ch == 0x0000) break;
                    partName[pos++] = (char)ch;                                                         // pos++ means: use pos, then increment it.
                }
                for (int j = 0; j < 12; j += 2) {
                    ch = ldir->LDIR_Name2[j] | (ldir->LDIR_Name2[j+1] << 8);
                    if (ch == 0xFFFF || ch == 0x0000) break;
                    partName[pos++] = (char)ch;
                }
                for (int j = 0; j < 4; j += 2) {
                    ch = ldir->LDIR_Name3[j] | (ldir->LDIR_Name3[j+1] << 8);
                    if (ch == 0xFFFF || ch == 0x0000) break;
                    partName[pos++] = (char)ch;
                }

                partName[pos] = '\0';
                char temp[256];
                strcpy(temp, longName);
                strcpy(longName, partName);
                strcat(longName, temp);
            } else {
                if ((dir->DIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0x00) {
                        // found a file      
                } else if ((dir->DIR_Attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {        
                        // found a directory
                    
                    if((dir->DIR_Name[0] == 0x2E && dir->DIR_Name[1] == 0x2E) && strcasecmp("..", filename) == 0){
                        // found the dot dot ..

                        uint32_t high = (uint16_t)dir->DIR_FstClusHI;
                        uint32_t low =  (uint16_t)dir->DIR_FstClusLO;    
                        uint32_t fatClusterPos = (high << 16) | low;                            // create the MSB+LSB pos of FAT cluster

                        if(fatClusterPos == 0){                                                 // it means that reached root dir /
                            strncpy(info->currentPath, "/", sizeof(info->currentPath)-1);
                            info->currentCluster = 2;
                            write_state_binary(info);                                           // write data to config file
                            printf("/\n");                                                      // print the '/' of the root dir
                        } else {                                                                // if fatClusterPos != 0 need to go back one level of dir info->currentPath = /kenos/myfolder/                                                                                         
                            
                            size_t len = strlen(info->currentPath);                             // get the size of the path                                                                                              
                            if (len > 1 && info->currentPath[len - 1] == '/') {                 // Remove trailing slash if exists and it's not the root        
                                info->currentPath[len - 1] = '\0';                              // eliminate the last / and add \0
                                len--;
                            }
                            char tmp2[len];
                            strcpy(tmp2, info->currentPath);
                            char *lastSlash = strrchr(tmp2, '/');                  // create a pointer that points to info.currentPath on the last occurance of /

                            if (lastSlash != NULL && lastSlash != tmp2) {          // at that point '/' add a \0 to truncate the path                                                                 
                                *(lastSlash + 1) = '\0';       
                            } //else if (lastSlash == info->currentPath) {         // It's already root (/), keep it as is
                              //  info->currentPath[1] = '\0';
                              //}
                            strncpy(info->currentPath, tmp2, sizeof(info->currentPath) - 1);
                            info->currentPath[sizeof(info->currentPath) - 1] = '\0';  
                            info->currentCluster = fatClusterPos;                               // set the FAT pos
                            printf("%s\n", info->currentPath);
                            write_state_binary(info);                                           // write the config file with the new path, deviceName, and new cluster pos
                            return;
                        }
                        // flag = 1;    
                    }

                    if(longName[0]){
                        if(strcasecmp(longName, filename) == 0){                                // found the directory to "cd"
                            // flag = 1;
                            uint32_t high = (uint16_t)dir->DIR_FstClusHI;
                            uint32_t low =  (uint16_t)dir->DIR_FstClusLO;    
                            uint32_t fatClusterPos = (high << 16) | low;                        // create the MSB+LSB pos of FAT cluster
                            //printf("the fat pos: %d\n", fatClusterPos);

                            strncat(longName, "/",sizeof(longName) - strlen(longName) - 1);                                  // add to the folder you find a myfolder'/'
                            strncat(info->currentPath, longName, sizeof(info->currentPath) - strlen(info->currentPath) - 1); // append to /myfolder/ minus the \0 
                            info->currentCluster = fatClusterPos;
                            write_state_binary(info);
                            printf("%s\n", info->currentPath);
                            return;
                        }    
                    }
                }
                longName[0] = '\0';
            }
        }
        
        if(cluster != 0x0FFFFFFF) cluster = getNextCluster(disk, bs, cluster);
        // if(flag == 1) break;

    }while(cluster < 0xfffffff8);   // read until you find the end of fat chain
    
}
