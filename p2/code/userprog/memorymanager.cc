/*
 * MemoryManager implementation
 *
 * Used to facilitate contiguous virtual memory.  Utilizes the provided bitmap
 * data structure to store the state of pages in the page table.
*/

#include "memorymanager.h"
#include "machine.h"

// Constructor
MemoryManager::MemoryManager() {
    physPageAllocation = new BitMap(NumPhysPages);
    lock = new Lock("memoryManager->lock");
}

// Destructor
MemoryManager::~MemoryManager() {
    delete physPageAllocation;
    delete lock;
}

// Allocates the first clear page
// Return -1 upon error.
int MemoryManager::getPage() {

    int pageIndex = physPageAllocation->Find(); // sets the bit as a side-effect
    if (pageIndex == -1) {
        DEBUG('m', "Unable to find a page from the page table.");
        ASSERT(FALSE);
        return -1;
    } else {
        return pageIndex;
    }
}

// Frees a page from the table
void MemoryManager::clearPage(int pageIndex) {
    physPageAllocation->Clear(pageIndex);
}

// Returns the number of available pages
unsigned int MemoryManager::getNumFreePages() {
    return physPageAllocation->NumClear();
}
