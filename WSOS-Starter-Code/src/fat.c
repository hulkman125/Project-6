#include "fat.h"
#include "fdc.h"
#include "string.h"
#include "io.h"

// Initialize the file system
// Loads the FATs and root directory
void init_fs()
{
    // The FATs and directory are loaded into 0x7C00
    // These addresses were chosen because they are far enough away from the kernel (0x01000 - 0x07BFF)

    // Read the first copy of the FAT (Drive 0, Cluster 1, 512 bytes * 9 clusters)
    fat0 = (fat_t *) (FAT0_ADDR); // Put FAT at 0x20000
    floppy_read(0, FAT0_SECTOR, (void *)fat0, FAT_SIZE);

    // Read the second copy of the FAT (Drive 0, Cluster 10, 512 bytes * 9 clusters)
    fat1 = (fat_t *) (FAT1_ADDR); // Put FAT at 0x21200
    floppy_read(0, FAT1_SECTOR, (void *)fat1, FAT_SIZE);

    // Read the root directory (Drive 0, Cluster 19, 512 bytes * 14 clusters)
    currentDirectory.isOpened = 1;
    currentDirectory.directoryEntry = &rootDirectoryEntry;
    currentDirectory.directoryEntry->startingCluster = 0x0000;
    currentDirectory.directoryEntry->attributes = 0x10;
    currentDirectory.directoryEntry->fileSize = ROOT_SIZE;

    currentDirectory.startingAddress = (uint8 *) (ROOT_ADDR); // Put ROOT at 0x22400
    stringcopy("ROOT    ", (char *)currentDirectory.directoryEntry->filename, 8);

    floppy_read(0, ROOT_DIR_SECTOR, (void *)currentDirectory.startingAddress, ROOT_SIZE);

    // Start our file out blank
    currentFile.isOpened = 0;
    currentFile.directoryEntry = 0;
    currentFile.index = 0;
    currentFile.startingAddress = 0;

    // Set the path to start at root
    stringcopy("./", shellPath, 3);
}

uint16 nextFreeCluster()
{
    // Loop over the FAT table
    for (int i = 2; i < FAT_SIZE / 2; i++)
    {
        // If this entry is free, return it
        if (fat0->clusters[i] == 0) return i;
    }

    // If we never returned, then there are no free entries in the FAT!
    print("Error: Disk is full!\n");
    return 0xFFFF;
}

int closeFile()
{
    // If the file is not open we cannot close it
    if(!currentFile.isOpened)
    {
        print("Error: No file was opened to be closed.\n");
        return 1;
    }

    // Resize the file if it grew or shrunk
    int resizeFailed = resizeFile(currentFile.directoryEntry->startingCluster, currentFile.directoryEntry->fileSize);

    if (resizeFailed)
    {
        print("Error: Resizing the file in the FAT failed when closing!\n");
        return 1;
    }

    // Write the file to the disk
    int writeFailed = writeCurrentFile();

    if (writeFailed)
    {
        print("Error: Writing the file to the disk failed when closing!\n");
        return 1;
    }

    // Write the directory to the disk
    writeFailed = writeCurrentDirectory();

    if (writeFailed)
    {
        print("Error: Writing the directory to the disk failed when closing!\n");
        return 1;
    }

    // Label the file as not open
    currentFile.isOpened = 0;
    currentFile.index = 0;

    return 0;
}

