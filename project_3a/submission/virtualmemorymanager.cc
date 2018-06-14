/*
 * VirtualMemoryManager implementation
 *
 * Used to facilitate demand paging through providing a means by which page
 * faults can be handled and pages loaded from and stored to disk.
*/

#include <stdlib.h>
#include <machine.h>
#include "virtualmemorymanager.h"
#include "system.h"



VirtualMemoryManager::VirtualMemoryManager()
{
	fileSystem->Create(SWAP_FILENAME, SWAP_SECTOR_SIZE * SWAP_SECTORS);
	swapFile = fileSystem->Open(SWAP_FILENAME);

	swapSectorMap = new BitMap(SWAP_SECTORS);
	physicalMemoryInfo = new FrameInfo[NumPhysPages];
	//swapSpaceInfo = new SwapSectorInfo[SWAP_SECTORS];
	nextVictim = 0;
}

VirtualMemoryManager::~VirtualMemoryManager()
{
	fileSystem->Remove(SWAP_FILENAME);
	delete swapFile;
	delete [] physicalMemoryInfo;
	//delete [] swapSpaceInfo;
}

int VirtualMemoryManager::allocSwapSector()
{
	int location = swapSectorMap->Find() * PageSize; // also marks the bit
	return location;
}
/*
SwapSectorInfo * VirtualMemoryManager::getSwapSectorInfo(int index)
{
	return swapSpaceInfo + index;
	
}
*/
void VirtualMemoryManager::writeToSwap(char *page, int pageSize,
									   int backStoreLoc)
{
	swapFile->WriteAt(page, pageSize, backStoreLoc);
}

void VirtualMemoryManager::swapPageIn(int virtAddr)
{
	TranslationEntry* currPageEntry;
	while(true)
	{
		FrameInfo *physPageInfo = physicalMemoryInfo + nextVictim;
		if(physPageInfo->space != NULL)
		{
			currPageEntry = getPageTableEntry(physPageInfo);
			if(currPageEntry->use == true)
			{
				currPageEntry->use = false;
				nextVictim = (nextVictim+1)%NumPhysPages;
			}
			else
			{
				if(currPageEntry->dirty == false)
				{
					// do nothing
					currPageEntry->valid = false;
					currPageEntry->use = false;
					physPageInfo->space = currentThread->space;
					physPageInfo->pageTableIndex = virtAddr/PageSize;
					TranslationEntry *newEntry = getPageTableEntry(physPageInfo);
					newEntry->physicalPage = currPageEntry->physicalPage;
					nextVictim = (nextVictim+1)%NumPhysPages;
					loadPageToCurrVictim(virtAddr);
					break;
				}
				else
				{
					char *pageContent = machine->mainMemory + currPageEntry->physicalPage*PageSize;
					writeToSwap(pageContent,PageSize,currPageEntry->locationOnDisk);
					currPageEntry->valid = false;
					currPageEntry->use = false;
					physPageInfo->space = currentThread->space;
					physPageInfo->pageTableIndex = virtAddr/PageSize;
					TranslationEntry *newEntry = getPageTableEntry(physPageInfo);
					newEntry->physicalPage = currPageEntry->physicalPage;
					nextVictim = (nextVictim+1)%NumPhysPages;
					loadPageToCurrVictim(virtAddr);
					return;
				}
			}
		}
		else
		{
			physPageInfo->pageTableIndex = virtAddr/PageSize;
			physPageInfo->space = currentThread->space;
			currPageEntry = getPageTableEntry(physPageInfo);
			currPageEntry->physicalPage = memoryManager->getPage();
			nextVictim = (nextVictim+1)%NumPhysPages;
			loadPageToCurrVictim(virtAddr);
			break;
		}
	}
}

/*
 * Cleanup the physical memory allocated to a given address space after its 
 * destructor invokes.
*/
void VirtualMemoryManager::releasePages(AddrSpace* space)
{
	for (int i = 0; i < space->getNumPages(); i++)
	{
		TranslationEntry* currPageEntry = space->getPageTableEntry(i);
	//  int swapSpaceIndex = (currPageEntry->locationOnDisk) / PageSize;
 //     SwapSectorInfo * swapPageInfo = swapSpaceInfo + swapSpaceIndex;
//      swapPageInfo->removePage(currPageEntry);
	  //swapPageInfo->pageTableEntry = NULL;

		if (currPageEntry->valid == TRUE)
		{
			//int currPID = currPageEntry->space->getPCB()->getPID();
			int currPID = space->getPCB()->getPID();
			DEBUG('v', "E %d: %d\n", currPID, currPageEntry->virtualPage);
			memoryManager->clearPage(currPageEntry->physicalPage);
			physicalMemoryInfo[currPageEntry->physicalPage].space = NULL; 
		}
		swapSectorMap->Clear((currPageEntry->locationOnDisk) / PageSize);
	}
}

/*
 * After selecting a slot of physical memory as a victim and taking care of
 * synchronizing the data if needed, we load the faulting page into memory.
*/
void VirtualMemoryManager::loadPageToCurrVictim(int virtAddr)
{
	int pageTableIndex = virtAddr / PageSize;
	TranslationEntry* page = currentThread->space->getPageTableEntry(pageTableIndex);
	char* physMemLoc = machine->mainMemory + page->physicalPage * PageSize;
	int swapSpaceLoc = page->locationOnDisk;
	swapFile->ReadAt(physMemLoc, PageSize, swapSpaceLoc);

  //  int swapSpaceIndex = swapSpaceLoc / PageSize;
 //   SwapSectorInfo * swapPageInfo = swapSpaceInfo + swapSpaceIndex;
	page->valid = TRUE;
//    swapPageInfo->setValidBit(TRUE);
//    swapPageInfo->setPhysMemPageNum(page->physicalPage);
}

/*
 * Helper function for the second chance page replacement that retrieves the physical page
 * which corresponds to the given physical memory page information that the
 * VirtualMemoryManager maintains.
 * This return page table entry corresponding to a physical page
 */
TranslationEntry* VirtualMemoryManager::getPageTableEntry(FrameInfo * physPageInfo)
{
	TranslationEntry* page = physPageInfo->space->getPageTableEntry(physPageInfo->pageTableIndex);
	return page;
}

void VirtualMemoryManager::copySwapSector(int to, int from)
{
	char sectorBuf[SectorSize];
	swapFile->ReadAt(sectorBuf, SWAP_SECTOR_SIZE, from);
	swapFile->WriteAt(sectorBuf, SWAP_SECTOR_SIZE, to);
}
