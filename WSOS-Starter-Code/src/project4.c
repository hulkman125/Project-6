#include "project4.h"

// FAT Copies
// First copy is fat0 stored at 
// Second copy is fat1
// There were issues declaring the FATs as non-pointers
// When they would get read from floppy, it would overwrite wrong areas of memory
fat_t *fat0;
fat_t *fat1;

directory_t currentDirectory;  // The current directory we have opened
directory_entry_t rootDirectoryEntry;   // The root directory's directory entry (this does not exist on the disk since the root is not inside of another directory)
file_t currentFile;            // The current file we have opened

#if PROJECT == 4
int kernel()
{
    startkernel(startshell);
    return 0;
}
#endif

int writeFATs()
{
    int SizeOfFAT = 512 * 11; // 11 sectors of 512 bytes each
    uint16 fataCluster;
    uint16 fatbCluster;

    for(int x = 0; x < FAT_ENTRIES_MAX; x++) {
        fataCluster = fat0->clusters[x];
        fatbCluster = fat1->clusters[x];

        if(fataCluster != fatbCluster) {
            print("ERROR #1: -FATs are not the same!\n");
            return 1;
        }    
    }

    int error;
    error = floppy_write(0, FAT0_SECTOR, (void *)fat0, SizeOfFAT);
    if(error != 0) {
        print("ERROR #2: -Could not write to FAT0 to disk\n");
        return 1;
    }

    error = floppy_write(0, FAT1_SECTOR, (void *)fat1, SizeOfFAT);
    if(error != 0) {
        print("ERROR #3: -Could not write to FAT1 to disk\n");
        return 1;
    }

    return 0;
}

int writeCurrentDirectory()
{
    //print("Writing directory...\n");
    if(!currentDirectory.isOpened) {
        print("ERROR #4: -No directory is open, cannot write to disk\n");
        return 1;
    }

    if(currentDirectory.directoryEntry->startingCluster == 0) {
        // Root directory, write to the root directory sector
        int error = floppy_write(0, ROOT_DIR_SECTOR, (void *)currentDirectory.startingAddress, ROOT_SIZE);
        if(error != 0) {
            print("ERROR #5: -Could not write root directory to Floppy disk\n");
            return 1;
        }
    }
    else{
        // Non-root directory, write to the cluster
        uint16 cluster = currentDirectory.directoryEntry->startingCluster;
        uint8 *startingAddress = currentDirectory.startingAddress;
        uint32 offset = 0;
        uint32 sectorCount = 0;

        while(cluster != 0xFFFF) {
            if(cluster < 2 || cluster >= FAT_ENTRIES_MAX) {
                print("ERROR #6: -Cluster is out of bounds\n");
                return 1;
            }
            uint32 sector = cluster + 22;

            uint8 error = floppy_write(0, sector, (void *)startingAddress + offset, 512);
            if(error) {
                print("ERROR #7: -Could not write to cluster\n");
                return 1;
            }
            sectorCount++;
            offset += 512;
            cluster = fat0->clusters[cluster];

            if(sectorCount > FAT_ENTRIES_MAX) {
                print("ERROR #8 : -Cluster count exceeded maximum!\n");
                return 1;
            }
        }
    }
    return 0;
}

int writeCurrentFile()
{
    //print("Writing file...\n");
    if(!currentFile.isOpened) {
        print("ERROR #9: -No file is open, cannot write to disk\n");
        return 1;
    }

    uint16 Cluster = currentFile.directoryEntry->startingCluster;
    uint8 *startingAddress = currentFile.startingAddress;
    uint32 offset = 0;
    uint16 sectorCount = 0;

    while(Cluster != 0XFFFF) {

        if(Cluster < 2 || Cluster >= FAT_ENTRIES_MAX) {
            print("ERROR #10: -Cluster is out of bounds\n");
            return 1;
        }
        uint32 sector = Cluster + CLUSTER_TO_SECTOR;

        int error = floppy_write(0, sector, startingAddress + offset, ROOT_SIZE);

        if(error != 0) {
            print("ERROR #11: -Could not write to cluster\n");
            return 1;
        }
        sectorCount++;
        offset += ROOT_SIZE;
        Cluster = fat0->clusters[Cluster];

        if(sectorCount > FAT_ENTRIES_MAX) {
            print("ERROR #12: -Cluster count exceeded maximum!\n");
            return 1;
        }

    }
    return 0;
}

