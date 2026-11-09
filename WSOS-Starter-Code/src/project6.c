#include "project6.h"

#if PROJECT == 6
int kernel()
{
    startkernel(startshell);
    return 0;
}
#endif

// Delete the file that is currently opened
// All FAT entries for this file must be assigned a value of x0000
// Set the directory entry's first byte to x00
// Copy the last directory entry into this one if there are other files in this directory
// Returns 0 if file was successfully deleted
// Returns non-zero if error occurred
int deleteFile()
{
    //i. Check to see if currentFile is opened, if not return with error code (i.e. return -1) 
    if(!currentFile.isOpened){
        print("ERROR #1: Current file is not opened, SO OPEN THE FILE UP ALLREADY YOU DING DONG");
        return -1;
    }

    //ii. Overwrite all FAT clusters in the FAT table for this file with x0000 
    uint16 currentCluster = currentFile.directoryEntry->startingCluster;
    while(currentCluster != 0xFFFF && currentCluster != 0x0000){
        uint16 nextCluster = fat0->clusters[currentCluster];

        fat0->clusters[currentCluster] = 0x0000;
        fat1->clusters[currentCluster] = 0x0000;

        currentCluster = nextCluster;
    }


    /* iii. Remove the directory entry that has the currentFile.directoryEntry->filename inside of it 
at currentDirectory.startingAddress by overwriting the first element with NULL (x00) or by 
overwriting entire directory entry with NULL */

        //1. Recommended: overwrite this directory entry with the last directory entry in the directory
        directory_entry_t *lastEntry = (directory_entry_t *)currentDirectory.startingAddress;
        int count = 0;
        int maxEntries =  512 / sizeof(directory_entry_t);
        
        while(count < maxEntries && lastEntry->filename[0] != 0){ //checking for each file and stopping if its null
            count++;
            lastEntry++;
        }

        if(count == 0){
            print("ERROR #1: No files in the directory, SO CHECK THE DIRECTORY YOU DING DONG");
            return -1;
        }
        lastEntry-- ; //go back to the last entry that is not null

        //Overwrite the currentFile.directoryEntry with the lastEntry
        // Copy the last directory entry into the current file's entry
        *currentFile.directoryEntry = *lastEntry;

        // Mark the last entry as empty
        lastEntry->filename[0] = 0;


        // iv. Call writeFATs() to write the changes made to the FATs back to the disk 
    //1. If error occurs, return non zero

        if(writeFATs() != 0){
            print("ERROR #2: Could not write FATs to disk. SO FIND A WAY TO WRITE BACK TO THE DISK YOU DOOF!!!!!!!!!!\n");
            return -1;
        }
    // v. Call writeCurrentDirectory() to write the changes made to the current directory back to the disk 
        if(writeCurrentDirectory() != 0){ //1. If error occurs, return non zero 
            print("ERROR #3: Could not write current directory to disk. SO FIX THIS YOU BUFFOON!!!!!!!!!!\n");
            return -1;
        }
    

    // vi.Set currentFile.isOpened to 0 (false) 
        currentFile.isOpened = 0;

    return 0;
}