// Creates an empty file and writes it to the floppy disk
// You can see the effects of calling this function by viewing your OS image in a hex editor
int createFile(char *filename, char *ext)
{
    // Point to the first directory entry in our directory
    directory_entry_t *emptyEntry = (directory_entry_t *)currentDirectory.startingAddress;
    currentFile.directoryEntry = emptyEntry;
    
    // If the first directory entry is not empty, loop until we find an empty directory entry
    uint8 entryCount = 1;
    while(emptyEntry->filename[0] != 0)
    {
        // Incrementing emptyEntry points to the next 32 byte directory entry
        emptyEntry++;
        entryCount++;
        currentFile.directoryEntry = emptyEntry;
    }

    // Do we need a larger directory? (We can only resize non-root directories)
    uint32 directorySizeNeeded = entryCount * 32;
    uint32 directorySize = currentDirectory.directoryEntry->fileSize;
    char directoryIsTooSmall = directorySizeNeeded > directorySize;
    char isRootDirectory = currentDirectory.directoryEntry->startingCluster == 0;

    // If we need to expand the directory
    if (directoryIsTooSmall && !isRootDirectory)
    {
        int resizeFailed = resizeFile(currentDirectory.directoryEntry->startingCluster, directorySizeNeeded);

        if (resizeFailed)
        {
            print("Error: Could not resize the directory when creating a new file!\n");
            return 1;
        }
    }

    // If we do need to resize and this is the root directory, then the root directory is full!
    if (directoryIsTooSmall && isRootDirectory)
    {
        print("Error: The root directory is full!\n");
        return 1;
    }

    // Copy name and extension and set default size and starting cluster
    stringcopy("        ", (char *) currentFile.directoryEntry->filename, 8);
    stringcopy("   ", (char *) currentFile.directoryEntry->ext, 3);
    stringcopy(filename, (char *)currentFile.directoryEntry->filename, 8);
    stringcopy(ext, (char *)currentFile.directoryEntry->ext, 3);
    currentFile.directoryEntry->fileSize = 512;
    currentFile.directoryEntry->startingCluster = nextFreeCluster();
    currentFile.isOpened = 1;

    // Update the FAT tables to reflect that we created a new file and it has one cluster
    fat0->clusters[currentFile.directoryEntry->startingCluster] = 0xFFFF;
    fat1->clusters[currentFile.directoryEntry->startingCluster] = 0xFFFF;
    writeFATs();

    
    // Set the starting address of the file to some memory location
    uint8 *startingAddress = (void *)0x30000;
    currentFile.startingAddress = startingAddress;

    // Add 0's to this file
    for(int i = 0; i < 512; i++)
    {
        *(currentFile.startingAddress + i) = 0;
    }

    // Close the file to write it (512 bytes of 0's)
    closeFile();

    // Write the directory to the disk
    int writeFailed = writeCurrentDirectory();

    if (writeFailed)
    {
        print("Error: Could not write the current directory with newly created file to the disk!\n");
        return 1;
    }

    return 0;
}

// Returns a byte from a file that is currently loaded into memory
// This does NOT modify the floppy disk
// This function requires the file to have been loaded into memory with floppy_read()
uint8 readByte(uint32 index)
{
    // Are we trying to read from the end of a file?
    if(index >= currentFile.directoryEntry->fileSize)
    {
        return -2;
    }

    // Check if the file is opened and is not a NULL pointer
    if(currentFile.isOpened && currentFile.startingAddress != 0)
    {
        currentFile.index = index + 1;              // Point us to the next index
        // Return the byte at the specified index
        return currentFile.startingAddress[index];
    }

    // If the file was not opened, or was a NULL pointer, return error
    print("Error: File was not opened or pointed to NULL!\n");
    return -1;
}

// Returns the next byte from a file that is currently loaded into memory
uint8 readNextByte()
{
    return readByte(currentFile.index);
}

// Writes a byte to the current file that is currently loaded into memory
// This does NOT modify the floppy disk
// To write this to the floppy disk, we have to call floppy_write()
int writeByte(uint8 byte, uint32 index)
{
    // Check if the file is opened and is not a NULL pointer
    if(currentFile.isOpened && currentFile.startingAddress != 0)
    {
        currentFile.startingAddress[index] = byte;  // Place the byte at the address + index
        currentFile.directoryEntry->fileSize = ((index / 512) + 1) * 512;    // Increase the file size
        currentFile.index = index + 1;              // Point us to the next index
        return 0;
    }

    // If the file was not opened, or was a NULL pointer, return error
    print("Error: File was not opened or pointed to NULL!\n");
    return -1;
}

// Writes a byte to the current file that is currently loaded into memory at the next index
int writeNextByte(uint8 byte)
{
    return writeByte(byte, currentFile.index);
}

// Writes a byte to the current file that is currently loaded into memory at the next index multiple times
int writeBytes(uint8 byte, uint32 count)
{
    for(int i = 0; i < (int)count; i++)
    {
        int error = writeByte(byte, currentFile.index);

        if (error != 0) return error;
    }

    return 0;
}

