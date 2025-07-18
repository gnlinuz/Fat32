#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dir.h"
#include "fat32.h"
#include "readFile.h"


void findAndPrintFile(FILE *disk, const FAT32_BootSector *bs, const char *filename, FAT32_CWDInfo *info){

    uint8_t buffer [bs->bytesPerSector * bs->sectorsPerCluster];                    // ex. 512 * 8 = 4096 The buffer size is 4096 one cluster
    // uint32_t cluster = bs->rootCluster;
    uint32_t cluster = info->currentCluster;                                        // read the FAT cluster pos from the conf file
    char longName[256] = {0};
    char partName[14];
    uint32_t block = 1;
   
    DirectoryEntry *entry;

    do {
        readCluster(disk, bs, cluster, buffer, block);                              // read the whole cluster ex. 4096 bytes

        entry = (DirectoryEntry *)buffer;                                           // Cast DirectoryEntry to buffer

        for (unsigned int i = 0; i < bs->sectorsPerCluster * bs->bytesPerSector / sizeof(DirectoryEntry); i++) { // calculates the number of 32 Bytes clusters 
            DirectoryEntry *dir = &entry[i];                                        // A *dir pointer of DirectoryEntry points to &entry[i] 

            if (dir->DIR_Name[0] == 0x00) return;                                   // Microsoft says that if dir names == 0 there is no need to look further
                                                                                    // The special 0 value, rather than the 0xE5 value, indicates to FAT file system driver code that the
                                                                                    // rest of the entries in this directory do not need to be examined because they are all free.    
            if ((dir->DIR_Attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
                LongDirectoryEntry *ldir = (LongDirectoryEntry *)dir;
                int pos = 0;
                uint16_t ch;

                for (int j = 0; j < 10; j += 2) {
                    ch = ldir->LDIR_Name1[j] | (ldir->LDIR_Name1[j+1] << 8);        // ch = 0x2D | (0x4E << 8) = 0x4E2D it shifts to create 2 byte and or it finally create 0x4E2D 0r 0x002D
                    if (ch == 0xFFFF || ch == 0x0000) break;
                    partName[pos++] = (char)ch;                                     // pos++ means: use pos, then increment it.
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
                    if(longName[0]){
                        if(strcasecmp(longName, filename) == 0){                                                // Found the file to cat
                            uint32_t high = (uint16_t)dir->DIR_FstClusHI;
                            uint32_t low =  (uint16_t)dir->DIR_FstClusLO;    
                            uint32_t fatClusterPos = (high << 16) | low;                                        // create the MSB+LSB pos of FAT cluster
                            uint32_t numberOfClusters = 0;                                                      // The number of clusters a file is spread across
                            cluster = getNextCluster(disk, bs, fatClusterPos);
                            numberOfClusters = 1;
                            while (cluster < 0x0FFFFFF8 && cluster >= 0x00000002){
                                cluster = getNextCluster(disk, bs, cluster);
                                numberOfClusters++;
                            }
                            uint8_t *fileBuffer = malloc((bs->bytesPerSector * bs->sectorsPerCluster) * numberOfClusters); // Suggestion: Use malloc for fileBuffer
                            if (!fileBuffer) {
                                fprintf(stderr, "Memory allocation failed\n");
                                exit(1);
                            }
                            /* uint8_t fileBuffer [(bs->bytesPerSector * bs->sectorsPerCluster) * numberOfClusters];  // Avoid Large Stack Allocations
                              fileBuffer can be huge (especially if the file is fragmented). This can lead to stack overflow. Suggestion: Use malloc for fileBuffer
                              This allocates memory on the stack. For example, if your cluster size is 4096 bytes and the file spans 100 clusters, you're allocating:
                              4096 * 100 = 409,600 bytes 400 KB
                              That's 400 KB on the stack, which is very risky.
                              Allocating large buffers like this can cause a stack overflow.
                              Stack overflows are not caught gracefully they usually result in a segmentation fault or crash.
                              Why malloc is Safer
                              uint8_t *fileBuffer = malloc(bufferSize);
                              This allocates memory from the heap, which is much larger (often GBs depending on system).
                              You can safely allocate large buffers here just remember to free them when done.
                              Even if malloc fails, you can detect and handle it:
                            */
                            readCluster(disk, bs, fatClusterPos, fileBuffer, numberOfClusters); // read the whole cluster ex. 4096 * numberOfClusters, one file can be spread across multiple clusters
                            // printf("%s\n", fileBuffer);
                            fwrite(fileBuffer, 1, dir->DIR_FileSize, stdout);
                            free(fileBuffer);
                            return;
                        }
                    }
                } else if ((dir->DIR_Attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {                // found a directory
                    // printf("%s/\n", longName[0] ? longName : (char *)dir->DIR_Name);
                }
                longName[0] = '\0';
            }
        }
        
        if(cluster != 0x0FFFFFFF) cluster = getNextCluster(disk, bs, cluster);

    }while(cluster < 0xfffffff8);   // read until you find the end of fat chain

}
