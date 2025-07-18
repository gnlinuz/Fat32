#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include<math.h>
/*
	Estimate the count of data clusters:
	CountOfDataClusters = TotalSectors - ReservedSectors - (NumberOfFATs × FATSize)
	ClusterCount = CountOfDataClusters / SectorsPerCluster

	Estimate how many FAT entries are needed:
	One FAT entry per cluster
	Add 2 reserved clusters (cluster 0 and 1)
	Compute required FAT size:
	RequiredFATSize = ceil((ClusterCount + 2) × 4 / BytesPerSector)
    If RequiredFATSize != current FATSize, iterate:
    This is because FATSize itself affects ClusterCount, which in turn affects FATSize. So you loop until the values stabilize.


	Define structures to match FAT32 BPB (BIOS Parameter Block)
 	The use of __attribute__((packed)) in the typedef struct
 	is specifically to prevent the compiler from inserting padding bytes between structure members, ensuring the structure's layout in memory exactly matches 
  	the layout defined by the FAT32 boot sector specification.
	Without __attribute__((packed)), the structure may have unexpected padding, which would make reading a FAT32 boot sector from a raw disk inaccurate — you'd end up with incorrect field values.

*/

#define PRINT_UTF8(ch, count, color)	\
do{										\
	printf("\033[%sm", color);			\
	for(int i = 0; i < (count); i++){	\
		printf("%s", ch);				\
	}									\
	printf("\033[0m\n");				\
}while(0);


#define PRINT_CHARS(ch, count)       \
    for (int i = 0; i < (count); i++) putchar(ch); \
    putchar('\n');
	
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
#pragma pack(pop)



unsigned char* readFromDeviceFile(const char* fname, FAT32_BootSector *bootsector, FSInfo *fsinfosector) {
    
    // SEEK_SET: Beginning of the file
    // SEEK_CUR: Current file position
    // SEEK_END: End of the file
	
    FILE *file;
    
    unsigned char *rbuffer = malloc(1024);
    if (rbuffer == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }

    file = fopen(fname, "rb");
    if (file == NULL) {
        printf("Error: file not found!\n");
        free(rbuffer);
        return NULL;
    }

    //size_t bytesRead = fread(rbuffer, sizeof(unsigned char), 1024, file);
    fread(rbuffer, sizeof(unsigned char), 1024, file);
	fseek(file, 0, SEEK_SET);
    fread(bootsector, sizeof(FAT32_BootSector), 1, file);
    
    fseek(file, bootsector->bytesPerSector * bootsector->FSInfo, SEEK_SET);
    fread(fsinfosector, sizeof(FSInfo), 1, file);
    fclose(file);

    return rbuffer;
}



uint32_t getNextCluster(const char* fname, const FAT32_BootSector *bootSector, const uint32_t calculatedFATSize, uint32_t* firstEntry) {
    
	uint32_t FATSizeBytes = calculatedFATSize * bootSector->bytesPerSector;
    uint32_t totalNumberClusters = FATSizeBytes / 4;
	uint32_t countFreeClusters = 0;

    FILE *disk = fopen(fname, "rb");
    if (!disk) {
        perror("Failed to open disk image");
        return 1;
    }

    // Allocate memory to load the whole FAT into memory
    uint8_t *FAT = malloc(FATSizeBytes);
    if (!FAT) {
        perror("Memory allocation failed");
        fclose(disk);
        return 1;
    }

    // Seek to start of the FAT
    fseek(disk, bootSector->reservedSectorCount * bootSector->bytesPerSector, SEEK_SET);
    fread(FAT, 1, FATSizeBytes, disk); // read the whole FAT0 block 

    // Now loop over FAT entries in memory
    for (uint32_t i = 0; i < totalNumberClusters; i++) {
		uint32_t entry;
        memcpy(&entry, FAT + i * 4, 4);
        if ((entry == 0x00000000) || (entry == 0x10000000) || (entry == 0xF0000000)) {
            countFreeClusters++;
        }
    }

    // Optional: check for dirty volume, etc., still using in-memory data
    memcpy(firstEntry, FAT + 1 * 4, 4);  // cluster #1
    
	free(FAT);
    fclose(disk);
    return countFreeClusters;

}




