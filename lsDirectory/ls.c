#include <stdio.h>      // FILE, printf, fread, fseek
#include <stdint.h>     // uint8_t, uint32_t
#include <string.h>     // strlen, memcpy, strcasecmp
#include <ctype.h>      // toupper

#include "types.h"
#include "fat32.h"
#include "readFile.h"
#include "ls.h"

void listRootDirectory(FILE *disk, const FAT32_BootSector *bs, FAT32_CWDInfo *info) {
    uint8_t buffer[bs->sectorsPerCluster * bs->bytesPerSector];
    uint32_t cluster = info->currentCluster;        // read the config file for the pos of the current cluster
    // uint32_t cluster = bs->rootCluster;
    char longName[256] = {0};
    char partName[14];
    DirectoryEntry *entry;

    do {
        readCluster(disk, bs, cluster, buffer);
        entry = (DirectoryEntry *)buffer;

        size_t entries = (bs->sectorsPerCluster * bs->bytesPerSector) / sizeof(DirectoryEntry);
        for (size_t i = 0; i < entries; i++) { // calculates the number of 32 Bytes clusters 
            DirectoryEntry *dir = &entry[i];

            if (dir->DIR_Name[0] == 0x00) return;   // Microsoft says that if dir names == 0 there is no need to look further
                                                    // The special 0 value, rather than the 0xE5 value, indicates to FAT file system driver code that the
                                                    // rest of the entries in this directory do not need to be examined because they are all free.    
            if ((dir->DIR_Attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
                LongDirectoryEntry *ldir = (LongDirectoryEntry *)dir;
                int pos = 0;
                uint16_t ch;

                for (int j = 0; j < 10; j += 2) {
                    ch = ldir->LDIR_Name1[j] | (ldir->LDIR_Name1[j+1] << 8);    // ch = 0x2D | (0x4E << 8) = 0x4E2D it shifts to create 2 byte and or it finally create 0x4E2D 0r 0x002D
                    if (ch == 0xFFFF || ch == 0x0000) break;
                    partName[pos++] = (char)ch;                                 // pos++ means: use pos, then increment it.
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
                    printf("%s\n", longName[0] ? longName : (char *)dir->DIR_Name);
                } else if ((dir->DIR_Attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
                    printf("%s/\n", longName[0] ? longName : (char *)dir->DIR_Name);
                }
                longName[0] = '\0';
            }
        }
        cluster = getNextCluster(disk, bs, cluster);
    } while (cluster < 0x0FFFFFF8);
}