int resizeFile(uint16 startingCluster, uint32 newFileSize)
{
    //print("Resizing file...\n");
    if(startingCluster < 2 || startingCluster >= FAT_ENTRIES_MAX) {
        print("ERROR #13: -Starting cluster is out of bounds\n");
        return 1;
    }
    uint32 ClustersNeeded = (newFileSize + ROOT_SIZE - 1) / ROOT_SIZE; // Round up to the nearest cluster
    
    if(ClustersNeeded == 0 || ClustersNeeded > FAT_ENTRIES_MAX) {
        print("ERROR #14: -Invalid cluster count\n");
        return 1;
    }


    uint16 Cluster = startingCluster;
    uint16 clusterCount = 0;

    while(Cluster != 0xFFFF){
        
        if(Cluster < 2 || Cluster >= FAT_ENTRIES_MAX || clusterCount >= FAT_ENTRIES_MAX) {
            print("ERROR #15: -Cluster is out of bounds\n");
            return 1;
        }

        clusterCount++;
        Cluster = fat0->clusters[Cluster];
    }
    
    if(ClustersNeeded == clusterCount)
    {return 0;}

    if(ClustersNeeded < clusterCount) { //shrink the cluster chain
        Cluster = startingCluster;

        for(uint16 i = 1; i < ClustersNeeded; i++) {
            Cluster = fat0->clusters[Cluster];
        }

        uint16 ToFreeCluster = fat0->clusters[Cluster];

        //setting the new 'last' cluster to EOF
        fat0->clusters[Cluster] = 0xFFFF;
        fat1->clusters[Cluster] = 0xFFFF;

        uint32 freedCount = 0;
        //Overwrite the remaining clusters to x0000
        while(ToFreeCluster != 0xFFFF) {
            if(ToFreeCluster < 2 || ToFreeCluster >= FAT_ENTRIES_MAX || freedCount >= FAT_ENTRIES_MAX) {
                print("ERROR #17: -Cluster is out of bounds\n");
                return 1;
            }
            uint16 nextCluster = fat0->clusters[ToFreeCluster];
            fat0->clusters[ToFreeCluster] = 0x0000;
            fat1->clusters[ToFreeCluster] = 0x0000;
            ToFreeCluster = nextCluster; // Move to the next cluster in the chain
            freedCount++;
        }
    }

    else if(ClustersNeeded > clusterCount){ //add clusters to the chain
        
        uint32 additionalClustersNeeded = ClustersNeeded - clusterCount;
        uint32 freeCount = 0;

        for(uint16 i = 2; i < FAT_ENTRIES_MAX; i++) {
            if(fat0->clusters[i] == 0x0000) {
                freeCount++;
            }
        }

        if(freeCount < additionalClustersNeeded) {
            print("ERROR #16: -Not enough free clusters to resize file\n");
            return 1;
        }

        uint16 currentCluster = startingCluster;
        uint32 clusterCount = 0;

        // Traverse to the last cluster in the current chain
        while(1){
            if(currentCluster < 2 || currentCluster >= FAT_ENTRIES_MAX || clusterCount >= FAT_ENTRIES_MAX) {
                print("ERROR #17: -Current cluster is out of bounds\n");
                return 1;
            }

            clusterCount++;

            if(fat0->clusters[currentCluster] == 0xFFFF) { // Found the last cluster in the chain
                break;
            }
            currentCluster = fat0->clusters[currentCluster];
        }

        //add the required clusters
        for(uint32 i = 0; i < additionalClustersNeeded; i++) { 
            uint16 nextFree = nextFreeCluster(); // Find the next free cluster

            if(nextFree == 0xFFFF || nextFree < 2 || nextFree >= FAT_ENTRIES_MAX || fat0->clusters[nextFree] != 0x0000) {
                print("ERROR #18: -Next free cluster is out of bounds\n");
                return 1;
            }

            // Link the current cluster to the next free cluster and mark the next free cluster as EOF
            fat0->clusters[currentCluster] = nextFree;
            fat1->clusters[currentCluster] = nextFree;
            fat0->clusters[nextFree] = 0xFFFF;
            fat1->clusters[nextFree] = 0xFFFF;
            currentCluster = nextFree; // Move to the newly added cluster
        }
    }

    bool error = writeFATs();
    if(error) {
        print("ERROR #19: -Could not write FATs to disk\n");
        return 1;
    }

   return 0;
}