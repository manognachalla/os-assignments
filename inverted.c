#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define VIRTUAL_PAGES 16   
#define PHYSICAL_FRAMES 8   

typedef struct {
    int virtualPage;
    bool valid;      
}invertedPageTableEntry;

typedef struct {
    invertedPageTableEntry invertedPageTable[PHYSICAL_FRAMES]; //inverted page table
    int fifoQueue[PHYSICAL_FRAMES];  //fifo queue for page replacement
    int queueFront;    //front index of the fifoo/queue
    int queueSize;   //current size of thefifo queue
}invertedPagingSystem;

void initializePagingSystem(invertedPagingSystem* system) {
    for(int i = 0; i < PHYSICAL_FRAMES; i++){
        system->invertedPageTable[i].virtualPage = -1; //initialize each frame as empty
        system->invertedPageTable[i].valid = false;
        system->fifoQueue[i] = -1;
    }
    system->queueFront = 0;
    system->queueSize = 0;
}

int findFrame(invertedPagingSystem* system, int virtualPage){
    for(int i = 0; i < PHYSICAL_FRAMES; i++){
        if (system->invertedPageTable[i].valid && system->invertedPageTable[i].virtualPage == virtualPage){
             //if virtual page is mapped
            return i; //return physical frame
        }
    }
    return -1; //if page not found in physical frame
}

void loadPageIntoFrame(invertedPagingSystem* system, int virtualPage, int frameNumber) {
    system->invertedPageTable[frameNumber].virtualPage = virtualPage;
    system->invertedPageTable[frameNumber].valid = true;
    system->fifoQueue[(system->queueFront + system->queueSize) % PHYSICAL_FRAMES] = frameNumber;
    system->queueSize++;
    printf("loaded virtual page %d into physical frame %d\n", virtualPage, frameNumber);
}

void handlePageFault(invertedPagingSystem* system, int virtualPage) {
    printf("page fault occurred for virtual page-%d\n", virtualPage);
    if (system->queueSize < PHYSICAL_FRAMES) {
        //free frame-available-load the page into it
        int freeFrame = system->queueSize;
        loadPageIntoFrame(system, virtualPage, freeFrame);
    } 
    else {
        //fifo page replacement
        int victimFrame = system->fifoQueue[system->queueFront];
        int victimPage = system->invertedPageTable[victimFrame].virtualPage;
        system->invertedPageTable[victimFrame].valid = false; //remove the victim page
        system->queueFront = (system->queueFront + 1) % PHYSICAL_FRAMES;
        system->queueSize--;
        loadPageIntoFrame(system, virtualPage, victimFrame); //leadnew page into emptied frame
    }
}

int translateAddress(invertedPagingSystem* system, int virtualPage) {
    int frameNumber = findFrame(system, virtualPage);
    if(frameNumber != -1) {
        //page found in a physical frame
        return frameNumber;
    } 
    else {
        handlePageFault(system, virtualPage);
        return findFrame(system, virtualPage); //retrieve the new frame number after loading
    }
}

void displayInvertedPageTable(invertedPagingSystem* system) {
    printf("\ninverted page able:\n");
    for (int i = 0; i < PHYSICAL_FRAMES; i++) {
        printf("physical frame %d: ", i);
        if (system->invertedPageTable[i].valid) {
            printf("virtual page %d\n", system->invertedPageTable[i].virtualPage);
        } else {
            printf("empty\n");
        }
    }
}

int main() {
    invertedPagingSystem system;
    initializePagingSystem(&system);

    int virtualAddresses[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 1, 9, 10, 2, 11, 3, 12};
    int numAddresses = sizeof(virtualAddresses) / sizeof(virtualAddresses[0]);
    for(int i = 0; i < numAddresses; i++) {
        int va = virtualAddresses[i];
        int frame = translateAddress(&system, va);
        printf("virtual address %d -> physical frame %d\n", va, frame);
    }
    displayInvertedPageTable(&system);
    return 0;
}
