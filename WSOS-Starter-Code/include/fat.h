#pragma once
#include "types.h"
#include "string.h"
#include "shell.h"
#define FAT_SIZE (512 * 11)
#define FAT_ENTRIES_MAX (FAT_SIZE / 2)
#define CLUSTER_TO_SECTOR (24 - 2)
#define ROOT_SIZE 512
#define START_ADDR 0x20000
#define FAT0_ADDR START_ADDR
#define FAT1_ADDR (FAT0_ADDR + FAT_SIZE)
#define ROOT_ADDR (FAT1_ADDR + FAT_SIZE)
#define FAT0_SECTOR 1
#define FAT1_SECTOR (FAT0_SECTOR + (FAT_SIZE / 512))
#define ROOT_DIR_SECTOR (FAT1_SECTOR + (FAT_SIZE / 512))

typedef struct
{
    // FAT12 Bios Parameter Block
    uint8   jumpInstruction[3];
    uint8   oem[8];
    uint16  bytesPerSector;
    uint8   sectorsPerCluster;
    uint16  ReservedSectors;
    uint8   fatCount;
    uint16  rootDirectoryEntries;
    uint16  sectorCount;
    uint8   mediaDescriptorType;
    uint16  sectorsPerFat;
    uint16  sectorsPerTrack;
    uint16  headCount;
    uint32  hiddenSectorCount;
    uint32  largeSectorCount;

    // Extended Boot Record
    uint8   driveNumber;
    uint8   reserved;
    uint8   signature;
    uint32  volumeID;
    uint8   volumeLabel[11];
    uint8   systemID[8];

} __attribute__((packed)) boot_sector_t;

typedef struct
{
    // File Allocation Table (FAT)
    // We will use 16-bits for our entries
    uint16 clusters[FAT_ENTRIES_MAX];

} __attribute__((packed)) fat_t;

typedef struct
{
    // Directory entry contents
    uint8   filename[8];    //x22400 - x22407   (8) (0 - 7)
    uint8   ext[3];         //x22408 - x2240A   (3) (8 - 10)
    uint8   attributes;     //x2240B            (1) (11)
    uint16  reserved;       //x2240C - x2240D   (2) (12 - 13)
    uint16  creationTime;   //x2240E - x2240F   (2) (14 - 15)
    uint16  creationDate;   //x22410 - x22411   (2) (16 - 17)
    uint16  lastAccessDate; //x22412 - x22413   (2) (18 - 19)
    uint16  ignored;        //x22414 - x22415   (2) (20 - 21)
    uint16  lastWriteTime;  //x22416 - x22417   (2) (22 - 23)
    uint16  lastWriteDate;  //x22418 - x22419   (2) (24 - 25)
    uint16  startingCluster;//x2241A - x2241B   (2) (26 - 27)
    uint32  fileSize;       //x2241C - x2241F   (4) (28 - 31)

} __attribute__((packed)) directory_entry_t;

typedef struct
{
    uint32 index;
    uint8 *startingAddress;

    // Set to 0 if not opened
    // Set to non-zero if opened
    char isOpened;

    // The directory entry for the file, containing all its metadata
    directory_entry_t *directoryEntry;

} __attribute__((packed)) file_t;

typedef struct
{   
    uint32 index;
    uint8 *startingAddress;

    // Set to 0 if not opened
    // Set to non-zero if opened
    char isOpened;

    // The directory entry for the directory, containing all its metadata
    directory_entry_t *directoryEntry;

} __attribute__((packed)) directory_t;

extern fat_t *fat0;
extern fat_t *fat1;

extern directory_t currentDirectory;  // The current directory we have opened
extern directory_entry_t rootDirectoryEntry;   // The root directory's directory entry (this does not exist on the disk since the root is not inside of another directory)
extern file_t currentFile;            // The current file we have opened

void init_fs();
int openFile(char *filename, char* ext);
int closeFile();
int createFile(char *filename, char* ext);
int deleteFile();
uint8 readByte(uint32 index);
uint8 readNextByte();
int writeByte(uint8 byte, uint32 index);
int writeBytes(uint8 byte, uint32 count);
int writeNextByte(uint8 byte);
uint16 nextFreeCluster();
int createSubdirectory(char *subdirectoryName);
int openDirectory(char *directoryName);
int directoryExists(char *directoryName);
int deleteSubdirectory(char *directoryName);

int writeFATs();
int writeCurrentDirectory();
int writeCurrentFile();
int resizeFile(uint16 startingCluster, uint32 newFileSize);

int createSubdirectory(char *subdirectoryName);
int openDirectory(char *directoryName);

void listEntries();

void padFileName(char *name);
void padExtension(char *ext);
void pad(char *string, char character, int count);