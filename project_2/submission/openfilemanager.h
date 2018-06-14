		// openfilemanager.h
//

#ifndef OPENFILEMANAGER_H
#define OPENFILEMANAGER_H

#include "sysopenfile.h"
#include "synch.h"

#define OPEN_FILE_TABLE_SIZE 32
#define FAILED_TO_ADD -1
#define FAILED_TO_FIND -1

class OpenFileManager {

public:
    OpenFileManager();
    ~OpenFileManager();

    int addOpenFile(OpenFile* openFile, char* filename);
    SysOpenFile *getOpenFile(int index);
    void reduceProccessOpeningOf(int index);

    Lock *consoleWriteLock;
    //    Lock **consoleReadLock;
private:
    SysOpenFile* openFileTable[OPEN_FILE_TABLE_SIZE];
    int getIndexOf(OpenFile* file);
    int used;
};

#endif // OPENFILEMANAGER_H
