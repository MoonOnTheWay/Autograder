// sysopenfile.cc
//

#include "sysopenfile.h"

SysOpenFile::SysOpenFile(OpenFile *file, int fileId, char* filename)
{
	this->openFile = file;
	this->fileId = fileId;
	this->filename = filename;
	this->numProcOpen = 1;
}

void SysOpenFile::reduceProcOpen(){
	this->numProcOpen--;
}

SysOpenFile::~SysOpenFile()
{
	if(numProcOpen == 0)
		delete openFile;
}