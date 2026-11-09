#include "project5.h"
#define DIRECTORY_BUFFER_CAPACITY 4096
#if PROJECT == 5
int kernel()
{
    startkernel(startshell);
    return 0;
}
#endif

int createSubdirectory(char *subdirectoryName)
{
    int directorySize = 512;
    //i. 
    //1. checks if the current Directory is opened///////////////////////////////
    if(!currentDirectory.isOpened)
    {
        print("Error #1: current directory is not opened\n");
        return -1;
    }/////////////////////////////////////////////////////////////////////////////

    //ii. find the next free directory entry inside of the currentDirectory
    //1. Loop through currentDirectory.startingAddress 
    // until a directory_entry_t is found with filename[0] == 0/////////////////////////////
    directory_entry_t *entry = (directory_entry_t *)currentDirectory.startingAddress;
    int maxEntries = directorySize / sizeof(directory_entry_t);
    int count = 0;
    while(count < maxEntries && entry[count].filename[0] != 0){
        
        count++;
    }/////////////////////////////////////////////////////////////////////////////////////////



    //iii. Check to see if the directory that you are creating this subdirectory in needs to be resized///////////
    if(count < maxEntries){
        //A free entry exists
        entry = &entry[count];
    }else{

        //if the directory is full
        if(currentDirectory.directoryEntry == &rootDirectoryEntry){
            print("Error #2: Root directory is full.\n");
            return 1;
        }else{
            //find the final cluster in the directory's FAT chain
            uint16 lastCluster = currentDirectory.directoryEntry->startingCluster;

            while(fat0->clusters[lastCluster] < 0xFFF8){
                lastCluster = fat0->clusters[lastCluster];
            }

            //Find a free cluster
            uint16 nextCluster = nextFreeCluster();

            if(nextCluster == 0){
                print("Error #3: No free clusters available.\n");
                return 1;
            }

            // Make sure the buffer has room for another cluster
            if (directorySize + 512 > DIRECTORY_BUFFER_CAPACITY)
            {
                print("Error #4: Directory buffer is full.\n");
                return 1;
            }

            //Link the old final cluster to the new cluster
            fat0->clusters[lastCluster] = nextCluster;
            fat1->clusters[lastCluster] = nextCluster;


            //mark the new cluster as the end of the chain
            fat0->clusters[nextCluster] =0xFFFF;
            fat1->clusters[nextCluster] =0xFFFF;


            //uint8 *newClusterAddress = (uint8 *)currentDirectory.startingAddress + directorySize;

            

            uint8 *newClusterAddress =
                (uint8 *)currentDirectory.startingAddress + directorySize;

            for (int i = 0; i < 512; i++) {
                newClusterAddress[i] = 0;
            }

            entry = (directory_entry_t *)newClusterAddress;
            directorySize += 512;

            for(int i = 0; i < 512; i++){
                newClusterAddress[i] = 0;
            }

            //point to the first entry in the new cluster
            entry = (directory_entry_t *)newClusterAddress;

            //update the directory size
            directorySize += 512;

            //finally write the FAT copies to the disk
            writeFATs();
        }
    }////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //iv. Find the next free cluster in the FAT and mark it as EOF///////////////////////////////////////////
    uint16 nextCluster = nextFreeCluster(); //finds an available cluster

    if(nextCluster == 0){
        print("Error #5: No free clusters available\n");
        return 1;
    }

    //Mark the cluster as the end of the FAT chain
    fat0->clusters[nextCluster] = 0xFFFF; //reserves the cluster and marks it as the end of the chain
    fat1->clusters[nextCluster] = 0xFFFF; //reserves the cluster and marks it as the end of the chain


    //Write the FATs to the disk
    writeFATs(); // writes the updated copies to the disk
    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    //v. Using the free entry found in step ii///////////////////////////////////////////////////////////////
    //fill in the directory entry for the new subdirectory
    //Set the filename to the subdirectory name
    stringcopy(subdirectoryName, (char *)entry->filename, 8);

    //Set the extension to spaces
    stringcopy("   ", (char *) entry->ext, 3);

    //Set the attributes to 0x10 (subdirectory)
    entry->attributes = 0x10;

    //Set the starting cluster to the cluster from step iv
    entry->startingCluster = nextCluster;

    //Set the file size to 512
    entry->fileSize = 512;

    //Write the current directory to disk
    writeCurrentDirectory();
    ///////////////////////////////////////////////////////////////////////////////////

    //Step vi. Create a buffer for the new sector for the subdirectory and add:///////////////////////////////////
        //1. Add an entry for "." containing the directory entry for the subdirectory itself
        uint8 buffer[512] = {0}; 
        directory_entry_t *selfEntry = (directory_entry_t *)buffer;

        //Copy the new subdirectory's entry
        *selfEntry = *entry;
        stringcopy(".       ",(char *)selfEntry->filename, 8);
        stringcopy("   ", (char *)selfEntry->ext, 3);

        //2. Add an entry for ".." containing the directory entry for the currentDirectory
        directory_entry_t *parentEntry = ((directory_entry_t *)buffer) + 1;
        *parentEntry = *currentDirectory.directoryEntry;
        stringcopy("..      ", (char *) parentEntry->filename, 8);
        stringcopy("   ", (char *) parentEntry->ext, 3);

        //3. Write the buffer to the disk
        floppy_write(0, nextCluster + CLUSTER_TO_SECTOR, (void *)buffer, 512);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //vii. Return 0 if no errors have occured in any of the previous steps
    return 0;
}

