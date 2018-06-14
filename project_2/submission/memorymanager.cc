// memorymanager.cc
//

#include "memorymanager.h"
#include "system.h"

//----------------------------------------------------------------------
// MemoryManager::MemoryManager
//  Create a new memory manager and initialize it with the size of
//  physical memory.
//----------------------------------------------------------------------

MemoryManager::MemoryManager()
{
    // Create a bitmap with one bit for each frame
    frames = new BitMap(NumPhysPages);
    lock = new Lock("MemoryManagerLock");
}

//----------------------------------------------------------------------
// MemoryManager::~MemoryManager
//  Deallocate a memory manager.
//----------------------------------------------------------------------

MemoryManager::~MemoryManager()
{
    delete frames;
    delete lock;
}

//----------------------------------------------------------------------
// MemoryManager::allocFrame
//  Allocate a free frame of physical memory to be used by a process.
//----------------------------------------------------------------------

int MemoryManager::allocFrame()
{
    int frameIndex = frames -> Find();
    if (frameIndex == -1){
    	DEBUG('m', "Unable to find a page from the page table.");
        ASSERT(FALSE);
    } else {
        return frameIndex;
    }	
}

//----------------------------------------------------------------------
// MemoryManager::freeFrame
//  Deallocate a frame that is in use so that it can be allocated by
//  another process.
//----------------------------------------------------------------------

void MemoryManager::freeFrame(int frame)
{
	frames -> Clear(frame);
}