int main(int argc, char *argv[]) {
    
    // Check if an argument is provided
    if (argc < 2) {
        printf("Usage: %s <device_path>\n", argv[0]);
        return 1; // Exit with an error code
    }

    FAT32_BootSector bootSector;
    FSInfo fsInfoSector;
	const char *filename = argv[1];
	
	system("clear");

	unsigned char *data = readFromDeviceFile(filename, &bootSector, &fsInfoSector);
    
    /*
    for(int i=0; i<1024; i++){
        //if (i % 16 == 0) printf("\n%04x: ", i);  // offset header
        if (i % 16 == 0) printf("\n");
        printf("%02X ", data[i]);
    }
    */
	
	//How to calculate the required FAT size! 
    uint32_t CountOfDataClusters = bootSector.totalSectors32 - bootSector.reservedSectorCount - (bootSector.FATSize32 * bootSector.numFATs);
    uint32_t ClusterCount = CountOfDataClusters / bootSector.sectorsPerCluster;
    uint32_t RequiredFATSize = ceil((ClusterCount + 2) * 4 / bootSector.bytesPerSector);
	

	PRINT_UTF8("\xE2\x96\x88", 66, "34");
	printf("\t\tFAT32 BPB information table.\n");
	PRINT_UTF8("\xE2\x96\x88", 66, "34");

	printf("bytesPerSector:\t\t0x%x\t\t\t%u bytes\n", bootSector.bytesPerSector, bootSector.bytesPerSector);
    printf("Sectors per cluster:\t0x%x\t\t\t%u\n", bootSector.sectorsPerCluster, bootSector.sectorsPerCluster);
    printf("Bytes per Cluster:\t0x%x\t\t\t%u bytes\n", bootSector.bytesPerSector * bootSector.sectorsPerCluster, bootSector.bytesPerSector * bootSector.sectorsPerCluster);
    printf("Reserved Sec count:\t0x%x\t\t\t%u bytes\n",bootSector.reservedSectorCount, bootSector.reservedSectorCount * bootSector.bytesPerSector);
    printf("Total FAT32 Sectors:\t0x%x\t\t%llu bytes\n",bootSector.totalSectors32, (unsigned long long)((uint64_t) bootSector.totalSectors32 * bootSector.bytesPerSector));
	
	printf("No FATs:\t\t%x\n",bootSector.numFATs);
	printf("Size of one FAT:\t0x%x\t\t\t%u bytes\n",bootSector.FATSize32, bootSector.FATSize32 * bootSector.bytesPerSector);
    printf("Size of both FATs:\t0x%x\t\t\t%u bytes\n",bootSector.FATSize32 * bootSector.numFATs, (bootSector.FATSize32 * bootSector.numFATs) * bootSector.bytesPerSector);
	printf("Required FAT size:\t0x%x\t\t\t%u bytes\n", RequiredFATSize, RequiredFATSize * bootSector.bytesPerSector);
    
	printf("Root exntry count:\t%x\n",bootSector.rootEntryCount);
	printf("rootCluster:\t\t0x%x\n",bootSector.rootCluster);
	printf("firstDataSector:\t0x%x\t\t\t%u bytes\n", bootSector.reservedSectorCount + (bootSector.FATSize32 * bootSector.numFATs),((bootSector.reservedSectorCount + (bootSector.FATSize32 * bootSector.numFATs)) * bootSector.bytesPerSector) ); 
	printf("backupBootSector:\t%x\n",bootSector.backupBootSector);

	printf("OEM-name:\t\t%s\n", bootSector.OEMName);
    printf("volumeLabel: \t\t");
    for (int i=0; i<11; i++){
	printf("%c", bootSector.volumeLabel[i]);
    }
    printf("\n");
    
    printf("volumeID:\t\t0x%x\n",bootSector.volumeID);
    printf("bootSignature:\t\t0x%x\n",bootSector.bootSignature);
    printf("extFlags:\t\t%x\n",bootSector.extFlags);
    printf("FSVersion:\t\t%x\n",bootSector.FSVersion);
    printf("FSInfo:\t\t\t%x\n",bootSector.FSInfo);
	
	printf("fileSystemType: \t");
    for (int i=0; i<8; i++){
        printf("%c", bootSector.fileSystemType[i]);
    }
    printf("\n");
    printf("FSI_LeadSig:\t\t%x\n", fsInfoSector.FSI_LeadSig);
    printf("FSI_StrucSig:\t\t%x\n", fsInfoSector.FSI_StrucSig);
    printf("FSI_TrailSig:\t\t%x\n" ,fsInfoSector.FSI_TrailSig);

    uint32_t countFreeClusters = 0;
	uint32_t fEntry;
	countFreeClusters = getNextCluster(filename, &bootSector, RequiredFATSize, &fEntry);
	
	printf("FSI_Free_Count:\t\t0x%x\n", fsInfoSector.FSI_Free_Count);
    printf("Free FAT clusters: \t0x%x\n", countFreeClusters);
    printf("FSI_Nxt_Free:\t\t0x%x\t\t\t%u decimal\n", fsInfoSector.FSI_Nxt_Free, fsInfoSector.FSI_Nxt_Free);
	PRINT_UTF8("\xE2\x96\x88", 66, "34");

    if ((fEntry & 0x08000000) == 0x08000000) {
        printf("The volume is clean!\n");
    } else {
        printf("The volume is dirty, you need to scandisk the volume\n");
    }

    if ((fEntry & 0x04000000) == 0x04000000) {
        printf("No disk I/O errors!\n");
    } else {
        printf("Disk error encountered — some data may be BAD!\n");
    }
	PRINT_UTF8("\xE2\x96\x88", 66, "34");

    free(data);

    return 0;

}