// Delete the subdirectory inside of the directory that is currently opened
// Check to see if the subdirectory exists and is empty
// All FAT entries for this subdirectory must be assigned a value of x0000
// Set the directory entry's first byte to x00
// Copy the last directory entry into this one if there are other files in this directory
// Returns 0 if subdirectory was successfully deleted
// Returns non-zero if error occurred
int deleteSubdirectory(char *directoryName)
{
    //i. check to see if current file is opened or not
    if(!currentDirectory.isOpened){
        print("ERROR :Current directory is not opened, SO OPEN THE DIRECTORY UP ALLREADY YOU BLOCKHEAD!!!!!!!!!!");
        return 1;
    }

    //ii. Loop through the directory entries starting at currentDirectory.startingAddress and find the entry with filename of directoryName 
        //1. If directoryName is not found, return non zero 

        directory_entry_t *entry = (directory_entry_t *)currentDirectory.startingAddress;
        int nameLength = 0;

        while(directoryName[nameLength] != '\0' ){
            nameLength++;
        }

        int count = 0;
        int maxEntries =  512 / sizeof(directory_entry_t); //calculate the maximum number of entries in the directory
        directory_entry_t *directoryToDelete = 0;

        while(count < maxEntries && entry[count].filename[0] != 0){
            if(!(entry[count].filename[0] == '.' &&
             (entry[count].filename[1] == ' ' ||
              entry[count].filename[1] == '.' ||
              entry[count].filename[1] == '\0'))){ //check if the entry is
            
            if(stringcompare((char *)entry[count].filename, directoryName, nameLength) && (entry[count].attributes & 0x10) == 0x10){ //check if the entry is a directory
                directoryToDelete = &entry[count];
                break;
            }
        }
            count++;
        }

        if(directoryToDelete == 0){
            print("ERROR: Subdirectory not found, SO CHECK THE NAME OF THE DIRECTORY YOU DUNDERHEAD!!!!!!!!!!");
            return -1;
        }

    //iii. Create a 512 byte buffer and load the subdirectory into it (we must check that the 
    //subdirectory is empty before continuing) 
    //1. Note: you only need to load the first cluster of the subdirectory since the first two 
    //entries should be “.” and “..” if the third directory is not empty, return non-zero 

        uint8 directoryBuffer[512] = {0};

        directory_entry_t *subdirectoryEntry = (directory_entry_t *)directoryBuffer;

        if(floppy_read(0, directoryToDelete->startingCluster + CLUSTER_TO_SECTOR, (void *)directoryBuffer, 512) != 0){
            print("ERROR: Could not read the subdirectory from disk, SO CHECK THE DISK YOU EINSTIEN!!!!!!!!!!");
            return -1;
        }

        
        if(subdirectoryEntry[2].filename[0] != 0){ //check if the third entry is not null
            print("ERROR: The directory is not empty, SO DELETE THE FILES INSIDE OF IT FIRST YOU CRAZY LOON!!!!!!!!!!");
            return -1;
        }



    // iv. Overwrite all FAT clusters in the FAT table for this subdirectory with x0000 
    
        uint16 currentCluster = directoryToDelete->startingCluster;
        while(currentCluster != 0xFFFF && currentCluster != 0x0000){
            
            uint16 nextCluster = fat0->clusters[currentCluster];

            fat0->clusters[currentCluster] = 0x0000;
            fat1->clusters[currentCluster] = 0x0000;

            currentCluster = nextCluster;
        }

    //v. Remove the directory entry that has the directoryName inside of it at 
    //currentDirectory.startingAddress by overwriting the first element with NULL (x00) or by 
    //overwriting entire directory entry with NULL 
    //1. Recommended: overwrite this directory entry with the last directory entry in the 
    //directory 
        
    uint16 MaxEntries = 512 / sizeof(directory_entry_t); //calculate the maximum number of entries in the directory
    directory_entry_t *base = (directory_entry_t *)currentDirectory.startingAddress; //get the base address of the directory
    directory_entry_t *lastEntry = base; //get the last entry in the directory 
    count = 0;

    while(count < MaxEntries && lastEntry->filename[0] != 0){ //checking for each file and stopping if its null
        count++;
        lastEntry++;
    }

    if(count == 0){
        print("ERROR: No files in the directory, SO CHECK THE DIRECTORY YOU DING DONG");
        return -1;
    }

    lastEntry--; //go back to the last entry that is not null

    *directoryToDelete = *lastEntry; //overwrite the entry to delete with the last entry   
    lastEntry->filename[0] = 0; //set the first byte of the last entry to null

    

    //vi. Call writeFATs() to write the changes made to the FATs back to the disk 
    //1. If error occurs, return non zero 
    if(writeFATs() != 0){
        print("ERROR: Could not write FATs to disk, SO CHECK THE DISK YOU IDIOT!!!!!!!!!!");
        return -1;
    }

    //vii. Call writeCurrentDirectory() to write the changes made to the current directory back to the disk 
    //1. If error occurs, return non zero 
    if(writeCurrentDirectory() != 0){
        print("ERROR: Could not write current directory to disk, SO CHECK THE DISK YOU DUMMY!!!!!!!!!!");
        return -1;
    }
    
    return 0;
}