// Finds a file within our current directory and loads every sector of the file into memory
// Returns 0 if the file was found in the current directory
// Returns -3 if the file was not found in the current directory
// Returns other error codes if something went wrong
int openFile(char *filename, char *ext)
{
	// If a file is open, stop!
    if(currentFile.isOpened)
    {
        print("A file is already open! Please close this file before opening another!\n");
        return 0;
    }

    // Pad our filename and extension with spaces
    char filenamepadded[] = "        ";
    char extpadded[] = "   ";

    // Pad the filename
    int index = 0;
    while (filename[index] != 0 && index < 8)
    {
        filenamepadded[index] = filename[index];
        index++;
    }

    // Pad the extension
    index = 0;
    while (ext[index] != 0 && index < 3)
    {
        extpadded[index] = ext[index];
        index++;
    }

    // Get a pointer to the first address of the directory entry in this directory
	directory_entry_t *directoryEntry = (directory_entry_t *)currentDirectory.startingAddress;
    char fileExists = 0;

    // Assume we can have a maximum of 16 directory entries per directory
    // Check each one to see if our filename and extension match
    // This should actually be checking if the first byte of the directory entry is null
	while(directoryEntry->filename[0] != 0)
	{
        // Check if this entry has the same file name and extension
		fileExists = stringcompare((char *)directoryEntry->filename, filenamepadded, 8) && stringcompare((char *)directoryEntry->ext, extpadded, 3);
		
        // If we found the file we can stop looping!
		if(fileExists) break;

		directoryEntry++;
	}

    // If the file exists, let's open it
    if(fileExists)
    {
        // Check if the file has been corrupted
        // Check each entry in the FAT table, ensure both FATs are consistent before opening the file
        uint16 cluster = directoryEntry->startingCluster;
        while(cluster != 0xFFFF)
        {
            // Check if the copies of the FATs are consistent
            if(fat0->clusters[cluster] != fat1->clusters[cluster])
            {
                print("Error: The file was found BUT the FAT table entries for this file differ!\n");
                return -1;
            }

            // Get the next cluster
            cluster = fat0->clusters[cluster];
        }

        // Set the starting address of the file to some memory location
        uint8 *startingAddress = (void *)0x30000;

        // Starting at the first sector, each each sector from floppy into memory
        cluster = directoryEntry->startingCluster;
        uint16 sectorCount = 0;

        // Loop through every link in the FAT
        while(cluster != 0xFFFF)
        {
            // Convert the cluster to a sector
            uint32 sector = cluster + CLUSTER_TO_SECTOR;
            
            // Read the sector from the floppy disk
            floppy_read(0, sector, (void *) startingAddress + (512 * sectorCount), 512);
            sectorCount++;

            // Get the next cluster
            cluster = fat0->clusters[cluster];

            // It is possible to get stuck in an infinite loop, reading FAT entries forever
            // We prevent that here by checking if the amount of sectors could actually fit on disk
            if(sectorCount > FAT_ENTRIES_MAX)
            {
                print("Error: The file appears to be bigger than the entire floppy disk!\n");
                return -2;
            }
        }

        // If no error has occured, label the file as opened and point it to all the data we just read in
        currentFile.directoryEntry = directoryEntry;
        currentFile.startingAddress = startingAddress;
        currentFile.index = 0;
        currentFile.isOpened = 1;
        return 0;
    }

    // If we did not find the file return -3
	return -3;
}

int directoryExists(char *directoryName)
{
    // Check if the current directory is opened, if not, return
    if (!currentDirectory.isOpened)
    {
        print("Error: Could not search for a subdirectory because there is no currently opened directory.\n");
        return 1;
    }

    // Get a pointer to the first address of the directory entry in this directory
	directory_entry_t *directoryEntry = (directory_entry_t *)currentDirectory.startingAddress;
    char exists = 0;

    // Pad the file name with spaces
    padFileName(directoryName);

	while (directoryEntry->filename[0] != 0)
	{
        // Check if this entry has the same file name and extension
		exists = stringcompare((char *)directoryEntry->filename, directoryName, 8) && ((directoryEntry->attributes & 0x10) == 0x10);
		
        // If we found the file we can stop looping!
		if(exists) return exists;

		directoryEntry++;
	}

    return exists;
}

void listEntries()
{
    directory_entry_t *directoryEntry = (directory_entry_t *)currentDirectory.startingAddress;

    print("name:   ");
    print("ext:    \n");
    while(directoryEntry->filename[0] != 0)
    {
        for(int i = 0; i < 8; i++)
        {
            putchar(directoryEntry->filename[i]);
        }

        for(int i = 0; i < 3; i++)
        {
            putchar(directoryEntry->ext[i]);
        }

        putchar('\n');

        directoryEntry++;
    }
    
}

void padFileName(char *name)
{
    pad(name, ' ', 8);
}

void padExtension(char *ext)
{
    pad(ext, ' ', 3);
}

void pad(char *string, char character, int count)
{
    int index = 0;
    while(string[index] != 0) index++;
    while (index < count)
    {
        string[index] = character;
        index++;
    }
}