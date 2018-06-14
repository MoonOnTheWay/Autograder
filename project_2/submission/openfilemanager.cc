// openfilemanager.cc
//

#include "openfilemanager.h"

//----------------------------------------------------------------------
// OpenFileManager::OpenFileManager
//  Construct and initialize the open file manager.
//----------------------------------------------------------------------

OpenFileManager::OpenFileManager()
{
   consoleWriteLock = new Lock("consoleWriteLock");
   int used = 0;
}

//----------------------------------------------------------------------
// OpenFileManager::~OpenFileManager
//----------------------------------------------------------------------

OpenFileManager::~OpenFileManager()
{
    for(int i = 0; i < OPEN_FILE_TABLE_SIZE; i++){
      if (openFileTable[i] != NULL){
	delete openFileTable[i];
      }
    }
    delete[] openFileTable;
    delete consoleWriteLock;
}

//----------------------------------------------------------------------
// OpenFileManager::addOpenFile
//  Adds an on open file to the system file table.
//----------------------------------------------------------------------

int OpenFileManager::addOpenFile(OpenFile* openFile, char* filename)
{
	int index = getIndexOf(openFile);
	if(index == FAILED_TO_FIND){
		if(used == OPEN_FILE_TABLE_SIZE) return FAILED_TO_ADD;
		for(int i = 0; i < OPEN_FILE_TABLE_SIZE; i++){
			if(openFileTable[i] == NULL){
				openFileTable[i] = new SysOpenFile(openFile, i, filename);
				used++;
				return i;
			}
		}
	}
	else{
		openFileTable[index]->numProcOpen++;
		return index;
	}
	return FAILED_TO_ADD; //Should never get here; only here for compiler
}

//----------------------------------------------------------------------
// OpenFileManager::getOpenFile
//  Retrieves the system file table entry from the file table.
//----------------------------------------------------------------------

SysOpenFile *OpenFileManager::getOpenFile(int index)
{
	if(openFileTable[index] == NULL)
		return NULL;
	return openFileTable[index];
}

//----------------------------------------------------------------------
// OpenFileManager::getIndexOf
//  Retrieves the index of file
//----------------------------------------------------------------------

int OpenFileManager::getIndexOf(OpenFile* file)
{
	for(int i = 0; i < OPEN_FILE_TABLE_SIZE; i++){
		if (openFileTable[i] != NULL && openFileTable[i]->openFile == file){
			return i;
		}
	}
	return FAILED_TO_FIND;
}

//----------------------------------------------------------------------
// OpenFileManager::reduceProccessOpeningOf
//  Reduce the number of processes accessing an opened file.
//	Deallocate if no longer needed
//----------------------------------------------------------------------

void OpenFileManager::reduceProccessOpeningOf(int index)
{
	openFileTable[index] -> reduceProcOpen();
	if(openFileTable[index] -> numProcOpen == 0){
		delete openFileTable[index];
		used--;
		openFileTable[index] = NULL;
	}
}
