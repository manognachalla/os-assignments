#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define VIRTUAL_PAGES 16 
#define PHYSICAL_FRAMES 8 

typedef struct {
    int frameNumber; //physical memory
    bool valid;     
} pageTableEntry;

typedef struct {
    pageTableEntry pageTable[VIRTUAL_PAGES]; // page table
    int physicalMemory[PHYSICAL_FRAMES];  // physical memory
    int fifoQueue[PHYSICAL_FRAMES];      // fifo queue for page replacement
    int queueFront;       // front index of the fifo queue
    int queueSize;         // current size of the fifo queue
} pagingSystem;

void initializePagingSystem(pagingSystem* system) {
    for (int i = 0; i < VIRTUAL_PAGES; i++) {
        system->pageTable[i].frameNumber = -1;
        system->pageTable[i].valid = false;
    }
    for (int i = 0; i < PHYSICAL_FRAMES; i++) {
        system->physicalMemory[i] = -1;
        system->fifoQueue[i] = -1;
    }
    system->queueFront = 0;
    system->queueSize = 0;
}

void loadPageIntoFrame(pagingSystem* system, int virtualPage, int frameNumber) {
    system->pageTable[virtualPage].frameNumber = frameNumber;
    system->pageTable[virtualPage].valid = true;
    system->physicalMemory[frameNumber] = virtualPage;
    system->fifoQueue[(system->queueFront + system->queueSize) % PHYSICAL_FRAMES] = virtualPage;
    system->queueSize++;

    printf("Loaded virtual page %d into physical frame %d\n", virtualPage, frameNumber);
}

void handlePageFault(pagingSystem* system, int virtualPage) {
    printf("Page fault occurred for virtual page %d\n", virtualPage);

    if (system->queueSize < PHYSICAL_FRAMES) {
        int freeFrame = system->queueSize; // uses the next free frame
        loadPageIntoFrame(system, virtualPage, freeFrame);
    } else {
        //fifopage replacement
        int victimPage = system->fifoQueue[system->queueFront];
        system->pageTable[victimPage].valid = false;
        int victimFrame = system->pageTable[victimPage].frameNumber;
        
        //to updarw the queue
        system->queueFront = (system->queueFront + 1) % PHYSICAL_FRAMES;
        system->queueSize--;
        
        loadPageIntoFrame(system, virtualPage, victimFrame);
    }
}

int translateAddress(pagingSystem* system, int virtualPage) {
    if (system->pageTable[virtualPage].valid) {
        return system->pageTable[virtualPage].frameNumber;
    } else {
        handlePageFault(system, virtualPage);
        return system->pageTable[virtualPage].frameNumber;
    }
}

void displayPageTable(pagingSystem* system) {
    printf("\npage table:\n");
    for (int i = 0; i < VIRTUAL_PAGES; i++) {
        printf("virtual page %d: ", i);
        if (system->pageTable[i].valid) {
            printf("frame %d\n", system->pageTable[i].frameNumber);
        } else {
            printf("not in memory\n");
        }
    }
}

int main() {
    pagingSystem system;
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
