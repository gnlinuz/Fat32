#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

	
#pragma pack(push, 1)
typedef struct {
    uint8_t  jumpBoot[3];
    uint8_t  OEMName[8];
    uint16_t bytesPerSector;
    uint8_t  sectorsPerCluster;
    uint16_t reservedSectorCount;
    uint8_t  numFATs;
    uint16_t rootEntryCount;
    uint16_t totalSectors16;
    uint8_t  media;
    uint16_t FATSize16;
    uint16_t sectorsPerTrack;
    uint16_t numberOfHeads;
    uint32_t hiddenSectors;
    uint32_t totalSectors32;

    uint32_t FATSize32;
    uint16_t extFlags;
    uint16_t FSVersion;
    uint32_t rootCluster;
    uint16_t FSInfo;
    uint16_t backupBootSector;
    uint8_t  reserved[12];

    uint8_t  driveNumber;
    uint8_t  reserved1;
    uint8_t  bootSignature;
    uint32_t volumeID;
    uint8_t  volumeLabel[11];
    uint8_t  fileSystemType[8];
} __attribute__((packed)) FAT32_BootSector;

typedef struct {
    uint32_t FSI_LeadSig;
    uint8_t  FSI_Reserved1[480];
    uint32_t FSI_StrucSig;
    uint32_t FSI_Free_Count;
    uint32_t FSI_Nxt_Free;
    uint8_t  FSI_Reserved2[12];
    uint32_t FSI_TrailSig;   
} __attribute__((packed)) FSInfo;

typedef struct {
    char currentPath[256];     // Fixed size
    char deviceName[64];       // Fixed size
    uint32_t currentCluster;   // 4 bytes FAT #1
} FAT32_CWDInfo;

#pragma pack(pop)


unsigned char* readFromDeviceFile(FILE *disk, FAT32_BootSector *bootsector, FSInfo *fsinfosector) {
    
    unsigned char *rbuffer = malloc(1024);
    if (rbuffer == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    //size_t bytesRead = fread(rbuffer, sizeof(unsigned char), 1024, file);
    fread(rbuffer, sizeof(unsigned char), 1024, disk);
	fseek(disk, 0, SEEK_SET);
    fread(bootsector, sizeof(FAT32_BootSector), 1, disk);
    
    fseek(disk, bootsector->bytesPerSector * bootsector->FSInfo, SEEK_SET);
    fread(fsinfosector, sizeof(FSInfo), 1, disk);

    return rbuffer;
}

void write_state_binary(const FAT32_CWDInfo *info) {
    
    FILE *fp = fopen(".fat32_cwd", "wb");
    if (!fp) {
        perror("Failed to open file for writing");
        return;
    }

    fwrite(info, sizeof(FAT32_CWDInfo), 1, fp);
    fclose(fp);

}


void readFatOne(FILE *disk, FAT32_BootSector *bs){

    uint32_t fEntry;
    uint32_t volCheckFatPos = (bs->reservedSectorCount * bs->bytesPerSector) + 4; // set the pos to read from FAT #1

    fseek(disk, volCheckFatPos, SEEK_SET);  // starting pos
    fread(&fEntry, 4, 1, disk);             // read FAT #1

    // Check for dirty volume or I/O erros
    if ((fEntry & 0x08000000) == 0x08000000) {
        printf("The volume is clean!\t");
    } else {
        printf("The volume is dirty, you need to scandisk the volume\n");
    }

    if ((fEntry & 0x04000000) == 0x04000000) {
        printf("no disk I/O errors!\n");
    } else {
        printf("Disk error encountered some data may be BAD!\n");
    }

}

int readBPBdata(FILE *disk, FAT32_BootSector *bs){

    printf("mounted: \t\t");
    for (int i=0; i<8; i++){
        printf("%c", bs->fileSystemType[i]);
    }
    printf("\n");
    printf("OEM-name:\t\t%s", bs->OEMName);
    printf("  VolumeLabel: ");
    for (int i=0; i<11; i++){
	printf("%c", bs->volumeLabel[i]);
    }
    printf("\n");
    
    if(bs->FSVersion != 0){
        printf("FSVersion not 00.00 FS not mount!\n");
        return 1;
    }
    return 0;

}



int main(int argc, char *argv[]) {
    
    // Check if an argument is provided
    if (argc < 2) {
        printf("Usage mountFAT32 %s <device_path>\n", argv[0]);
        return 1; // Exit with an error code
    }

    FILE *disk = fopen(argv[1], "rb");
    if (!disk) {
        perror("Failed to open disk image");
        return 1;
    }

    FAT32_BootSector bootSector;
    FSInfo fsInfoSector;
    FAT32_CWDInfo info;

	system("clear");

    const char *device = argv[1];                                                   // get the device-name from arg
	unsigned char *data = readFromDeviceFile(disk, &bootSector, &fsInfoSector);     // read the BPB
    uint32_t rootCluster = bootSector.rootCluster;                                  // get the current rootCluster pos
    char cPath[1] = {"/"};                                                          // set the default path to '/'

    int status = readBPBdata(disk, &bootSector);
    if(status == 1) return 1;
    readFatOne(disk, &bootSector);

    // Write data, path, device name, rootCluster pos
    strncpy(info.currentPath, cPath, sizeof(info.currentPath)); // or "/" instead of cPath
    strncpy(info.deviceName, device, sizeof(info.deviceName));
    info.currentCluster = rootCluster;
    write_state_binary(&info);

    fclose(disk);
    free(data);
    return 0;

}











