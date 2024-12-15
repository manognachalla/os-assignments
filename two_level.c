#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define VIRTUAL_PAGES 16    
#define PHYSICAL_FRAMES 8   
#define DIRECTORY_SIZE 4    //top-level table size
#define PAGE_TABLE_SIZE 4    //next-level page table size

typedef struct {
    int frameNumber; 
    bool valid;  
} pageTableEntry;

typedef struct {
    pageTableEntry* pageTable; //to pooint to second-level page table
} directoryEntry;

typedef struct {
    directoryEntry directory[DIRECTORY_SIZE]; // top-level
    int physicalMemory[PHYSICAL_FRAMES]; //physical memory
    int fifoQueue[PHYSICAL_FRAMES];     //fifo queue for page replacement
    int queueFront;   //front index-fifo queue
    int queueSize; //current size -fifoqueue
} twoLevelPagingSystem;

void initializePagingSystem(twoLevelPagingSystem* system) {
    for (int i = 0; i < DIRECTORY_SIZE; i++) {
         //each directory entry to NULL
        system->directory[i].pageTable = NULL;
    }
    for (int i = 0; i < PHYSICAL_FRAMES; i++) {
        system->physicalMemory[i] = -1;
        system->fifoQueue[i] = -1;
    }
    system->queueFront = 0;
    system->queueSize = 0;
}

void allocatePageTable(twoLevelPagingSystem* system, int directoryIndex){
    system->directory[directoryIndex].pageTable = (pageTableEntry*)malloc(PAGE_TABLE_SIZE * sizeof(pageTableEntry));
    for(int i = 0; i < PAGE_TABLE_SIZE; i++){
        system->directory[directoryIndex].pageTable[i].frameNumber = -1;
        system->directory[directoryIndex].pageTable[i].valid = false;
    }
}

void loadPageIntoFrame(twoLevelPagingSystem* system, int directoryIndex, int pageIndex, int frameNumber){
    system->directory[directoryIndex].pageTable[pageIndex].frameNumber = frameNumber;
    system->directory[directoryIndex].pageTable[pageIndex].valid = true;
    system->physicalMemory[frameNumber] = directoryIndex * PAGE_TABLE_SIZE + pageIndex;
    system->fifoQueue[(system->queueFront + system->queueSize) % PHYSICAL_FRAMES] = directoryIndex * PAGE_TABLE_SIZE + pageIndex;
    system->queueSize++;

    printf("loaded virtual page %d into physical frame %d\n", directoryIndex * PAGE_TABLE_SIZE + pageIndex, frameNumber);
}

void handlePageFault(twoLevelPagingSystem* system, int directoryIndex, int pageIndex){
    printf("page fault has occurred-virtual page %d\n", directoryIndex * PAGE_TABLE_SIZE + pageIndex);

    if(system->queueSize < PHYSICAL_FRAMES){
        int freeFrame = system->queueSize; //use the next free frame
        loadPageIntoFrame(system, directoryIndex, pageIndex, freeFrame);
    } 
    else{
        //fifo page replacement
        int victimPage = system->fifoQueue[system->queueFront];
        int victimDirectory = victimPage / PAGE_TABLE_SIZE;
        int victimPageIndex = victimPage % PAGE_TABLE_SIZE;
        system->directory[victimDirectory].pageTable[victimPageIndex].valid = false;
        int victimFrame = system->directory[victimDirectory].pageTable[victimPageIndex].frameNumber;
        //update the queue
        system->queueFront = (system->queueFront + 1) % PHYSICAL_FRAMES;
        system->queueSize--;
        loadPageIntoFrame(system, directoryIndex, pageIndex, victimFrame);
    }
}

int translateAddress(twoLevelPagingSystem* system, int virtualPage){
    int directoryIndex = virtualPage / PAGE_TABLE_SIZE;
    int pageIndex = virtualPage % PAGE_TABLE_SIZE;
    if(system->directory[directoryIndex].pageTable == NULL){
        allocatePageTable(system, directoryIndex);
    }
    if(system->directory[directoryIndex].pageTable[pageIndex].valid){
        return system->directory[directoryIndex].pageTable[pageIndex].frameNumber;
    } 
    else{
        handlePageFault(system, directoryIndex, pageIndex);
        return system->directory[directoryIndex].pageTable[pageIndex].frameNumber;
    }
}

void displayPageTable(twoLevelPagingSystem* system){
    printf("\ntwo-lvel page table:\n");
    for(int i = 0; i < DIRECTORY_SIZE; i++){
        if(system->directory[i].pageTable != NULL){
            for(int j = 0; j < PAGE_TABLE_SIZE; j++){
                int virtualPage = i*PAGE_TABLE_SIZE+j;
                printf("virtual page %d: ",virtualPage);
                if(system->directory[i].pageTable[j].valid){
                    printf("frame %d\n", system->directory[i].pageTable[j].frameNumber);
                } 
                else{
                    printf("not in memory\n");
                }
            }
        }
    }
}

int main(){
    twoLevelPagingSystem system;
    initializePagingSystem(&system);
    int virtualAddresses[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 1, 9, 10, 2, 11, 3, 12};
    int numAddresses = sizeof(virtualAddresses) / sizeof(virtualAddresses[0]);
    for (int i = 0; i < numAddresses; i++) {
        int va = virtualAddresses[i];
        int frame = translateAddress(&system, va);
        printf("virtual address %d -> physical frame %d\n", va, frame);
    }
    displayPageTable(&system);
    return 0;
}