int openDirectory(char *directoryName)
{
    int directorySize = 512;
    //i. Checks if the currentDirectory is opened///////////////////////////////////
    if(!currentDirectory.isOpened)
    {
        print("Error #6: current directory is not opened\n");
        return -1;
    }
    ////////////////////////////////////////////////////////////////////////////////

    

    //ii. Checks if the currentFile is opened////////////////////////////////////////
    if(currentFile.isOpened){ // checks if a file is currently open
        print("Error #7: A file is currently open.\n");
        return 1;
    }
    ///////////////////////////////////////////////////////////////////////////



/*
iii. Find the directory entry that has the name directoryName //////////////////////////////////////
1. Loop through currentDirectory.startingAddress until a directory_entry_t is 
found with filename equal to directoryName and attributes with the 0x10 
bit set 
2. Hint: you can reuse some sections of openFile() to do this */

directory_entry_t *currentEntry = (directory_entry_t *)currentDirectory.startingAddress;

int MAXENTRIES = directorySize / sizeof(directory_entry_t);
int found = 0;

for(int x = 0; x < MAXENTRIES; x++){
    if(currentEntry[x].filename[0] == 0){
        break;
    }

    if(stringcompare((char *)currentEntry[x].filename, directoryName, 8) && (currentEntry[x].attributes & 0x10) != 0){
        currentEntry = &currentEntry[x];
        found = 1;
        break;
    }
}

if(!found){
    print("Error #8: Subdirectory could not be found!\n");
    return -1;
}

directory_entry_t selectedEntry = *currentEntry;
/////////////////////////////////////////////////////////////////////////////////////////////////////

    //iv Checks if we are opening the root directory//////////////////////////////////////////////////////////////////
    if(selectedEntry.startingCluster == 0){ // check to see if the selected directory is the root directory
        currentDirectory.directoryEntry = &rootDirectoryEntry;

        if(floppy_read(0, ROOT_DIR_SECTOR, (void *)currentDirectory.startingAddress, ROOT_SIZE) != 0){
            print("Error #9: Could not read the root directory.\n");
            return -1;
        }
        currentDirectory.isOpened = 1;
    }/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //v. If we are opening a non-root directory/////////////////////////////////////////////////////////////////////
    else{
        uint16 cluster = selectedEntry.startingCluster;
        int offset = 0;

        while(cluster < 0xFFF8){

            if(cluster >= FAT_ENTRIES_MAX){
                print("Error #10: Invalid directory cluster.\n");
                return 1;
            }

            if(offset + 512 > directorySize){
                print("Error #11: Directory buffer is too small.\n");
                return -1;
            }

            if(floppy_read(0, cluster + CLUSTER_TO_SECTOR, (void *)(currentDirectory.startingAddress + offset), 512) != 0){
                print("Error #12: Could not read directory cluster.\n");
                return -1;
            }
            offset += 512;

            //Get the next cluster in the FAT chain
            cluster = fat0->clusters[cluster];
        }

        //Find the "." directory entry
        directory_entry_t *entries = (directory_entry_t *)currentDirectory.startingAddress;

        int maxEntries = offset / sizeof(directory_entry_t);
        int foundSelf = 0;

        for(int i = 0; i < maxEntries; i++){
            if(entries[i].filename[0] == '.' && (entries[i].filename[1] == ' ')){
                currentDirectory.directoryEntry = &entries[i];
                foundSelf = 1;
                break;
            }
        }

        if(!foundSelf){
            print("Error #13: Directory self-entry could not be found.\n");
            return -1;
        }

        currentDirectory.isOpened = 1;
    }/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  
    return 0;
}